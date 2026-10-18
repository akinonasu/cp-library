# 多点評価・補間・Taylor shift の追加候補

固定 baseline `144ae32387675fa2a889ea3e8cd19b03ad5016bf` に対し、公開配列との境界コピーと Taylor shift の factorial chain 幅を調べた。下記の候補はいずれも入力間・反復間で安定した短縮を確認できず、実装には採用していない。`components/fps_evaluation.hpp` と `components/fps_taylor_blocked.hpp` は baseline のままである。

比較対象は Library Checker の [409516](https://judge.yosupo.jp/submission/409516)（多点評価）、[409545](https://judge.yosupo.jp/submission/409545)（補間）、[409534](https://judge.yosupo.jp/submission/409534)（Taylor shift）。既存実装とこれらの原本を、AMD EPYC 9V74、GCC 14.2.0、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、CPU 2 固定で比較した。ビルド・正答確認は共有ロック、計時は `/tmp/cp-fps-tuning.lock` の排他ロックで実施した。

各行は `max_random_00.in` / `max_random_01.in` の順。値はプロセス起動から終了までの中央値（ms）であり、judge 上の時間を表すものではない。各入力で warmup を 1 回行い、順序を shuffle して 21 回（chain 幅のみ 15 回）計測した。各出力を公式 checker の `input actual official` 順で検証した。

| 候補 | baseline | 候補 | 参照原本 | 判定 |
|---|---:|---:|---:|---|
| Taylor 入力コピー融合 | 10.725 / 10.605 | 10.702 / 10.590 | 9.328 / 9.158 | 対比較の勝ち数 12/21、13/21。再測定では 11/21、8/21 に低下 |
| Taylor 入出力コピー融合 | 10.823 / 10.690 | 10.728 / 10.704 | 9.396 / 9.270 | 勝ち数 11/21、9/21。入力 01 は退行 |
| 多点評価の境界コピー削減 | 15.760 / 15.679 | 15.674 / 15.568 | 14.863 / 15.045 | 勝ち数 12/21、11/21 |
| 補間の境界コピー削減 | 20.424 / 19.965 | 20.514 / 20.182 | 19.950 / 19.312 | 勝ち数 10/21、10/21。両入力で中央値が退行 |
| Taylor 8 lanes（既存 32） | 10.797 / 11.253 | 10.827 / 10.896 | 9.397 / 9.386 | 勝ち数 6/15、11/15。入力間で傾向が異なる |
| Taylor 16 lanes（既存 32） | 10.797 / 11.253 | 10.685 / 10.880 | 9.397 / 9.386 | 勝ち数 5/15、10/15。入力 00 は対比較で退行 |

中央値同士の差と、同じ反復内の baseline と候補の差の中央値は一致しない。原票には全サンプルと実行順を保存しており、採否には両方を確認した。特に 16 lanes の入力 00 は、中央値だけでは改善に見えるが、対応する反復内の差は中央値で 0.101 ms の退行だった。

## 候補の内容と確認

- `taylor_input.patch`: 公開 `Mint` 配列を factorial weight 計算で直接読む。全 8 要素を読める場合だけ `memcpy` し、端数はゼロ初期化したローカル配列を使う。元のコピー先配列への事前コピーを除く。
- `taylor_both.patch`: 上記に加え、最後の weight 計算から公開配列の生存中の `Mint` オブジェクトへ直接書く。入力を読む計算がすべて終わった後だけ出力を書き、公開配列の padding を読み書きしない。
- `evaluation_boundary.patch`: PointTree が公開座標を直接変換し、division が公開係数の変換と逆順コピーを同時に行う。補間では公開値から既存の内部作業領域へ直接変換し、最後の逆順コピーも省く。公開入力の const 性と既存 raw-word API は維持する。
- 8/16 lanes: Taylor の `kVectors` をそれぞれ 1/2 に変更する。factorial の漸化式と NTT は既存のまま。

Taylor コピー融合は公式 38 ケース、多点評価は 11 ケース、補間は 9 ケースを通過した。コピー融合と境界コピー削減は通常の Werror unit build も通過した。chain 幅の候補は計測対象の最大 2 ケースを検証しており、全ケース検証や採用は行っていない。

コピーを減らすだけでは参照原本との差を安定して埋められなかった。Taylor shift は依然約 1.3–1.9 ms、多点評価は約 0.6–0.9 ms、補間は約 0.5–0.7 ms の差がある。これらは上記 baseline と同条件の参照原本の中央値の差であり、残差の原因を断定する測定ではない。

## 原票と再現

`fps_evaluation_round2/` に各候補の patch、全サンプル・入力 hash・ソース hash を含む JSON、当初の測定スクリプトを保存した。当初のスクリプトは JSON 内の harness hash に対応する記録であり、一時作業パスを含む。今後の再現には次の runner を使う。

```sh
python3 test/benchmark/fps_evaluation_round2.py \
  --output /tmp/fps-evaluation-round2-repeat \
  --trials 21
```

baseline の git object、`/tmp/library-checker-problems` の公式入力・出力・checker、`/tmp/fps-reference/<problem>/<submission>.cpp` を必要とする。`--baseline-tree`、`--official-root`、`--reference-root` で場所を変更できる。`--groups evaluation`、`--groups taylor_fused`、`--groups taylor_vectors` で測定を限定できる。

runner は patch を隔離した baseline に適用し、共有ロックを各ビルドごとに解放する。候補の公式全ケース確認後、排他ロック内で baseline・候補・参照原本を同条件で測定する。この共通 runner は記録用に整理し、構文と patch 適用を確認した。当初の原票は整理前のスクリプトによる実測である。
