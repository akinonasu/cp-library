# Wavelet Matrix の和の境界探索

`min_count_largest(l,r,target)` は、区間内の値を大きい順に選び、和が `target` 以上になる最小個数を返す。二分探索で `sum_largest` を繰り返さず、個数と和を同時に持って値域を一度下る。小さい順の探索と、予算以下の最大個数の探索も追加した。境界探索では値を非負整数に限定する。

静的な `WaveletMatrix` は既存の和の索引を利用する。`WeightedWaveletMatrix` は値を固定して整数の個数を更新する。`PointSetWaveletMatrix` は代入する `(位置,値)` を先に登録し、対応する個数を移動することで一点代入を扱う。後者の更新・探索は、登録数を m、異なる値の数を σ として O(log m log σ)。

ABC467 G の[提出用コード](abc467_g.cpp)では、更新候補の登録後、各クエリを次の二つで処理する。

```cpp
wm.set(c, x);
auto answer = wm.min_count_largest(l, r, k);
```

## 調査

- [ABC467 G の公式解説](https://atcoder.jp/contests/abc467/editorial/23438): 値域を直接下ることで、個数の二分探索による余分な log を除く。添付入力の独立な比較には、[dyc2022 の公式別解](https://atcoder.jp/contests/abc467/editorial/23445)の平方分割コードを使用した。参照コードは `main()` を `int main()` に直しただけで、ブロック幅 500 や入出力は変更していない。
- Library Checker [Point Add Rectangle Sum](https://judge.yosupo.jp/problem/point_add_rectangle_sum) の AC を実行時間順に取得し、上位 5 提出を確認した。[401818 / toomer](https://judge.yosupo.jp/submission/401818) は取得時 115 ms、[404949 / toomer](https://judge.yosupo.jp/submission/404949) は 116 ms の Rust 実装。4 分岐、0 側だけの Fenwick Tree、区間和で共通の祖先を省く構成を比較した。[408282 / ArcAki](https://judge.yosupo.jp/submission/408282) は 132 ms。[206958 / oldyan](https://judge.yosupo.jp/submission/206958) と [206843 / oldyan](https://judge.yosupo.jp/submission/206843) は 151 / 152 ms の C++ 実装で、二次元クエリを Wavelet Matrix に変換している。
- 今回は 4 分岐、個数と `値 * 個数` の和、0 側の Fenwick Tree を組み合わせた。残りの個数が 1 なら和から値が分かるため、その時点で探索を終了する。二点の個数移動は共通の Fenwick 祖先をまとめて更新する。共通部品は `components/fenwick_sum.hpp` に分離した。

取得結果・参照ソースの SHA-256 は [reference.json](results/weighted_wavelet_matrix_reference.json) に保存した。ABC467 G の提出一覧は認証が必要で取得できず、AtCoder 上の最速提出との比較は行っていない。

## 測定条件

Core i7-13700F、WSL2 Ubuntu / Linux x86_64、CPU 2 に固定。GCC 15.2、`-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -DATCODER -DONLINE_JUDGE`。LTO は使用しない。[AtCoder の言語一覧](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)と [2026 年 6 月の更新](https://atcoder.jp/posts/language_update20260616_en)に合わせた設定であり、AtCoder 実機での時間ではない。

毎回新しいプロセスを起動し、入力の解析、更新候補の登録、構築、全更新・全クエリ、Linux の一時ファイルへの出力までを測る。入力生成とコピー、コンパイルは含めない。各組み合わせで 1 回予熱し、順序を乱択して測定する。中央値を記載し、生データ、ソースの SHA-256、環境情報、出力ハッシュ、別実行の最大 RSS を JSON に保存する。

### 添付された全 14 入力

GCC は各 9 回、公式別解は各 3 回測定し、それぞれ 1 回予熱。時間は中央値、RSS は別実行の最大値。

| 入力 | N | Q | 本実装 ms | 公式別解 ms | 本実装 RSS MiB |
| --- | ---: | ---: | ---: | ---: | ---: |
| 00_sample_00.txt | 7 | 5 | 0.682 | 0.707 | 3.49 |
| 00_sample_01.txt | 15 | 10 | 0.670 | 0.671 | 3.57 |
| 01_random_00.txt | 88877 | 82986 | 210.367 | 814.310 | 52.49 |
| 01_random_01.txt | 64843 | 94540 | 226.402 | 751.184 | 51.91 |
| 01_random_02.txt | 52303 | 57106 | 119.796 | 381.774 | 35.36 |
| 01_random_03.txt | 100000 | 100000 | 261.273 | 1104.070 | 60.43 |
| 01_random_04.txt | 100000 | 100000 | 294.838 | 1083.192 | 60.35 |
| 01_random_05.txt | 100000 | 100000 | 273.012 | 1072.100 | 60.32 |
| 01_random_06.txt | 100000 | 100000 | 272.341 | 1106.009 | 60.43 |
| 01_random_07.txt | 100000 | 100000 | 279.504 | 1068.731 | 60.32 |
| 01_random_08.txt | 100000 | 100000 | 22.731 | 180.352 | 17.52 |
| 01_random_09.txt | 100000 | 100000 | 38.733 | 179.173 | 26.46 |
| 01_random_10.txt | 100000 | 100000 | 41.630 | 172.657 | 27.40 |
| 01_random_11.txt | 100000 | 100000 | 71.280 | 186.027 | 33.35 |

全ケースで GCC の出力を公式別解の出力と完全一致で照合する。Clang 21 の汎用ビルドでも同じ入力と出力を確認する。公式の出力ファイルは添付されていないため、これは公式ジャッジでの AC を意味しない。入力そのものはリポジトリに追加せず、ファイル名と SHA-256 を [結果](results/weighted_wavelet_matrix/abc467_g_cases.json) に記録する。

### 生成した ABC467 G 形式の入力

N = Q = 100000。9 回測定、1 回予熱。一般の乱数、全区間、長さ 16 以下、値 256 種、値 2 種を用いた。乱数 seed は 829971。`binary_search` は同じ構築と更新を使い、公開 API の `sum_largest` を二分探索する。

| 入力 | 直接探索 ms | 個数の二分探索 ms |
| --- | ---: | ---: |
| random | 242.001 | 828.767 |
| full | 199.135 | — |
| short | 172.642 | 202.755 |
| dense | 105.123 | — |
| binary | 37.283 | — |

[native の全結果](results/weighted_wavelet_matrix_native.json)。`-march=native` を外した 5 回の測定では random が 268.414 / 899.052 ms、short が 190.540 / 231.755 ms。[汎用設定の全結果](results/weighted_wavelet_matrix_generic.json)。

[候補比較](results/weighted_wavelet_matrix_exploration.json)では初期実装、個数 1 での終了、Fenwick の共通祖先の省略、左右の prefix を独立に計算する方法、それらの組み合わせ、二点更新を融合しない方法を各 5 回比較した。採用構成は初期実装に対して random が 245.005 → 240.566 ms、full が 198.641 → 182.578 ms、short が 195.913 → 173.399 ms。一方、少数の値だけの入力では同じ改善は出ておらず、全入力で速くなるとは主張しない。

### Library Checker の近い問題

Point Add Rectangle Sum の公式全 18 入力について、本実装・Rust 401818・C++ 206958 を公式 checker で確認した。次は各 9 回の中央値。

| 入力 | 本実装 ms | Rust 401818 ms | C++ 206958 ms |
| --- | ---: | ---: | ---: |
| max_random_00 | 243.424 | 74.540 | 123.698 |
| many_points_00 | 196.693 | 66.721 | 120.792 |
| many_queries_00 | 263.362 | 73.760 | 111.528 |
| xy_concentrate_00 | 135.794 | 42.373 | 115.044 |

本実装は個数と `値 * 個数` の 128 bit の和を同時に保持し、参照実装は重みの和だけを保持する。目的と保持する情報が異なり、この問題で最速にはなっていない。C++ は同じ GCC の設定、Rust は rustc 1.97.1、`--edition=2021 -C opt-level=3 -C target-cpu=native`。各プログラムの元の入出力を含む。[全結果](results/weighted_wavelet_matrix_rectangle.json)。

## 検証と再現

- 変更した 4 ヘッダーと依存する 8 unit を GCC 15 / Clang 21 で検証。新規の 4 unit とヘッダーを GCC 13 でも検証。`NDEBUG`、Clang の ASan / UBSan、空区間、0、負の値を含む通常の順位・和、64 bit 境界、大きな個数、未登録の代入、再構築も確認した。[ログ](results/weighted_wavelet_matrix/validation.log)、[記録](results/weighted_wavelet_matrix/validation.json)。
- ABC467 G の公式 2 サンプルと 14400 回の乱択更新・クエリを、ソートによる愚直解と照合した。
- 既存 Wavelet Matrix の 3 検証と、新規の長方形和の検証を GCC / Clang の両方で実行した。[ログ](results/weighted_wavelet_matrix/judge.log)。
- ドキュメントと展開コード、公開関数の表を確認した。[ログ](results/weighted_wavelet_matrix/docs.log)。

測定後に main の FastIO への宣言追加を取り込んだ。提出コードを統合前後に同じ条件でコンパイルし、バイナリ全体の一致を確認した。他の測定対象ソースは変更していない。[統合時の記録](results/weighted_wavelet_matrix/integration.json)。

Linux の GCC / Clang、Python、`online-judge-tools` が必要。測定時は他の重い処理を止める。

```sh
python3 test/benchmark/abc467_g_cases.py --input-dir /path/to/inputs
python3 test/benchmark/weighted_wavelet_matrix.py --output test/benchmark/results/weighted_wavelet_matrix_native.json
python3 test/benchmark/weighted_wavelet_matrix.py --generic --trials 5 --output test/benchmark/results/weighted_wavelet_matrix_generic.json
python3 test/benchmark/weighted_wavelet_matrix.py --explore --snapshot test/benchmark/results/weighted_wavelet_matrix_exploration.json --trials 5 --output /tmp/wm-exploration.json
python3 test/benchmark/weighted_wavelet_matrix_reference.py
python3 test/benchmark/weighted_wavelet_matrix_rectangle.py
```

Library Checker の比較には生成済みの公式入力と checker、および reference スクリプトが保存する提出ソースが必要。`weighted_wavelet_matrix_rectangle.py --problem-root` と `--cpu` で環境に合わせて変更できる。
