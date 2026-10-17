# Link-Cut Tree の GCC 高速化

2026-10-16 に Linux x86_64 / GCC 15.2 で測定した。比較元は
`23a941fc6d336cfd7d1c7d41917b4192a6612ead` の
`data_structure/link_cut_tree.hpp`（Git blob
`d0bb5ec1beed9ed02d6141c98c0a988bd85b3818`）。再現用に
[`link_cut_tree_baseline.hpp`](link_cut_tree_baseline.hpp) にも保存した。

## 参考にした AC 提出

Library Checker の公式 API の `status=AC&order=%2Btime&limit=4` で両問題の上位を取得し、ソースを調べた。
下記の時間は取得時の judge の表示値であり、今回のローカル測定値とは比較しない。
取得した一覧、提出情報、ソースの SHA256 は
[`references.json`](results/link_cut_tree_reference/references.json) に保存した。

| 問題 | 提出 | ユーザー | judge 表示時間 |
| --- | --- | --- | ---: |
| Vertex Add Path Sum | [270626](https://judge.yosupo.jp/submission/270626) | chaihf | 0.174 s |
| 同上 | [405115](https://judge.yosupo.jp/submission/405115) | Rohan_Kapri | 0.176 s |
| 同上 | [278244](https://judge.yosupo.jp/submission/278244) | chaihf | 0.178 s |
| 同上 | [332878](https://judge.yosupo.jp/submission/332878) | nandhagk | 0.182 s |
| Vertex Set Path Composite | [270645](https://judge.yosupo.jp/submission/270645) | chaihf | 0.237 s |
| 同上 | [278247](https://judge.yosupo.jp/submission/278247) | chaihf | 0.242 s |
| 同上 | [405116](https://judge.yosupo.jp/submission/405116) | Rohan_Kapri | 0.245 s |
| 同上 | [334740](https://judge.yosupo.jp/submission/334740) | nandhagk | 0.247 s |

最速の 270626 / 270645 では、子の向きをキャッシュして根判定を軽くし、`access` の途中で対象頂点を回転して持ち上げていた。
値更新にも `access` を使わず、`splay` を使っていた。270645 は補助木の根の集約更新を省略し、経路クエリを左部分木と自身の値から計算する。
上位の提出は近い構造を使っており、334740 は 270645 と同じソースの SHA256 だった。

これらの提出には固定長配列、`mmap` / SWAR による入出力、`O3` 指定も含まれる。
閉路を作る `link` の検査は省略され、合成関数の実装は前後で共通の係数を共有するなど問題に特化している。
本ライブラリでは汎用モノイドと既存の検査付き API を維持した。

## 採用した変更と不変条件

1. `parent > 0` は補助木内の親、`parent < 0` は仮想辺の親、`parent == 0` は親なしとした。根判定のために親の子配列を読む必要がなくなり、ノードへのフィールド追加も不要になる。
2. `access(x)` は最初に `splay(x)` し、仮想辺を上がるたびに親を splay してから `x` を1回回転する。各部分木で集約更新してから最後に再度 `splay(x)` する処理を省く。`lca` 用の戻り値には、最後に通った仮想辺の親を保持する。
3. `set` は `splay` 後に値だけ変更する。仮想辺をたどる `access` は不要。
4. **補助木の根の `prod` / `back` / `count` は最新とは限らない。根以外では常に正しい。** 回転で根から降りる頂点を `pull` してこの不変条件を保つ。`access` 後は右部分木が空なので、`prod` は左部分木と自身の値から、`path_size` と `jump` の範囲判定は左部分木の個数から求める。

反転は子の交換と遅延フラグの伝播を引き続き行う。補助木の根で集約値が古くても、その値を直接参照せず、子になる際に再計算する。
非可換モノイドの向き、閉路になる `link`、存在しない辺の `cut`、LCA、経路上の頂点検索、コピー・ムーブの仕様は維持した。
新しいコンパイラ拡張や CPU 命令への依存は追加していない。各操作の償却計算量も変わらない。

## 候補の比較

[`link_cut_tree_variants.py`](link_cut_tree_variants.py) で変更を個別に生成した。
40,000 頂点・60,000 操作、後述の30条件、各3回の中央値を用い、条件ごとの速度比を幾何平均した。
速度比は `改修前の操作時間 / 候補の操作時間`。

| 候補 | 速度比 |
| --- | ---: |
| 値更新を splay に変更 | 1.314 |
| access の回転を統合 | 1.186 |
| 根判定を親の符号に変更 | 1.109 |
| 子の向きを別フィールドに保持 | 1.053 |
| 値更新 + access 統合 | 1.572 |
| 値更新 + 符号による根判定 | 1.498 |
| 値更新 + 子の向きキャッシュ | 1.380 |
| 値更新 + access 統合 + 符号による根判定 | 1.816 |
| 直前の候補に GCC O3 指定を追加 | 1.828 |

別の3回測定で、最後の候補が 1.835 倍、さらに根の集約更新を省いた候補が 1.938 倍だったため、後者を採用した。
子の向きのキャッシュはフィールドと更新処理が増えるため採用しなかった。
比較した O3 指定の追加効果は小さく、本ヘッダーには指定を追加していない。
探索の全標本と各条件の結果は
[`link_cut_tree_exploration`](results/link_cut_tree_exploration/) と
[`link_cut_tree_lazy_exploration`](results/link_cut_tree_lazy_exploration/) にある。

## 大規模な処理本体の測定

- Intel Core i7-13700F、WSL2 Linux x86_64、GCC 15.2、CPU 2 に固定。
- `-std=c++2b -O2 -Wall -Wextra -Werror`。native 設定ではさらに `-march=native -ftrivial-auto-var-init=zero`。assert は有効。
- 200,000 頂点・200,000 操作。鎖、星、二分木、ランダムな木、ほうき型の5形状。
- 和は `long long`、非可換な関数合成は `math/mint.hpp` と `array<mint, 2>`。
- `query`: 経路クエリ100%。`mixed`: クエリ50%、値更新30%、辺の付け替え20%。`update`: クエリ10%、値更新90%。乱数による選択なので比率は期待値。
- 付け替えでは初期木の葉を切り離し、初期木で葉でない頂点へ接続する。任意の内部辺の付け替えは別途公式ケースで確認する。
- 生成と入出力は計測外。初期構築と操作列の時間を分離する。候補の実行順を固定 seed でシャッフルし、各7回の中央値を採用。
- 全クエリの出力配列を毎回バイト単位で比較する。チェックサムだけの比較ではない。

表は5形状における速度比の幾何平均。個別の時間・速度比・構築時間は CSV にある。

| モノイド | 操作 | 汎用 GCC 設定 | native 設定 |
| --- | --- | ---: | ---: |
| 和 | query | 1.438 | 1.437 |
| 和 | mixed | 1.553 | 1.545 |
| 和 | update | 2.914 | 2.842 |
| 関数合成 | query | 1.455 | 1.407 |
| 関数合成 | mixed | 1.637 | 1.536 |
| 関数合成 | update | 3.171 | 3.036 |

同じ木形状でも更新比率によって効果が異なる。たとえば汎用設定の和の `update` は 1.403～5.775 倍だった。
測定値を AtCoder や Library Checker の実行時間と同一視しない。

結果:
[`generic`](results/link_cut_tree_generic/summary.csv)、
[`native`](results/link_cut_tree_native/summary.csv)。
各ディレクトリの `samples.csv` に全標本、`environment.json` にコンパイラ・フラグ・CPU・ソースの SHA256 を保存した。

## 公式入力と最速提出のローカル比較

両問題の `almost_line_00` / `almost_line_01` / `max_random_00` を使用した。
各 200,000 頂点・200,000 操作で、任意の内部辺の切断・接続を含む。
既存の judge 用ドライバで改修前後を比較し、各問題の最速提出も原文のままコンパイルした。
入力を Linux の一時ディレクトリへコピーしてから実行し、Windows マウントの入力読み込み時間を含めない。
CPU 2 固定、`-std=c++23 -O2 -Wall -Wextra`、実行順をシャッフルした7回の中央値。
プログラム起動・構築・入出力を含む経過時間であり、上の処理本体だけの測定とは異なる。
最速提出内の `NDEBUG` / `O3` / 入出力コードは変更していない。全出力を公式解答と比較した。

| 問題 | 入力 | 改修前 | 改修後 | 速度比 | 最速提出の原文 |
| --- | --- | ---: | ---: | ---: | ---: |
| 和 | almost_line_00 | 220.07 ms | 182.32 ms | 1.207 | 124.06 ms |
| 和 | almost_line_01 | 222.33 ms | 185.83 ms | 1.196 | 125.29 ms |
| 和 | max_random_00 | 196.52 ms | 139.31 ms | 1.411 | 93.57 ms |
| 関数合成 | almost_line_00 | 332.98 ms | 276.25 ms | 1.205 | 152.67 ms |
| 関数合成 | almost_line_01 | 329.74 ms | 277.46 ms | 1.188 | 153.02 ms |
| 関数合成 | max_random_00 | 326.70 ms | 220.96 ms | 1.479 | 70.82 ms |

元の提出と同じく、最速コードには検査の省略、問題固有の集約方法、専用の入出力が含まれる。
この比較だけから最速提出との差の原因を一つに限定することはできない。
本ライブラリの改修前後では同じドライバと検査付き API を用いている。
全標本は [`samples.csv`](results/link_cut_tree_reference/samples.csv)、
中央値は [`summary.csv`](results/link_cut_tree_reference/summary.csv) に保存した。

## 正当性と移植性

- 全候補について、既存の Link-Cut Tree 単体テストを GCC で実行した。和・文字列を用いた素朴解とのランダム比較、全公開操作、無効な link/cut、20万頂点の鎖を含む。
- 採用実装は GCC 15.2 / Clang 21.1.8 でヘッダー単独コンパイル、`test/unit/link_cut_tree.cpp` と推移的に依存する `test/unit/standalone.cpp` を実行した。
- Clang の AddressSanitizer / UBSan で Link-Cut Tree 単体テストが成功した。
- ARM64 GCC 15.2 で静的リンクし、QEMU aarch64 上で同じ単体テストが成功した。
- Library Checker の公式ケースは和15件、関数合成23件について GCC / Clang の両方が AC、計76回成功した。
- Mac 実機での追加実行はしていない。Apple Clang は既存の macOS portability CI で確認する。今回追加した実装は標準 C++ のみを使う。

対象の選択は [`link_cut_tree_selection.json`](results/link_cut_tree_selection.json)、
結果は [`unit`](results/link_cut_tree_unit_check.log)、
[`sanitize`](results/link_cut_tree_sanitize_check.log)、
[`judge`](results/link_cut_tree_judge_check.log)、
[`validation`](results/link_cut_tree_validation.json) に記録した。

## 再現

Linux 上のリポジトリルートで実行する。CPU 番号は `--cpu` で変更できる。
最速提出のソースは `--source-dir` にキャッシュし、リポジトリには取り込まない。
`oj` と GCC / Clang は既存の検証環境を用いる。

```sh
python3 test/benchmark/link_cut_tree_variants.py /tmp/lct-variants --check
python3 test/benchmark/link_cut_tree.py --output /tmp/lct-generic --variants baseline set_signed_fused current --trials 7
python3 test/benchmark/link_cut_tree.py --output /tmp/lct-native --variants baseline current --trials 7 --native
python3 test/benchmark/link_cut_tree_reference.py --output /tmp/lct-reference --source-dir /tmp/lct-sources --data-root .verify-helper/cache/lct-data --trials 7
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/link_cut_tree_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/link_cut_tree_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/link_cut_tree_sum.test.cpp test/link_cut_tree_composite.test.cpp
```

ヘッダーの内部表現を変更したため、既存に成功済みの無関係なライブラリのテストは再実行していない。
測定用ブランチの作成・commit・push は行っていない。
