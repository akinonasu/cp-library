# トポロジカルソート

2026-10-21。`graph/topological_sort.hpp` に、隣接リストを受け取るトポロジカルソートを追加しました。閉路がある場合は `std::nullopt` を返し、テンプレート引数 `<true>` では辞書順最小の順序を求めます。

## 参考提出と手法

取得時の Library Checker の全 253 問を確認しました。一般の有向グラフのトポロジカルソートだけを扱う問題はありません。`Rooted Tree Topological Order with Minimum Inversions` は根付き木で転倒数を最小化する別の問題です。

関連問題として、成分をトポロジカル順に出力する [Strongly Connected Components](https://judge.yosupo.jp/problem/scc) の AC 実行時間順で先頭 3 件のソースを取得・確認しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [404707](https://judge.yosupo.jp/submission/404707) | 22 ms | 反復版 Tarjan。成分を逆トポロジカル順に確定し、後方から出力を構築 |
| [403026](https://judge.yosupo.jp/submission/403026) | 29 ms | Kahn による入次数 0 の除去と、残ったグラフの SCC 分解。確定した単独成分をキューに兼用 |
| [404616](https://judge.yosupo.jp/submission/404616) | 35 ms | 同系統の Kahn による除去と残余の SCC 分解 |

取得時刻・一覧・ソースの SHA-256 と問題一覧の検索結果は `results/topological_sort/reference.json` に保存しています。

一般のトポロジカルソートは DFS の帰りがけ順の反転でも線形時間で求められますが、閉路検出用の状態と探索スタックが必要です。本実装は Kahn のアルゴリズムを使い、通常版では出力配列を FIFO キューとして兼用します。入次数は `size_t` で保持し、多重辺を頂点数とは独立して数えます。入力は変更しません。

辞書順最小版は、入次数 0 の候補を最小ヒープで管理します。初期候補をまとめて `make_heap` し、各頂点を高々 1 回ずつ追加・削除します。通常版ではヒープを構築せず、どちらの版でも再帰やグラフのコピーは不要です。

上の時間は SCC という別問題での公開 judge の値です。今回、速度の比較測定や公開 judge への提出は行っていません。

## 検証

- 0〜4 頂点の全 66,067 有向グラフを列挙しました。自己辺も含みます。頂点の全順列を辞書順に調べる独立した判定と、閉路の有無・辞書順最小性を照合しました。
- 0〜7 頂点、多重辺を含むランダムな 1,000 グラフについて、辺の順序をシャッフルする前後で同じ照合を行いました。
- 通常版が返す全頂点の一意性と、全辺について始点が終点より前にあることを検査しました。入力の非変更と繰り返し呼び出しも確認しました。
- 空グラフ、非連結、閉路を含む一部の成分、自己辺、`array`・`span`・`deque` の隣接リストを確認しました。
- 50 万頂点の逆向きパス・閉路・多数の候補が同時に生じるグラフと、100 万本の多重辺を確認しました。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入を確認しました。
- [AOJ GRL_4_A](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_4_A) の公式全 44 ケースで、通常版の閉路判定を照合しました。
- [AtCoder ABC223 D](https://atcoder.jp/contests/abc223/tasks/abc223_d) の公式サンプル 2 件で、辞書順最小版の順序と閉路判定を照合しました。
- 両方の検証コード・ドキュメント例を GCC・Clang の `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror` でコンパイル・実行しました。

実行環境は Linux x86_64 / AMD EPYC 9V74 です。検証結果と公式入力・実装のハッシュは `results/topological_sort/validation.json` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/topological_sort_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/topological_sort_selection.json
oj-verify run test/topological_sort.test.cpp
```
