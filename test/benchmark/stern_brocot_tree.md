# Stern–Brocot Tree

## 参考提出・資料

2026-10-17 に [Stern–Brocot Tree](https://judge.yosupo.jp/problem/stern_brocot_tree) の AC を実行時間順に取得した。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [381187](https://judge.yosupo.jp/submission/381187) | 31 ms | Rust、32 bit。連分数で経路を圧縮し、境界の分数対で頂点を表現 |
| [405753](https://judge.yosupo.jp/submission/405753) | 35 ms | 取得時の C++ 最上位。境界との外積を使って移動回数をまとめる。祖先は圧縮経路を作ってから辿り直す |
| [327748](https://judge.yosupo.jp/submission/327748) | 36 ms | 405753 とソースの SHA-256 が一致するため、測定では重複させない |
| [327746](https://judge.yosupo.jp/submission/327746) | 36 ms | 405753 と同系統。探索方向の判定に下限との比較を追加 |

[maspy の実装](https://maspypy.github.io/library/nt/stern_brocot_tree.hpp) と [Library Checker の問題文](https://github.com/yosupo06/library-checker-problems/blob/master/number_theory/stern_brocot_tree/task.md) も確認した。提出一覧・詳細・取得日時・ソースハッシュは `results/stern_brocot_tree_reference.json` に保存した。取得したソースはローカルキャッシュに置く。

今回の実装はユークリッド互除法で連続する同方向の移動回数を取り出す。外積を使う探索や GCD の別計算を行わず、約分されていない入力も受け取る。処理を共有するイテレータを `components/stern_brocot_path.hpp` に分離した。

頂点は上下限の分数と深さだけを持つ。祖先・LCA は経路配列を作らず、必要な深さや共通部分で打ち切る。親への 1 回の移動は除算なし、一括の上方移動は末尾の同方向移動をまとめて戻す。深さが大きくても辺を 1 本ずつ辿らない。

左右への移動には幅の広い整数型で表現範囲の検査を行う。32 bit 型なら 64 bit、64 bit 型なら 128 bit の中間積を使う。分子・分母が最大の unsigned 64 bit 値まで使える。経路の連長や深さは unsigned、二頂点間の距離は必要に応じて 128 bit で返す。

```sh
python3 test/benchmark/stern_brocot_tree_reference.py --cache .verify-helper/cache/stern_brocot_tree_reference --output test/benchmark/results/stern_brocot_tree_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。各条件 7 回、順番をシャッフルし、計測前に各実装を 1 回実行した。seed は 519214。計測中に他のテストは実行していない。

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。参照提出のソースは変更していない。今回の 32 bit 版・64 bit 版は同じドライバを使う。32 bit 版は公式制約に合わせ、64 bit 版は既定の型に合わせたもの。Rust 提出はローカルでは測定していない。

入力はパイプで渡す。起動・入力・全操作・全出力・終了を含む実時間を計測した。コンパイル・入力生成・出力検査は時間外。全試行で出力全体を比較した。各入力は 100000 クエリ、分子・分母は 10^9 以下の既約分数。

| 入力 | 内容 |
| --- | --- |
| encode_path / decode_path / lca / ancestor / range | それぞれの操作のみ。分子・分母は乱数。ancestor は存在する場合と存在しない場合を半分ずつ含む |
| mixed | 5 種類の操作を順に繰り返す |
| skewed | 一方が 1 の分数で、長い同方向移動を含む混合入力 |
| fibonacci | 大きい連続 Fibonacci 数の分数で、圧縮経路の区間数が多い混合入力。LCA は同じ頂点同士で最後まで比較する |

以下は中央値、単位は ms。

| 条件 | 入力 | 今回・32 bit | 今回・64 bit | 405753 | 327746 |
| --- | --- | ---: | ---: | ---: | ---: |
| generic | encode_path | 22.072 | 23.754 | 35.588 | 34.710 |
| generic | decode_path | 25.856 | 26.435 | 27.473 | 28.046 |
| generic | lca | 11.053 | 11.075 | 14.108 | 13.265 |
| generic | ancestor | 14.031 | 14.973 | 24.362 | 24.164 |
| generic | range | 17.225 | 18.835 | 27.224 | 26.971 |
| generic | mixed | 18.250 | 19.277 | 26.419 | 25.827 |
| generic | skewed | 8.341 | 8.381 | 11.273 | 11.181 |
| generic | fibonacci | 29.437 | 32.120 | 42.156 | 42.316 |
| native | encode_path | 23.228 | 24.717 | 36.678 | 37.109 |
| native | decode_path | 27.338 | 26.497 | 29.642 | 29.463 |
| native | lca | 10.944 | 11.030 | 13.913 | 14.083 |
| native | ancestor | 13.949 | 14.591 | 23.639 | 23.879 |
| native | range | 17.625 | 19.416 | 27.749 | 27.657 |
| native | mixed | 18.637 | 19.798 | 26.496 | 26.834 |
| native | skewed | 8.275 | 8.632 | 11.285 | 11.305 |
| native | fibonacci | 29.615 | 32.518 | 42.728 | 41.802 |

今回の環境では、測定した入力で比較対象の上位 C++ AC より速かった。native 指定が一貫して有利なわけではなかった。AtCoder 実機では未測定で、公開ジャッジの時間とは直接比較しない。

全試行・入力ハッシュ・出力ハッシュ・ソースハッシュは `results/stern_brocot_tree_generic/results.json` と `results/stern_brocot_tree_native/results.json` に保存した。取得記録のハッシュは元ソースのバイト列、測定記録のハッシュは CRLF を LF に揃えたもの。

```sh
python3 test/benchmark/stern_brocot_tree.py --output test/benchmark/results/stern_brocot_tree_generic
python3 test/benchmark/stern_brocot_tree.py --native --output test/benchmark/results/stern_brocot_tree_native
```

## 検証

GCC・Clang の単体テスト、Clang の ASan/UBSan、GCC 13 の単体テストを通過した。経路は通常の連分数と最終項の補正から独立に求め、深さ 10 までの全頂点の移動列とも照合した。65 以下の分子・分母の全組合せ、乱数の unsigned 64 bit 値、最大値付近、連続 Fibonacci 数、既約でない入力を検証した。

移動・親・祖先・LCA・距離・包含・上下限・経路の往復を検査した。0 回の移動、根を越える上方移動、表現不能な左右移動、decode の不正な方向とオーバーフローも含む。最大深さ UINT64_MAX-1 と、それを左右に持つ頂点間の 128 bit の距離を検査した。小さい整数型・符号付き整数型・Fraction との併用・複数ヘッダーを含む standalone も確認した。

公式の入力・出力・チェッカーを使うローカル検証で、Stern–Brocot Tree の 18 ケースを GCC・Clang の両方で通過した。公開ジャッジへの提出ではない。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/stern_brocot_tree_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/stern_brocot_tree_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/stern_brocot_tree.test.cpp
```
