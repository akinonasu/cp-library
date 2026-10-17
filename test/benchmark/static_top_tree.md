# Static Top Tree

## 参考提出・資料

2026-10-17 に Library Checker の AC を実行時間順に取得した。

| 問題 | 提出 | 取得時の時間 | 手法 |
| --- | --- | ---: | --- |
| [Point Set Tree Path Composite Sum](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum) | [380241](https://judge.yosupo.jp/submission/380241) | 294 ms | Rust。Point と Path を分離し、親側に子の値を保持。高さを使うスタックとヒープで構築 |
| 同上 | [405112](https://judge.yosupo.jp/submission/405112) / [378799](https://judge.yosupo.jp/submission/378799) | 297 / 300 ms | 同系統の Rust 実装 |
| 同上 | [369678](https://judge.yosupo.jp/submission/369678) | 328 ms | 取得時の C++ 最上位。頂点と親辺のクラスタを使い、両方向の DP を保持 |
| 同上 | [275016](https://judge.yosupo.jp/submission/275016) / [232761](https://judge.yosupo.jp/submission/232761) | 330 / 341 ms | C++ の上位 AC |
| [同問題の固定根版](https://judge.yosupo.jp/problem/point_set_tree_path_composite_sum_fixed_root) | [205356](https://judge.yosupo.jp/submission/205356) | 152 ms | 取得時の最上位。bit 幅を使った heavy な子の選択と枝の併合。Point と Path を分離 |
| 同上 | [405110](https://judge.yosupo.jp/submission/405110) / [296417](https://judge.yosupo.jp/submission/296417) | 153 / 158 ms | 上位 C++ AC |

[ABC351 G の解説](https://atcoder.jp/contests/abc351/editorial/9910)、[maspy の Static Top Tree](https://maspypy.github.io/library/graph/ds/static_toptree.hpp)、[問題の解説](https://maspypy.com/library-checker-point-set-tree-path-composite-sum) も確認した。

今回の実装は既存の HLD を使い、light な枝を高さの小さい順に併合する。heavy path は高さを使うスタックで連結する。構築・更新・問い合わせに再帰を使わない。構造の構築は値に依存しない内部部品に分離し、実行時には Point と Path を別配列に置いて、親が二つの子の値を直接保持する。

固定根版は片方向の一次関数合成、可変根版は順方向・逆方向を持つ DP で検証する。構造自体は modint・演算の可逆性・特定の CPU 命令に依存しない。

提出一覧・詳細・ソースのハッシュは `results/static_top_tree_reference.json` に保存した。取得したソースはローカルキャッシュに置く。

```sh
python3 test/benchmark/static_top_tree_reference.py --cache .verify-helper/cache/static-top-tree-references --output test/benchmark/results/static_top_tree_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。各条件 7 回、実行順はシャッフル、各実装のウォームアップは 1 回。seed は 761825。測定中は他のテストを実行していない。

全入力は N=200000、Q=200000。頂点更新と辺更新をおよそ半分ずつ行い、更新対象と値、可変根版の根は乱数で選ぶ。値は 998244353 未満。

| 入力 | 頂点 v の親（v > 0） |
| --- | --- |
| random | 0..v-1 から一様に選ぶ |
| chain | v-1 |
| star | 0 |
| balanced | (v-1)/2 の切り捨て |
| broom | v < N/2 なら v-1、それ以外は v-N/2。長い鎖に葉を付ける |

子プロセスの起動・入力・構築・全クエリ・出力・終了を含む実時間。コンパイル・入力生成・出力検査は時間外。比較対象は固定根版が提出 205356、可変根版が提出 369678 で、各試行の出力全体のハッシュを照合した。比較対象を含む全プロセスのスタック上限は unlimited。提出 369678 は再帰 DFS を使い、通常の 8 MiB の上限では長い鎖で失敗したため、この条件を揃えた。

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。提出のソースは変更せず、ソース内の pragma も保持した。提出 205356 は generic でも `avx2,bmi,bmi2,lzcnt,popcnt` を指定している。以下は中央値、単位は ms。AtCoder 実機での速度は未測定。公開ジャッジの時間とは直接比較しない。

| 条件 | 入力 | 固定根・今回 | 固定根・205356 | 可変根・今回 | 可変根・369678 |
| --- | --- | ---: | ---: | ---: | ---: |
| generic | random | 118.430 | 110.022 | 275.973 | 221.242 |
| generic | chain | 109.050 | 88.293 | 229.987 | 191.631 |
| generic | star | 109.200 | 84.073 | 173.017 | 161.330 |
| generic | balanced | 116.420 | 94.330 | 306.823 | 219.822 |
| generic | broom | 110.282 | 90.523 | 209.812 | 209.275 |
| native | random | 119.429 | 108.473 | 289.845 | 224.263 |
| native | chain | 111.009 | 90.724 | 246.292 | 200.431 |
| native | star | 103.976 | 76.719 | 172.355 | 157.913 |
| native | balanced | 109.185 | 86.936 | 317.190 | 258.380 |
| native | broom | 99.386 | 81.718 | 266.004 | 196.971 |

今回の測定では上位の問題専用実装の方が速かった。汎用 DP の更新・部分木・根から指定頂点へのクラスタ取得を備える実装として公開し、最速とは主張しない。native 指定による一貫した改善も見られなかった。全試行・ハッシュ・条件は `results/static_top_tree_generic/results.json` と `results/static_top_tree_native/results.json` に保存した。

```sh
python3 test/benchmark/static_top_tree.py --fixed-reference .verify-helper/cache/static-top-tree-references/205356.cpp --reroot-reference .verify-helper/cache/static-top-tree-references/369678.cpp --output test/benchmark/results/static_top_tree_generic
python3 test/benchmark/static_top_tree.py --fixed-reference .verify-helper/cache/static-top-tree-references/205356.cpp --reroot-reference .verify-helper/cache/static-top-tree-references/369678.cpp --output test/benchmark/results/static_top_tree_native --native
```

## 検証

GCC 15.2・Clang 21.1 と ASan/UBSan で、固定根・任意根の一次関数合成、辺の更新、部分木、重み付き独立集合の DP を素朴な木 DP と比較した。辺の係数や積が 0 になる場合も含む。空の木・1 頂点・根の変更・再構築・親と辺 ID の対応も確認した。

30 万頂点の鎖・星・二分木・鎖に葉を付けた木・ランダム木で、構造の整合性・高さ・更新を確認した。構築順で全クラスタを一度ずつ評価できること、親子の対応、クラスタ数が 2N-1 以下であることも検査した。

Library Checker の固定根版・可変根版はいずれも 26 ケースを GCC・Clang の両方で通過した。GCC 13 でも単体テストを通過した。変更箇所と、複数ヘッダーを同時に使う standalone テストを検証対象にした。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/static_top_tree_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/static_top_tree_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/point_set_tree_path_composite_sum_fixed_root.test.cpp test/point_set_tree_path_composite_sum.test.cpp
```
