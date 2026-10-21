# 有向・無向 Euler 路

2026-10-21。`graph/eulerian_trail.hpp` に、有向・無向グラフの Euler 路を求める共通実装を追加しました。自己辺・多重辺・始点指定に対応し、頂点列と入力の辺 ID 列を返します。

## 参考提出と手法

Library Checker の [有向版](https://judge.yosupo.jp/problem/eulerian_trail_directed)・[無向版](https://judge.yosupo.jp/problem/eulerian_trail_undirected)について、それぞれ公式 API の AC 実行時間順で先頭 3 件を取得し、ソースを確認しました。

| 問題 | 提出 | 取得時の時間 | 手法 |
| --- | --- | ---: | --- |
| 有向 | [286963](https://judge.yosupo.jp/submission/286963) | 21 ms | forward-star、次数条件、反復版 Hierholzer、頂点と辺のスタック |
| 有向 | [219214](https://judge.yosupo.jp/submission/219214) | 28 ms | CSR、次数条件、再帰 DFS、辺列の後方確定 |
| 有向 | [185415](https://judge.yosupo.jp/submission/185415) | 28 ms | CSR、反復探索、辺のスタックと出力配列の共有 |
| 無向 | [404632](https://judge.yosupo.jp/submission/404632) | 28 ms | forward-star、辺 ID の使用済み判定、頂点と辺のスタック |
| 無向 | [286968](https://judge.yosupo.jp/submission/286968) | 29 ms | 同系統の反復版 Hierholzer |
| 無向 | [219217](https://judge.yosupo.jp/submission/219217) | 32 ms | CSR、次数条件、使用済み判定付きの再帰 DFS |

取得時刻・一覧・ソースの SHA-256 は `results/eulerian_trail/reference.json` に保存しています。

本実装も次数条件を先に調べ、反復版 Hierholzer を使います。有向・無向はテンプレートで分け、有向版では使用済み配列を確保しません。CSR は行き先と元の辺 ID だけを格納します。辺 ID を後から頂点列に対応付ける辞書は不要です。

[提出185415](https://judge.yosupo.jp/submission/185415)で確認した、探索と確定済み出力で配列を共有する方法と同様に、本実装では辺列と頂点列の両方をスタックにも使います。配列の前方に探索中の列を持ち、行き止まりで取り除いた要素を後方に確定します。要素の追加回数は頂点列で高々 `m+1`、辺列で高々 `m` なので、探索中の領域と確定済みの領域は重なりません。最後の反転も不要です。

全辺を使えたかを確認することで、次数条件だけでは判別できない非連結な場合を除きます。求解は入力を変更せず、同じグラフで繰り返し実行できます。

上の時間は公開 judge 上で入出力を含めた値です。今回、速度の比較測定や公開 judge への提出は行っていません。

## 検証

- 頂点数 1〜3、辺数 0〜4 の辺列を全列挙しました。有向 7,727 列、無向 1,681 列で、自己辺・多重辺を含みます。無向版の登録方向は正規化しています。
- 各グラフについて、始点の自動選択と全頂点の始点指定を確認しました。辺部分集合と終点を状態とする独立した DP で存在判定を照合し、返された辺 ID の全使用・一意性・向き・頂点列との対応も検査しました。
- 有向・無向でそれぞれ 1,000 個のランダムなグラフ。辺の追加前後、繰り返し求解、コピー・`clear` の独立性と入力の非変更を確認しました。辺数は追加後でも高々 10 本です。
- 0 頂点、辺なし、孤立点、非連結、始点不適合、20 万頂点のパスと閉路、100 万本の自己辺。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入。
- 有向・無向それぞれ公式 27 ケースを、GCC・Clang の検証コードで実行しました。解は一意ではないため、全て公式チェッカーで検査し、成功しました。
- `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror` で両方の検証コードとドキュメント例をコンパイルしました。

実行環境は Linux x86_64 / AMD EPYC 9V74 です。検証結果・公式入力のハッシュ・利用した生成器のコミット・実装のハッシュは `results/eulerian_trail/validation.json` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/eulerian_trail_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/eulerian_trail_selection.json
oj-verify run test/eulerian_trail_directed.test.cpp test/eulerian_trail_undirected.test.cpp
```
