# FPS 合成の根表共有

比較元は `144ae32387675fa2a889ea3e8cd19b03ad5016bf`。2026-10-18 UTC に Linux x86_64、AMD EPYC 9V74、GCC 14.2.0 で測定した。CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効。共通 lock の排他区間で計時し、コンパイルと検証は共有 lock を使う。

参照は通常合成 [409336](https://judge.yosupo.jp/submission/409336)、large 合成 [409535](https://judge.yosupo.jp/submission/409535)、通常合成逆元 [409399](https://judge.yosupo.jp/submission/409399)、large 合成逆元 [409549](https://judge.yosupo.jp/submission/409549)。原本 SHA-256 は [既存資料](fps_composition_speed.md) と各測定 JSON に記録している。原本を改変せず同じ flags でコンパイルしたローカル比較であり、judge の実測値や順位を示すものではない。

## 採用した変更

合成では共通 `Transform` と専用の `Tables` が根・逆根の表を重複して生成していた。native 経路で `Transform` を専用列変換にも足りる大きさへ広げ、`Tables` はその表を借りる。表の配置と生成関数は共通で、大きい表の先頭は従来の表と一致する。raw API は従来通り独立した表を生成し、公開 API、raw 関数の署名と scratch 計算、汎用係数型や他環境の経路は変えない。

借用先は同じ Arena 内で計算が終わるまで生存し、参照する表は変更しない。従来の Tables 用領域は予約したままとし、その領域の初期化と表生成を省く。このため仮想 Arena サイズは Transform の拡大分増える。利用者の配列に整列や padding を要求せず、入力も変更しない。

### 合成のプロセス全体

通常は公式 `max_random_00` の 8,000 項、large は 131,072 項。プロセス起動から終了までをファイル入出力込みで計り、各出力を `checker input actual answer` で検証する。予備実行 1 回後、候補と比較元・原本の順をシャッフルした各 21 回の中央値、単位 ms。

| 計測 | 通常 比較元 | 通常 表共有 | large 比較元 | large 表共有 |
| --- | ---: | ---: | ---: | ---: |
| 最初の 21 回 | 2.853 | 2.759 | 34.204 | 33.678 |
| 別 seed の 21 回 | 2.813 | 2.765 | 33.863 | 33.675 |
| 5 候補の同一 shuffle | 2.843 | 2.779 | 33.906 | 33.848 |

3 組で改善の方向は同じだが、large の差は 0.17〜1.54% と小さく、実行ごとの変動もある。最後の同一 shuffle での原本は通常 2.814 ms、large 32.827 ms であり、large の原本との差は残る。統合後の正式比較は総合ベンチマークを参照する。

[初回](results/fps_composition_round2_tableonly.json)、[再測定](results/fps_composition_round2_tableonly_repeat.json)、[同一 shuffle](results/fps_composition_round2_compare.json) に全試行・順番・source/header/binary/input/output のハッシュを保存した。測定時の table-only snapshot は射影・合成逆元の候補も含む。最終版はそれらを撤回し、合成の表共有だけを残す。[最終 header パッチ](results/fps_composition_round2_final.patch) と統合後の正式比較で対象を区別する。

## 不採用の候補

外側係数の正規化と逆順配置を融合して中間配列を一つ省く案は、large の独立した中央値が 34.581→34.164 ms、33.911→34.259 ms と変動し、通常版にも退行があったため撤回した。[初回](results/fps_composition_round2_outer.json)・[再測定](results/fps_composition_round2_outer_repeat.json)・[パッチ](results/fps_composition_round2_outer.patch) を残す。

共有した Tables の作業領域も削る案を調べた。全領域を削る compact と、後続配列の位置を保つため guard/padding の 80 words を残す padding を比較した。同一 shuffle の通常は table-only 2.779 / compact 2.763 / padding 2.844 ms、large は 33.848 / 33.868 / 33.625 ms だった。単回で最も速い候補は次数によって変わり、別測定も含め両サイズで同じ方向に改善した table-only に絞った。仮想領域削減のためにサイズ別の選択を追加しない。

[共有＋入力融合](results/fps_composition_round2_tables.json)、[compact](results/fps_composition_round2_tight.json)・[再測定](results/fps_composition_round2_tight_repeat.json)・[確認](results/fps_composition_round2_tight_confirm.json)、[padding](results/fps_composition_round2_padding.json)・[再測定](results/fps_composition_round2_padding_repeat.json)、[直接比較](results/fps_composition_round2_compare.json) を保存した。各候補の差分は同じ名前の `.patch` にある。

合成逆元の表共有も確認した。large の compact は 30.215→30.233、31.055→31.027、31.087→31.293 ms で、安定した短縮を確認できなかったため元の実装を維持する。

### 射影の確認

射影は既存の `fps_tuning_advanced.cpp` で入力を生成し、公開 API の確保・係数変換・結果構築を含めて計る。21 回の中央値で、外側はプロセス全体、括弧内は API の計算区間、単位 ms。チェックサムを比較元と照合する。

| 処理 | 項数 | 比較元 | 表共有候補 |
| --- | ---: | ---: | ---: |
| projection | 32,768 | 7.375 (5.377) | 7.086 (5.192) |
| projection | 131,072 | 28.193 (24.437) | 28.651 (24.485) |
| projection_weighted | 32,768 | 7.156 (5.291) | 7.266 (5.314) |
| projection_weighted | 131,072 | 30.122 (26.338) | 30.149 (26.314) |

大きい次数や一般の重みで安定した利益を確認できず、射影の変更は撤回した。[原票](results/fps_composition_round2_projection.json) を保存した。

## 検証と再現

`fps_composition_tables` は小さい表から最大 $$m=2^{21}$$ まで、独立して生成した根・逆根と借用した表の一致、空の作業領域での借用、表が足りない場合の独立生成を確認する。既存 `fps_composition_blocked` に 16,385 項を加え、2 冪をまたぐ次数で raw API、合成・重み付き射影の双対性、合成逆元の両側の恒等式、入力保存、汚した作業領域を確認する。最終版の通常・ASan/UBSan は [記録](results/fps_composition_round2_check.json) と [ログ](results/fps_composition_round2_check.log) を参照する。統合後の公式ケースは総合検証に含める。

```sh
mkdir -p /tmp/cp-fps-round2-base
git archive 144ae32387675fa2a889ea3e8cd19b03ad5016bf | tar -x -C /tmp/cp-fps-round2-base
python3 test/benchmark/fps_composition_round2.py --work /tmp/fps-composition-round2-final --output composition.json --operations compose compose_large
python3 test/benchmark/fps_composition_round2_repeat.py --manifest composition.json --output composition-repeat.json --seed 20261019
python3 test/benchmark/fps_composition_round2_check.py
```

不採用候補は固定比較元へ対応する `.patch` を適用し、`--candidate-root` と `--overlay components/fps_composition.hpp components/fps_reversion.hpp` で測る。`--reuse-controls` は既存 manifest の比較元と原本の binary hash、source hash、compiler、flags を確認して再利用する指定で、再ビルドなしの直接比較も同じ binary hash を照合する。射影と 5 候補の比較用 harness は `fps_composition_round2_projection.py`・`fps_composition_round2_compare.py` にある。

初期の測定に使った harness の原文は `results/fps_composition_round2_harness_v1.source`・`fps_composition_round2_harness_v2.source` に保存し、各 JSON の `harness_sha256` と一致を確認した。現行 harness は候補ディレクトリ指定と controls の再利用を追加したもので、計時区間は同じである。
