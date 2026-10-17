# 二辺連結成分分解

## 参考

2026-10-17 に Library Checker Two-Edge-Connected Components の実行時間順の上位 AC を取得しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [305681](https://judge.yosupo.jp/submission/305681) | 26 ms | 固定長の CSR、反復 DFS、橋以外の木辺を DSU で併合、連結リストで成分を出力 |
| [390338](https://judge.yosupo.jp/submission/390338) | 27 ms | 同系統の CSR・反復 DFS・DSU |
| [215566](https://judge.yosupo.jp/submission/215566) | 27 ms | 固定長バッファの CSR、再帰 DFS と頂点スタックによる成分の確定 |

順位一覧・取得時刻・原文の SHA256 は `results/two_edge_connected_components_reference.json`、取得スクリプトは `two_edge_connected_components_reference.py` にあります。測定スクリプト内のソース SHA256 は LF に正規化しています。judge の記録は下の手元の時間と比較できません。

## 実装

`TwoEdgeConnectedComponents` は成分 ID、成分の頂点一覧、橋の辺 ID、橋の森を扱います。多重辺・自己ループ・孤立点・非連結グラフ・空グラフに対応します。

- 辺 ID 付きの無向 CSR を構築し、親の辺 ID だけを除外して多重辺を処理。
- `(頂点,親の辺ID,次の出辺)` のフレームで DFS を反復化。深いパスでも再帰スタックを使わない。
- `low[v] == ord[v]` で成分を確定し、頂点スタックから `v` までを取り出す。親辺があれば橋として記録。
- 頂点一覧の先頭を作業用スタック、末尾を確定済みの成分の保存先として使い、頂点数分の配列を共用。
- 橋と成分境界の領域をあらかじめ確保し、橋の多いグラフでの再確保を削減。
- `group(c)` は平坦な領域への span。成分ごとの個別確保を行わず、`groups()` でのコピーは必要な場合だけ実行。
- `forest()` は要求時に構築し、各辺に元の辺 ID を保持。
- CSR の構築は `components/undirected_csr.hpp` に分離し、既存の Low-Link でも共用。Low-Link の DFS と公開 API は維持。

構築は O(n+m) 時間・空間です。成分 ID の取得、同一成分判定、橋判定、1 成分の span の取得は O(1) です。成分 ID と頂点順はソートしません。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の実測ではありません。参考提出は原文のままビルドし、390338 は調査のみです。

全実装に同じ通常ファイルを標準入力として渡し、Library Checker と同じ形式で全成分を出力させました。頂点の欠落・重複を調べた後、成分を最小頂点で正規化して分割の一致を確認しています。プロセス起動・入力・構築・出力・終了を含む壁時計時間です。ウォームアップ 1 回の後、順序を混ぜた 7 回の中央値を採用しました。計測中に他のコンパイル・テストは実行していません。

再帰版 215566 の深いパスに合わせ、全プログラムのスタック上限を 64 MiB にしています。今回の実装はその拡張を必要としません。最大 RSS は `/usr/bin/time` で、時間の計測とは別に 1 回ずつ取得しました。

`low_link_dsu` は同じ Fast I/O と CSR を使い、既存の Low-Link で橋を求めてから、橋以外の辺を DSU で併合する比較版です。実装は `two_edge_connected_components_baseline.hpp` にあります。

| 入力 | 頂点数 | 辺数 | 内容 |
| --- | ---: | ---: | --- |
| path | 200,000 | 199,999 | 一本のパス |
| star | 200,000 | 199,999 | 頂点 0 を中心とした星 |
| cycle | 200,000 | 200,000 | 一つの閉路 |
| random | 200,000 | 200,000 | 両端を独立に選んだ無向辺 |
| triangles_chain | 150,000 | 199,999 | 50,000 個の三角形を橋でつないだパス |
| parallel | 200,000 | 200,000 | 100,000 組の頂点にそれぞれ 2 本の平行辺 |
| loops | 200,000 | 200,000 | 各頂点に 1 本の自己ループ |

## 結果

単位は ms です。

| 入力 | 今回 generic | Low-Link + DSU | 305681 | 215566 |
| --- | ---: | ---: | ---: | ---: |
| path | 18.078 | 20.360 | 20.271 | 21.941 |
| star | 14.347 | 15.547 | 16.619 | 13.547 |
| cycle | 16.961 | 19.354 | 19.828 | 20.774 |
| random | 27.396 | 30.016 | 27.974 | 27.057 |
| triangles_chain | 15.870 | 17.319 | 17.577 | 18.986 |
| parallel | 17.819 | 17.320 | 18.253 | 16.941 |
| loops | 16.908 | 17.631 | 18.458 | 15.093 |

| 入力 | 今回 native | Low-Link + DSU | 305681 | 215566 |
| --- | ---: | ---: | ---: | ---: |
| path | 19.981 | 20.546 | 19.674 | 22.423 |
| star | 14.510 | 15.496 | 15.542 | 13.245 |
| cycle | 17.236 | 19.269 | 18.647 | 21.112 |
| random | 27.306 | 30.846 | 28.264 | 29.096 |
| triangles_chain | 16.432 | 17.706 | 17.420 | 18.657 |
| parallel | 16.234 | 17.975 | 18.159 | 15.067 |
| loops | 19.468 | 20.833 | 20.703 | 18.264 |

上位 AC に近い値になりました。今回の方式は再帰・DSU を使いませんが、全ての入力で最速という結果ではありません。特に小さい成分が多数ある入力では 215566 が速い場合があります。

generic の最大 RSS は path で今回 18,280 KiB、Low-Link + DSU 17,748 KiB、305681 20,812 KiB、215566 35,260 KiB です。random は順に 15,652 / 14,624 / 17,620 / 20,676 KiB です。

生データ・全オプション・入力と分割の SHA256・ソースの SHA256・各入力の最大 RSS は `results/two_edge_connected_components_generic/results.json` と `results/two_edge_connected_components_native/results.json` にあります。`results/two_edge_connected_components_initial/results.json` は `bridges.reserve` と `boundaries_.reserve` を追加する前の 3 回の予備測定です。

## 検証

- 頂点数 0〜5 の単純無向グラフを全列挙し、各辺を除く BFS による橋の判定と、全ての橋を除いた連結成分に照合。
- 多重辺・自己ループを含む 2,000 個のランダムグラフを同じ独立実装に照合。
- 全頂点対の同一成分判定、成分一覧、橋の重複・欠落、橋の森の両端と元の辺 ID を確認。
- 30 万頂点のパス、辺の追加による閉路化、平行辺を追加した後の再構築を確認。
- CSR の辺順・辺 ID・両方向の格納・自己ループを 1,530 個のグラフで照合。
- CSR を共用する既存 Low-Link の単体テストと橋・関節点のジャッジ、単独利用テストを実行。
- Library Checker の成分分解 21 ケース、AOJ の橋・関節点各 40 ケースを GCC・Clang で検証。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、リリース時の `-Werror` ビルド。
- ヘッダー単独 include、全 API 表と公開用 Bundle を検証。

## 再実行

```sh
python3 test/benchmark/two_edge_connected_components_reference.py --ids 305681 390338 215566 \
  --cache .verify-helper/cache/two-edge-reference \
  --output test/benchmark/results/two_edge_connected_components_reference.json
python3 test/benchmark/two_edge_connected_components.py \
  --references .verify-helper/cache/two-edge-reference \
  --output test/benchmark/results/two_edge_connected_components_generic
python3 test/benchmark/two_edge_connected_components.py --native \
  --references .verify-helper/cache/two-edge-reference \
  --output test/benchmark/results/two_edge_connected_components_native
python3 test/verify.py --unit-only --cxx g++ clang++ \
  --selection test/benchmark/results/two_edge_connected_components_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/two_edge_connected_components_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/two_edge_connected_components.test.cpp \
  test/low_link_bridge.test.cpp test/low_link_articulation.test.cpp
```
