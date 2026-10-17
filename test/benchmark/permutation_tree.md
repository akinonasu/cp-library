# Permutation Tree

## 参考実装

2026-10-18 に [Common Interval Decomposition Tree](https://judge.yosupo.jp/problem/common_interval_decomposition_tree) の AC 一覧を実行時間順で確認した。

- [279529 / Richard1211 / 86 ms](https://judge.yosupo.jp/submission/279529)：逆順列の RMQ と候補のスタック。16 要素ブロックの RMQ と高速入出力を使う。
- [252918 / Nachia / 86 ms](https://judge.yosupo.jp/submission/252918)：同系統の構築法。構築部分を取り出した比較と、提出コード全体の比較に使用した。
- [405146 / Rohan_Kapri / 88 ms](https://judge.yosupo.jp/submission/405146)：252918 とソースの SHA-256 が一致する。提出時の時間は異なるが、今回の同条件比較では同じ実装である。
- [作者による解説と議論](https://codeforces.com/blog/entry/78898?locale=en)：単調スタック・遅延セグ木による構築、逆順列 RMQ による構築を確認した。コメントの DSU 案には反例が報告されているため採用しない。
- [hitonanode の実装](https://hitonanode.github.io/cplib-cpp/other_algorithms/permutation_tree.hpp.html)：セグ木を使う構築と API を比較した。

一覧・取得日時・参照ソースのハッシュは [取得記録](results/permutation_tree_reference.json) に保存する。上の時間は公開ジャッジの記録であり、以下のローカル時間とは直接比較しない。

## 実装

逆順列の区間最小値と候補のスタックから区間を生成し、Increasing・Decreasing・Prime の頂点へまとめる。葉と内部頂点の位置区間・値区間・親、左から並ぶ子、幅優先順、全共通区間の個数を保持する。構築と走査順の生成は再帰を使わない。

RMQ は 32 要素の単調スタックをビットマスクで表し、ブロック間だけ Sparse Table を使う。再利用できる `StaticRangeMin` として分離した。ブロック数を `b = ceil(n/32)` とすると、構築時間と一時空間は `O(n + b log b)`、構築後の木は `O(n)` 空間。固定ブロックの Sparse Table を使うため、一般の入力長について厳密な線形時間とはしない。

採用した調整は以下。

1. 値の幅と位置の長さが等しければ、その区間は既に共通区間なので RMQ を省く。
2. 子の CSR 配列は左から葉を訪れ、各内部頂点をその左端の葉から一度だけ親へ接続する。左端によるバケットと連結リスト用配列を省く。
3. 対応コンパイラでは集成体の丸括弧初期化で `Node` を直接 emplace する。GCC の自動変数ゼロ初期化で一時オブジェクトの padding を初期化する費用を避ける。未対応の Apple Clang などには波括弧初期化を残す。

## 測定条件

WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2.0、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native`、assert 有効。ゼロ初期化の比較だけ `-ftrivial-auto-var-init=zero` を追加する。AtCoder 実機での測定ではない。

入力 seed 279529、実行順 seed 37192。各条件 1 回ウォームアップ後、実行順をシャッフルして 7 回測定した。以下は中央値、単位 ms。測定中は他のコンパイル・テストを実行していない。

構築部分は長さ 500000、各試行 3 反復の平均。入力生成を除き、構築と頂点区間・親区間・種類の checksum を含む。参照の 252918 は元の API による入力コピーを含む。一方、参照は親・種類・位置区間だけを構築し、今回のライブラリの子 CSR・値区間・幅優先順・共通区間数を構築しない。機能量の差がある。

公式ケースは実際の検証ドライバーと参照提出の元コードを比較する。Linux `/tmp` の通常ファイルを stdin / stdout に使い、プロセス起動・入力・構築・出力・終了を含む。コンパイル、入力コピー、公式 checker、ハッシュ計算は時間外。ウォームアップの出力を checker で確認し、以降の出力は SHA-256 の一致を確認する。異なる出力が出た場合は再度 checker を実行する。

## 構築部分

`baseline` は RMQ を先に呼び、子をバケット経由で格納する初期版。`packed` は子の格納だけを変更。`aggregate` は最終版と同じ処理だが波括弧による一時 `Node` を使う。`current` が採用版。`reference` は 252918 の構築部分。

| 入力 | baseline | packed | aggregate | current | reference |
| --- | ---: | ---: | ---: | ---: | ---: |
| random | 22.8213 | 21.9321 | 21.9777 | 21.8484 | 19.6578 |
| ascending | 15.1602 | 14.1544 | 13.0189 | 12.6919 | 15.4352 |
| descending | 15.6353 | 14.0323 | 13.1280 | 13.1363 | 15.3773 |
| pairs | 25.2308 | 23.1153 | 22.4786 | 21.9807 | 20.0630 |
| zigzag | 34.2692 | 31.5970 | 30.4830 | 30.4833 | 24.4972 |
| blocks | 19.8788 | 18.4535 | 18.1165 | 18.2823 | 17.6693 |
| recursive | 35.2805 | 33.7884 | 32.7794 | 33.4942 | 27.1416 |

`pairs` は隣接要素を交換、`blocks` は 256 要素ブロック内のランダム順列、`recursive` は二分した区間を再帰的に回転させた separable permutation。`zigzag` は深い木を作る交互の並び。正確な生成は測定プログラムに置く。

初期版に対して約 4.3–16.3% 短縮した。単調な入力では参照より速いが、ほかの入力では参照の構築部分より遅い。全入力での最速は主張しない。

ゼロ初期化ありの結果は以下。自動初期化の有無をまたいだ時間差を改善率とは扱わない。

| 入力 | aggregate | current | reference |
| --- | ---: | ---: | ---: |
| random | 23.4325 | 22.1677 | 19.4663 |
| ascending | 14.3101 | 12.9145 | 15.2470 |
| descending | 14.5458 | 13.0810 | 15.3503 |
| pairs | 24.4467 | 22.2419 | 19.8744 |
| zigzag | 33.6421 | 30.1557 | 24.4118 |
| blocks | 20.2015 | 18.4082 | 17.5799 |
| recursive | 35.1346 | 33.0592 | 27.0721 |

## 入出力を含む公式ケース

| 指標・ケース | current | 279529 | 252918 | 405146 |
| --- | ---: | ---: | ---: | ---: |
| 各ケースの中央値の最大値 | 62.1193 | 56.8025 | 67.4548 | 67.1598 |
| ascending_order_00 | 23.2534 | 27.6721 | 33.9228 | 34.2122 |
| descending_order_00 | 23.1839 | 26.9168 | 33.7669 | 33.7437 |
| max_random_00 | 38.0940 | 36.9721 | 46.6161 | 45.5859 |
| killer_case_00 | 60.2640 | 54.1907 | 66.7619 | 66.5880 |
| killer_straight_00 | 60.4516 | 52.4943 | 66.7508 | 66.5607 |

279529 に対して単調なケースでは短縮するが、killer 系では遅い。今回の実装を fastest とする結果ではない。公開ジャッジへの提出は行っていない。

## 検証と生データ

- 長さ 0..8 の全順列で、独立に列挙した全共通区間・strong interval・種類・親子関係・共通区間数と照合。
- ランダム 1000 個と 30 万要素の単調・ランダム・交互・隣接交換列。空列、再構築、コピー、ムーブも確認。
- RMQ は 32 要素境界を含む全区間、同値、整数・文字列・非デフォルト構築型・bool を確認。
- GCC 13.4 / 15.2、Clang 21.1.8、独立ヘッダー、マクロ非依存、ASan/UBSan、NDEBUG での検証。
- GCC と Clang の両方で公式 30 ケースを公式 checker により確認。波括弧初期化の互換経路も強制して全単体テストを確認。

[通常測定](results/permutation_tree/results.json)・[条件とハッシュ](results/permutation_tree/environment.json)、[ゼロ初期化あり](results/permutation_tree_zero/results.json)・[条件](results/permutation_tree_zero/environment.json)、[Clang と互換経路](results/permutation_tree_clang.json)。

再現手順。Linux の GCC と、生成済みの公式データ・checker を使用する。

```sh
python3 test/benchmark/permutation_tree_reference.py --cache .verify-helper/cache/permutation-tree-reference --output test/benchmark/results/permutation_tree_reference.json
python3 test/benchmark/permutation_tree.py --cache .verify-helper/cache/permutation-tree-benchmark --references .verify-helper/cache/permutation-tree-reference --official /path/to/library-checker-problems/tree/common_interval_decomposition_tree --output test/benchmark/results/permutation_tree
python3 test/benchmark/permutation_tree.py --cache .verify-helper/cache/permutation-tree-benchmark --references .verify-helper/cache/permutation-tree-reference --official /path/to/library-checker-problems/tree/common_interval_decomposition_tree --output test/benchmark/results/permutation_tree_zero --zero-init --core-only
```
