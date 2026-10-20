# 定数経路の分岐配置の比較

定数経路を含む最終版の分岐に `[[unlikely]]` だけを追加した。新たに1本だけビルドし、[最終比較](fps_convolution_constant_final_round4.md)の基準・最終版・未編集原本の3本は SHA-256 が一致する同一 binary を再利用した。全4ケースで21回と予備実行1回、順序を乱択、CPU2/EX、同じGCC14.2/flagsで比較し、352出力が公式checkerを通過した。

| 入力 | 基準 | 最終版 | unlikely | 原本 | 最終版−unlikely paired |
| --- | ---: | ---: | ---: | ---: | ---: |
| `max_random_00` | 13.216 | 12.884 | 13.036 | 10.177 | -0.066 |
| `max_random_01` | 12.880 | 12.784 | 12.859 | 10.050 | -0.039 |
| `all_same_00` | 13.875 | 9.723 | 9.808 | 8.737 | -0.191 |
| `all_same_01` | 12.779 | 8.653 | 8.577 | 9.253 | -0.037 |

最大ランダム2件を改善せず、ヒントは採用しなかった。最終版の dispatch は測定済みの noinline 版から変更していない。

同一binaryでの基準−最終版のpaired中央値は、最大ランダム00/01で +0.193/+0.082ms だった。先の13ケース比較では −0.164/−0.209ms で、小さい差の方向が逆転している。両セッションを保持し、均等な最大入力で安定した改善とは扱わない。

[原票](results/fps_convolution_branch_round4/result.json)、[実行script](results/fps_convolution_branch_round4/measure.py)、[差分](results/fps_convolution_branch_round4/unlikely.patch)、[親sourcepack・同一binaryの照合](results/fps_convolution_branch_round4/audit.json)、[manifest](results/fps_convolution_branch_round4/manifest.json)。sourceは親の最終sourcepackに差分1箇所を適用して復元できる。
