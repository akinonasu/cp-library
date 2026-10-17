# 重心分解

`graph/centroid_decomposition.hpp` の構築を測定した。元の木の走査と成分処理を反復し、隣接リストを CSR、重心祖先の距離を頂点ごとの連続領域に保存する。深さが決まってからキャッシュを一括確保し、頂点ごとの `vector` の拡張を省く。距離不要なら `build(false)` でキャッシュを省く。

## 参考提出

2026-10-17 に [Vertex Add Range Contour Sum on Tree](https://judge.yosupo.jp/problem/vertex_add_range_contour_sum_on_tree) の AC を実行時間順に取得した。

| 提出 | ジャッジ時間 | 確認した方法 |
| --- | ---: | --- |
| [403518](https://judge.yosupo.jp/submission/403518) | 0.121 秒 | CSR、成分の反復処理、枝の BFS 順の連続配列、枝の二分、32 頂点以下の小成分処理、パス・星型の専用処理、AVX2 の集計 |
| [403470](https://judge.yosupo.jp/submission/403470) | 0.122 秒 | DFS による頂点の並べ替え、成分の反復処理、枝をサイズ順に併合、ビットマスクによる小成分処理 |
| [405059](https://judge.yosupo.jp/submission/405059) | 0.123 秒 | 403518 と同系統の連続配置・小成分処理・形状別処理・AVX2 の集計 |

これらは距離範囲クエリを解く実装で、汎用の重心分解木を返す API とは目的が異なる。CSR・反復走査・連続配置を参考にし、専用の二分バケット、形状別クエリ、AVX2 集計は取り込んでいない。上のジャッジ時間は取得時点の値で、下の構築ベンチマークと比較できない。取得結果とソースのハッシュは `results/centroid_decomposition_reference.json` に保存した。ソースの再取得は次で行う。

```sh
python3 test/benchmark/centroid_decomposition_reference.py --cache .verify-helper/cache/centroid-references --output test/benchmark/results/centroid_decomposition_reference.json
```

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。頂点数 200000、乱数 seed 731、各条件 7 回、比較側・ライブラリ側の順番を毎回シャッフルした。入力辺の準備と結果のハッシュ計算は時間に含めず、辺配列から隣接リスト・重心分解木・任意の距離キャッシュを作る `build` 全体を測定した。各実行は別プロセスで、他のテスト・測定と同時実行していない。

比較側は `vector<vector<int>>` の隣接リスト、重心分解だけ深さ対数の再帰、元の木の反復 BFS、`vector<vector<Ancestor>>` の距離キャッシュを用いる。重心選択の順序・タイブレークは同じにそろえ、全頂点の親・深さ・成分サイズ・部分木区間と全キャッシュのハッシュ一致を確認した。

generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native はさらに `-march=native -ftrivial-auto-var-init=zero` を指定した。測定値は構築時間の中央値（ミリ秒）。AtCoder 実機での速度や Library Checker fastest との優劣は未測定。

### generic

| 形状 | 距離キャッシュ | 比較側 ms | 本実装 ms | 比較側 / 本実装 |
| --- | --- | ---: | ---: | ---: |
| path | なし | 29.905 | 22.461 | 1.33 |
| path | あり | 87.027 | 50.143 | 1.74 |
| star | なし | 10.212 | 5.911 | 1.73 |
| star | あり | 18.047 | 9.191 | 1.96 |
| balanced | なし | 21.139 | 11.388 | 1.86 |
| balanced | あり | 69.877 | 38.733 | 1.80 |
| broom | なし | 19.992 | 14.017 | 1.43 |
| broom | あり | 52.865 | 29.772 | 1.78 |
| random | なし | 43.413 | 26.521 | 1.64 |
| random | あり | 142.803 | 67.560 | 2.11 |
| random_labels | なし | 44.770 | 26.206 | 1.71 |
| random_labels | あり | 140.209 | 67.255 | 2.08 |

### native

| 形状 | 距離キャッシュ | 比較側 ms | 本実装 ms | 比較側 / 本実装 |
| --- | --- | ---: | ---: | ---: |
| path | なし | 30.804 | 22.263 | 1.38 |
| path | あり | 88.454 | 51.213 | 1.73 |
| star | なし | 9.830 | 5.895 | 1.67 |
| star | あり | 18.370 | 9.436 | 1.95 |
| balanced | なし | 21.308 | 12.005 | 1.77 |
| balanced | あり | 70.615 | 39.147 | 1.80 |
| broom | なし | 19.717 | 14.103 | 1.40 |
| broom | あり | 52.851 | 29.967 | 1.76 |
| random | なし | 43.696 | 26.101 | 1.67 |
| random | あり | 143.603 | 67.657 | 2.12 |
| random_labels | なし | 45.181 | 25.431 | 1.78 |
| random_labels | あり | 143.384 | 66.080 | 2.17 |

`random` は各頂点の親をそれ以前の頂点から一様に選ぶ木、`random_labels` はそれに頂点番号のシャッフルを加えた木。`broom` は半分をパス、残りをその末端に接続した木。生の測定値・コンパイラ・CPU・フラグ・入力条件とソースのハッシュは `results/centroid_decomposition_{generic,native}/` に保存した。

```sh
python3 test/benchmark/centroid_decomposition.py --output test/benchmark/results/centroid_decomposition_generic
python3 test/benchmark/centroid_decomposition.py --native --output test/benchmark/results/centroid_decomposition_native
```

## 検証

GCC 15 / Clang 21 の単体・単独 include・他ヘッダーとの共存検証、Clang の ASan / UBSan、GCC 13、ARM64 / QEMU の単体テストを実施。独立した全点対距離、重心の均衡条件、部分木の連結性、重心祖先と枝、最近傍クエリ、負の辺重み、64 bit 重み、浮動小数点重み、空の木・一点・再構築、30 万頂点のパス・星型を検証した。

公開 API とテスト側の Fenwick 木を組み合わせた `test/centroid_decomposition_contour.test.cpp` は、Library Checker の公式 36 ケースを GCC・Clang の両方で通過。これは汎用 API の正しさを確認するドライバで、上位提出の専用クエリ構造との性能比較には用いていない。全 API 表と結合ヘッダーも検証した。ログと検証対象のハッシュは `results/centroid_decomposition_*` に保存した。
