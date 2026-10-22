# Functional Graph

2026-10-22。移動先配列を受け取り、各弱連結成分を閉路と流れ込む木に分解します。閉路情報、k 回移動、到達距離、逆向きの森での LCA と部分木の区間を提供します。

## 手法と参考実装

[Nyaan's Library の FunctionalGraph](https://github.com/NyaanNyaan/library/blob/master/graph/functional-graph.hpp) を確認しました。Union Find で閉路を閉じる辺を見つけ、その辺を除いた木を仮想頂点へつなぎます。代表頂点の伝播には再帰 DFS を使います。

今回の実装は入次数 0 の頂点を取り除き、残った閉路を列挙します。除去時に部分木サイズと heavy な子を求め、除去順の逆順で閉路の ID・最初の到達点・距離を伝播します。閉路上の辺は全て除いた森として扱います。既存の `data_structure/advanced/hld.hpp` と同様に、heavy な子を親の直後、light な子の部分木を親の区間の後ろから割り当てます。このため、再帰・Union Find・逆向き隣接リスト・ダブリング表を持たずに線形時間・空間で構築できます。

Library Checker の最新の公式問題一覧を確認した時点では、Functional Graph の分解や任意回数の successor に直接対応する問題はありません。そのため、木部分の `lca` と `jump` を検証できる [Jump on Tree](https://judge.yosupo.jp/problem/jump_on_tree) の AC 時間順の先頭 3 件を調査しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [403437 / nandhagk](https://judge.yosupo.jp/submission/403437) | 56 ms | 葉の除去、経路の prefix 情報、小さい部分木の祖先ビット集合、パス・星の専用処理 |
| [405027 / Rohan_Kapri](https://judge.yosupo.jp/submission/405027) | 59 ms | 同系統の木専用実装 |
| [270103 / mukundan314](https://judge.yosupo.jp/submission/270103) | 71 ms | 葉の除去による DFS 順、RMQ、質問を頂点ごとにまとめたオフライン先祖検索 |

一般の Functional Graph でオンラインに質問できる API とするため、今回は HLD を採用しました。閉路に到達する移動は閉路長で剰余を取り、到達距離は森の部分木区間と閉路内の位置で求めます。最速提出や Nyaan 版との時間比較、公開 judge への提出は行っていません。取得した一覧・ソース・公式問題一覧の SHA256 は `results/functional_graph/references.json` に保存しています。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2、`-std=c++23 -O2 -DNDEBUG`。頂点数・質問数ともに 50 万で、CPU 0 に固定し、ウォームアップ 1 回後に 7 回測定した中央値です。

構築は入力配列のコピー、全情報の前処理、結果の解放を含みます。`jump`・`dist` はそれぞれ 50 万問全体の時間で、前処理・入力生成・入出力を含みません。`jump` は半分を 64 bit のランダムな移動回数、半分を閉路に到達するまでの範囲の移動回数にしています。`dist` はランダムな 2 頂点です。

| 形状 | 構築 | jump 50 万問 | dist 50 万問 |
| --- | ---: | ---: | ---: |
| 長いパスの末尾が自己ループ | 4.031 ms | 4.057 ms | 4.910 ms |
| 全体が 1 閉路 | 7.365 ms | 7.183 ms | 6.164 ms |
| 長いパスと長さ半分の閉路 | 8.545 ms | 6.956 ms | 8.051 ms |
| 全頂点がそれぞれ自己ループ | 8.370 ms | 10.601 ms | 0.747 ms |
| 二分木の根が自己ループ | 10.635 ms | 21.073 ms | 4.712 ms |
| 移動先が独立にランダム | 43.547 ms | 18.060 ms | 5.163 ms |

全試行・ソースの SHA256・コンパイラ・フラグ・乱数 seed は `results/functional_graph/measurements.json` に保存しています。質問の分布や到達不能な組の割合で時間は変わります。この表を judge の実行時間や最大制約の保証としては扱いません。

## 検証

- 1〜6 頂点の全 50,069 個の移動先配列と空のグラフを検証しました。各頂点から重複するまで直接移動して得た独立した列と、閉路、最初の到達点、距離、到達可能数を比較しました。
- 各頂点の 0〜頂点数の 2 倍までの移動、`10^18`・`2^63`・`UINT64_MAX-1`・`UINT64_MAX` 回の移動、全頂点対の到達距離・連結性・LCA を確認しました。
- 森の部分木区間とその全頂点、部分木サイズ、閉路上の辺を除いた無向森を独立した到達関係と比較しました。自己ループ・長さ 2 の閉路・複数成分も全探索に含まれます。
- 7〜80 頂点のランダムグラフ 400 個、繰り返す再構築、入力配列の不変性、コピー・move・自身の `to` 配列からの再構築を確認しました。
- 50 万頂点で、長いパス、長い閉路、パスと閉路の組、全頂点の自己ループ、星、二分木を確認しました。大きい移動回数と木の途中で止まる移動回数を検証しています。
- GCC・Clang の単独・二重 include、単体テスト、Clang ASan・UBSan を通過しました。constexpr 上限は通常 CI と同じです。
- Library Checker Jump on Tree の公開 21 ケースを生成し、GCC・Clang それぞれの出力を公式 checker で検証しました。根だけ自己ループとする Functional Graph に変換し、`lca` と `jump` を使って木のパス上の頂点を求めています。これは木部分の検証であり、一般の閉路は単体テストで検証しています。検証コードは `NDEBUG`・`-Wall -Wextra -Werror` でコンパイルしました。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2 --selection test/benchmark/functional_graph_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/functional_graph_selection.json
(ulimit -s 262144
python3 "$LC_ROOT/generate.py" "$LC_ROOT/tree/jump_on_tree/info.toml")
python3 test/benchmark/functional_graph.py --problem "$LC_ROOT/tree/jump_on_tree" --build-dir /tmp/functional-graph --output /tmp/functional-graph/measurements.json
```

`LC_ROOT` は Library Checker の問題リポジトリのパスです。公式の長い木の生成器には再帰があるため、生成時だけスタック上限を 256 MiB にしています。ライブラリの単体テスト・検証コード・測定は通常のスタック上限で実行しました。AtCoder 用テストは追加していません。
