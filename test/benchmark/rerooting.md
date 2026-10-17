# 全方位木 DP

`graph/rerooting.hpp` に `Rerooting<T,E>` を追加しました。`T` は頂点の DP 値、`E` は辺を通した寄与で、省略時は同じ型です。木・森、非可換な結合、方向ごとに異なる辺の変換に対応します。

## 実装

- 共通の `UndirectedCSR` を利用し、隣接辺を入力順に平坦な配列へ格納。
- 内部の辺 ID は `2 * id` と `2 * id + 1` にして、逆向きの寄与を XOR 1 で参照。DP の走査中に辺の両端を引き直さない。
- BFS 順の逆順で子側の寄与を求め、順方向で親側の寄与を求める。再帰を使わない。
- prefix 用の領域を最大次数だけ確保して使い回し、suffix は 1 個の値で保持。頂点ごとの作業配列の確保を省く。
- 単位元との不要な結合を省き、辺を通すコールバックは各方向 1 回、合計で辺数の 2 倍だけ呼ぶ。
- コールバックはテンプレートで受け、`std::function` を使わない。引数の関数オブジェクトのコピーも不要。

## 参考

2026-10-17 に Library Checker の AC 一覧を実行時間順に取得し、各問題の先頭 3 件のソースを確認しました。一覧・取得時刻・ソース SHA-256 は `results/rerooting_reference.json` にあります。

| 問題・参考提出 | 取得時の実行時間 | 手法・比較 |
| --- | ---: | --- |
| [Tree Path Composite Sum #346214](https://judge.yosupo.jp/submission/346214) | 38 ms | CSR・根付き木の順序と部分木サイズを使う 2 回の DP。加算の逆演算で子の寄与を除外。入出力込みで同じ入力を使って測定 |
| [同 #405049](https://judge.yosupo.jp/submission/405049) | 40 ms | 同系列の CSR と加減算による伝播。独自入出力を含むソースを確認 |
| [同 #346138](https://judge.yosupo.jp/submission/346138) | 40 ms | 各頂点で子を走査する下向き DP と加減算による伝播。入出力込みで同じ入力を使って測定 |
| [Tree Diameter #361517](https://judge.yosupo.jp/submission/361517)、[#361515](https://judge.yosupo.jp/submission/361515)、[#362495](https://judge.yosupo.jp/submission/362495) | 26 / 26 / 27 ms | XOR による隣接管理と葉からの処理で直径・両端点を求め、親を辿って経路を復元。全頂点の答えは不要なので、本ライブラリの直径用検証プログラムとの速度比較は行わない |

Tree Path Composite Sum の参考実装は、全体から子の寄与を引くことで prefix / suffix の計算を省いています。本実装は結合の逆演算を仮定せず、合成数 mod の積や非可換な結合も扱います。参考の実行時間は Library Checker 上の入出力込みの値で、下記のローカル測定とは別です。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、1 回のウォームアップ後に各 7 回を測定します。実行順は固定 seed でシャッフルし、中央値を取ります。各試行は別プロセスです。実行ファイル・入出力ファイルは Linux 側の `/tmp` に置き、他のテストとは同時実行しません。

- generic: `-std=c++2b -O2 -DNDEBUG`
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加

頂点数は 200,000 です。パス・星・二分木・パスの先端に星を付けた木・ランダムな木を使い、頂点番号・辺の順序・辺の向きをシャッフルします。

カーネルの測定は、各頂点から全頂点への距離和と頂点数を返す DP です。比較元は `rerooting.cpp` 内の反復型の汎用実装で、`vector<vector<...>>` に隣接辺を格納し、各頂点で prefix / suffix / 子の寄与を確保します。グラフ構築と DP を測り、入力生成・チェックサム・入出力を除きます。本実装の結果を持つオブジェクトの破棄は測定外です。比較元の隣接配列・作業配列の破棄は関数の呼び出し内に含まれます。

入出力込みの測定は Tree Path Composite Sum です。参考 #346214・#346138 は取得したソースを変更せずにコンパイルします。全実装に同じ通常ファイルを標準入力として渡し、起動・入力・グラフ構築・DP・出力・終了を測ります。入力生成と出力の照合は測定外です。参考提出の独自入出力と、本実装の `FastInput` / `FastOutput` の違いも含まれます。

全試行で全頂点の答えのチェックサムを照合します。環境、フラグ、ソース・入力のハッシュ、全試行の時間は `results/rerooting_generic/results.json`・`results/rerooting_native/results.json` に保存します。AtCoder 実機での測定ではありません。

## 結果

カーネルの時間は ms です。この比較では本実装は比較元の約 2.8～3.5 倍の速度でした。

| 形状 | generic 本実装 / 比較元 | native 本実装 / 比較元 |
| --- | ---: | ---: |
| パス | 19.434 / 62.636 | 18.438 / 54.201 |
| 星 | 13.147 / 43.698 | 13.490 / 43.575 |
| 二分木 | 18.794 / 63.032 | 19.521 / 64.012 |
| パスと星 | 16.643 / 58.612 | 17.418 / 60.035 |
| ランダム | 21.455 / 59.415 | 21.548 / 63.214 |

Tree Path Composite Sum の入出力込みの時間は次のとおりです。

| 形状 | generic 本実装 / #346214 / #346138 | native 本実装 / #346214 / #346138 |
| --- | ---: | ---: |
| パス | 39.542 / 26.749 / 30.953 | 40.294 / 27.094 / 29.892 |
| 星 | 26.997 / 20.457 / 21.777 | 26.917 / 20.490 / 22.135 |
| 二分木 | 38.450 / 22.949 / 25.237 | 39.269 / 22.247 / 25.230 |
| パスと星 | 33.847 / 23.326 / 25.279 | 33.357 / 23.105 / 25.751 |
| ランダム | 38.898 / 24.166 / 27.518 | 39.617 / 23.491 / 27.095 |

全形状で、加減算に特化した参考 #346214 が最速でした。本実装は可換性・逆元を仮定しない汎用版です。この結果から全方位木 DP 全般での最速性は主張しません。

## 検証

- 小さい木・森の全頂点、辺の両方向の寄与を再帰による愚直計算と比較。
- 辺順序・向き・走査の根を変更し、非可換な文字列の結合、方向依存の重み、負の重みを確認。
- 異なる頂点・辺の型、デフォルト構築できない同型の DP 値、コピーできないコールバック、再構築・辺の追加・コピーを確認。
- 連結な頂点集合の数を部分集合全列挙と比較。法 1・2・6・12・1,000,000,007 を使い、逆元に依存しないことを確認。
- 300,000 頂点のパス・星で、全頂点の距離和を閉じた式と比較。
- GCC 13.4・15.2、Clang、`NDEBUG` + `-Werror`、ASan / UBSan、単独 include と公開用 Bundle を検証。
- Library Checker の Tree Path Composite Sum・Tree Diameter を各 16 ケース、GCC・Clang の両方で AC。
- CI の依存先選択、API 表、Apple Clang の互換性、公開ページを確認。

## 再現

```sh
python3 test/benchmark/rerooting_reference.py --cache .verify-helper/cache/rerooting-reference --output test/benchmark/results/rerooting_reference.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/rerooting_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/rerooting_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/rerooting.test.cpp test/rerooting_diameter.test.cpp
python3 test/benchmark/rerooting.py --reference-cache .verify-helper/cache/rerooting-reference --output test/benchmark/results/rerooting_generic
python3 test/benchmark/rerooting.py --native --reference-cache .verify-helper/cache/rerooting-reference --output test/benchmark/results/rerooting_native
```
