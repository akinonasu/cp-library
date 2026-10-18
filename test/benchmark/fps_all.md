# FPS 全体の比較

2026-10-19 JST。比較元は `ce81108fe8e4f3a93d816c94e475d02ef0329608`。公開 API、係数型、非対応環境の計算経路を維持し、Linux x86_64 / GCC / AVX2 の内部処理を更新した。

「同じアルゴリズム」という比較では不十分だった。最速原本には、変換済みの配列を次の更新に渡す専用経路、必要な半分だけを復元する処理、最下段の積との融合、入力を作業配列へ直接読む main が含まれる。一般の FPS 演算へ分解すると、この共有と融合が失われる。今回は専用の生配列 API を components に分離し、公開 API の境界まで接続した。

| 関数 | 不足していた処理と変更 | 参照提出 |
| --- | --- | --- |
| 積・二乗 | 2 本の変換と積・逆変換を同じ部分木で実行。2 群の葉を融合し、半分が零の最上段をまとめる | [409465](https://judge.yosupo.jp/submission/409465) |
| `inv` | 最終 Newton 更新の作業領域を再利用。2 冪を僅かに超える場合は内積で末尾を補完 | [409370](https://judge.yosupo.jp/submission/409370) |
| `log` | 前回導入した 4 ブロック更新に、最上段 radix 8・倍率表・一括結果生成を追加 | [409452](https://judge.yosupo.jp/submission/409452) |
| `exp` | Newton 更新と最後の 2 ブロックを専用化し、変換を保持 | [409662](https://judge.yosupo.jp/submission/409662) |
| `pow` | 対数微分から指数まで変換済み配列と Arena を共有 | [409718](https://judge.yosupo.jp/submission/409718) |
| `sqrt` | 葉の二乗、最終の分割更新、必要な半分の積を融合 | [409728](https://judge.yosupo.jp/submission/409728) |
| `div_series`・`divmod` | 商のブロック長を選択し、残差計算と tail/split/wrap による剰余を専用化 | [409561](https://judge.yosupo.jp/submission/409561) |
| 多項式 GCD・拡張 GCD・逆元 | 必要な行列成分だけを更新し、積和と変換の倍増を融合。Arena を共有 | [409589](https://judge.yosupo.jp/submission/409589) |
| `compose` | pruned Kinoshita–Li 変換、最初の 2 段と最後の 3 段の特殊化 | [409336](https://judge.yosupo.jp/submission/409336)・[409535](https://judge.yosupo.jp/submission/409535) |
| `compositional_inverse` | 射影から冪まで同じ Arena で連結 | [409399](https://judge.yosupo.jp/submission/409399)・[409549](https://judge.yosupo.jp/submission/409549) |
| `power_projection` | 係数抽出・一般重みの初段と後続の変換を共有。零・単項式・stride の既存経路を維持 | 上記の逆合成提出 |
| `multipoint_eval` | 8 lane の木と変換を保持した降下を移植 | [409516](https://judge.yosupo.jp/submission/409516)・[409545](https://judge.yosupo.jp/submission/409545) |
| `interpolate` | 同じ木と分母・値の積を専用処理へ接続。係数の共通因子を保持 | [409545](https://judge.yosupo.jp/submission/409545) |
| `taylor_shift` | 32 本の階乗列と中間積を移植。積の部分木を共有 | [409534](https://judge.yosupo.jp/submission/409534) |
| Bostan–Mori・線形漸化式 | 既存の変換を使い、作業配列を整列確保。改善しなかった新カーネルは不採用 | [405848](https://judge.yosupo.jp/submission/405848) |
| `eval` | 直列 Horner を 8 lane × 4 本へ変更。非 AVX2 は 8 本の scalar Horner | 一点評価の独立した最速提出はなく、直列式と比較 |
| 微分・積分・スカラー演算等 | 既存のベクトル演算を維持。`onemul`・`onediv` の更新順序も維持 | 独立した順位はない |

疎な逆数・指数などは、密な入力用の新経路より前にある既存処理を維持する。多項式 GCD 自体の独立した Library Checker 問題はなく、逆元の公式問題、商と剰余の恒等式、単体テストで確認する。

## 同条件の比較

Yosupo 上の順位表示とローカルの時間を直接比較せず、公開された最速提出のソースを変更せずに同じ機械でビルドする。GCC 14.2、AMD EPYC 9V74、CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。コンパイル・テストと計時を flock で分離し、実行順を混ぜた 9 回の中央値を記録する。1 回の予熱は除外する。

`fps_all_speed.py` は最速原本・比較元・変更後のプロセス全体を比較する。入力・正解・ソース・バイナリ・ヘッダー・ハーネスの SHA-256 と全試行を JSON に保存し、各出力を `checker input actual answer` の順で検証する。平方根は答えが存在する最大入力を選ぶ。公開 FPS 配列の作成、変換、コピー、解放、入出力も時間に含む。

| 処理 | 比較元 ms | 変更後 ms | 最速原本 ms |
| --- | ---: | ---: | ---: |
| 積 | 24.840 | 15.127 | 11.508 |
| 逆数 | 15.996 | 12.878 | 10.793 |
| 対数 | 16.868 | 15.355 | 13.393 |
| 指数 | 25.689 | 17.229 | 15.178 |
| 冪 | 37.996 | 25.282 | 23.010 |
| 平方根 | 19.264 | 13.205 | 11.079 |
| 多項式除算 | 10.276 | 7.040 | 5.430 |
| 線形漸化式 | 59.291 | 56.516 | 58.736 |
| 合成（通常） | 3.720 | 3.068 | 3.081 |
| 合成（large） | 56.050 | 33.731 | 33.010 |
| 合成逆元（通常） | 3.287 | 2.579 | 2.603 |
| 合成逆元（large） | 51.446 | 30.851 | 30.421 |
| 多点評価 | 52.307 | 16.676 | 15.895 |
| 補間 | 74.847 | 20.439 | 19.773 |
| Taylor shift | 24.479 | 11.673 | 9.359 |
| 多項式逆元 | 47.481 | 15.317 | 14.975 |

線形漸化式と通常サイズの合成・合成逆元は原本と同等以上の中央値だった。large の合成・合成逆元、評価・補間は近づいたが、全関数で最速原本と同等には達していない。積・逆数・対数・指数・冪・平方根には約 2〜4 ms の差が残る。原本は入出力と同じ生配列を破壊的に更新するため、非破壊の公開 API に必要な係数配列・コピーも差になる。

通常の合成は短いプロセスの時間変動が大きかったため、同じバイナリを使い 21 試行へ増やした。全試行と再確認理由は `results/fps_all_compose_recheck.json`、正式な 16 問の値は `results/fps_all_speed.json` にある。順位はローカルの同条件比較であり、実際の judge での最速獲得を断定しない。

計算部分の補助測定と候補の採否は、[exp/pow](fps_exp_backend.md)、[逆数・平方根](fps_inverse_sqrt.md)、[除算・GCD](fps_division.md)、[合成・射影](fps_composition_speed.md)、[評価・補間・Taylor shift](fps_evaluation.md)、[線形漸化式](fps_bostan_speed.md)、[一点評価](fps_horner.md) を参照。

```sh
python3 test/benchmark/fps_all_speed.py --baseline ce81108 \
  --output test/benchmark/results/fps_all_speed.json
python3 test/benchmark/fps_tuning_selection.py --baseline ce81108 \
  --output test/benchmark/results/fps_all_selection.json
python3 test/benchmark/fps_tuning_selection.py --baseline ce81108 \
  --ignore-access-declarations \
  --output test/benchmark/results/fps_all_judge_selection.json
python3 test/benchmark/fps_speed_verify.py \
  --official-root /tmp/library-checker-problems \
  --selection test/benchmark/results/fps_all_judge_selection.json \
  --output test/benchmark/results/fps_all_judge.json
```

参照ソースは `fps_tuning_reference.py --cache /tmp/fps-reference --output reference.json` で取得できる。使用した提出 ID とソースハッシュの原票は `results/fps_speed_reference.json` にある。GCD 単独の問題名を指定していた従来の取得候補は修正し、上位 C++ 提出だけを取得するようにした。

`fast_io.hpp` 自体の差分は BulkOutput の前方宣言と friend 宣言のみで、既存の整数入出力の実装・型の配置は変えていない。全依存先の一覧は `fps_all_selection.json` に保存し、既存 I/O の単体も実行した。公式問題の選択では、宣言を取り除けば比較元と完全一致することをスクリプトで検査してから、動作変更のある FPS 側の依存先を選ぶ。

最終の出力は 64 係数単位で生成し、新規の大きな vector 領域の完全所有ページだけを Linux に準備させる。50 万語の cold 出力は 0.424→0.267 ms、1,048,575 語は HUGEPAGE 併用で 1.076→0.483 ms。syscall・確保・破棄込みの新規プロセス 21 試行で確認した。条件と失敗時の通常動作は [内部 API](../../docs/fps_output.md)、原票は `results/fps_output_pages.json` を参照。
