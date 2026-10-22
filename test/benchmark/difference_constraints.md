# 牛ゲー（差分制約系）

2026-10-22。差分制約を有向辺へ変換し、全成分の実現可能性と解の復元を Bellman–Ford、差の上下限を再重み付け後の Dijkstra で求めます。

## 整数幅の比較

Linux x86_64、AMD EPYC 9V74、GCC 14.2、`-std=c++23 -O2 -DNDEBUG`。CPU 0 に固定し、各経路をウォームアップ 1 回、その後 7 回測定した中央値です。実行順は試行ごとに入れ替えています。

同じ制約を持つ `DifferenceConstraints<>` と `DifferenceConstraints<__int128_t>` を比較しました。前者は制約の保持・内部計算・戻り値ともに 64 bit、後者は全て 128 bit です。制約の生成・検証・入出力は計測に含めません。`solve` はグラフ構築・実現可能性判定・解の復元・再重み付けを含み、結果の破棄は含みません。`upper_bounds` は求解済みの結果から 1 始点について全変数の上限を求め、返す配列の確保も含みます。

| 入力 | 変数数 | 制約辺数 | solve・64 bit | solve・128 bit | upper_bounds・64 bit | upper_bounds・128 bit |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| 疎な強連結グラフ | 20,000 | 100,002 | 6.777 ms | 8.826 ms | 1.182 ms | 4.982 ms |
| 番号の逆順に進む負辺のパス | 200,000 | 200,001 | 12.581 ms | 17.092 ms | 2.255 ms | 8.696 ms |
| 4 頂点の閉路に分かれたグラフ | 20,000 | 20,002 | 1.171 ms | 1.545 ms | 0.031 ms | 0.044 ms |
| 疎グラフに負の自己辺を追加 | 20,000 | 100,003 | 2.429 ms | 2.984 ms | — | — |

疎グラフの全頂点への上限計算は、64 bit 経路が約 4.21 倍速くなりました。128 bit の比較ヒープに対し、64 bit では既存の radix heap を使えることも速度差に含まれます。このため、デフォルトは 64 bit とし、大きい整数が必要な場合に利用者が 128 bit を指定する方式にしました。これはこの環境・分布での比較であり、judge の実行時間を保証するものではありません。

疎グラフは全頂点の巡回辺とランダム辺で構成し、重みを非負の slack と頂点ポテンシャルの差の和として生成しました。4 頂点の閉路に分かれた入力では、問い合わせ始点から大部分の変数へ到達できません。負辺のパスは辺の走査順が逆になる番号付けです。各入力には固定値の制約を 2 辺追加しています。乱数 seed・全測定値・コンパイラ・フラグ・入力およびソースの SHA256 は `results/difference_constraints/measurements.json` に保存しています。

## 安全性と手法

通常は `DifferenceConstraints<>` により 64 bit を使い、必要な場合だけ `DifferenceConstraints<__int128_t>` と指定します。範囲に基づく自動選択や、途中での型の切り替えは行いません。制約の変換・距離の加算・解の平行移動・再重み付け・上下限の復元はオーバーフローを検査します。中間値や結果が選んだ型に収まらない場合は `std::overflow_error` を投げます。

Dijkstra の無限大の番兵以上の距離は通常の到達不能と区別する必要があります。そのため、有限な距離を持つ頂点から、計算結果が到達不能となっている頂点への辺がないことを確認します。そのような辺があれば、到達可能なのに距離を表現できなかったことを示すため、例外を投げます。この検査も測定時間に含みます。

全成分を仮想始点から到達可能にして矛盾を判定するので、問い合わせと無関係な成分の負閉路も検出します。有限な差の最大値は最短路と一致し、逆方向の最大値の符号を反転すると最小値になります。解なしのときは、元の制約だけからなる負閉路を証明として返します。探索処理には既存の CSR・SCC 分解・Goldberg–Radzik・Dijkstra を利用しています。

## 参考実装

[Nyaan's Library の Dual_of_Shortest_Path](https://github.com/NyaanNyaan/library/blob/master/shortest-path/dual-of-shortest-path.hpp) を確認しました。差分制約を辺で保持し、全頂点を 0 で初期化するか、指定始点のみを 0 にして、最大で頂点数回の全辺緩和を行います。今回は全成分の実現可能性判定と複数回の上下限問い合わせを分け、既存の Bellman–Ford と再重み付けを利用しました。

差分制約系の全機能に直接対応する Library Checker の問題はないため、非負辺の [Shortest Path](https://judge.yosupo.jp/problem/shortest_path) の AC 時間順の先頭 3 件を調査しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [407097](https://judge.yosupo.jp/submission/407097) | 38 ms | CSR、入次数による非巡回部分の除去、radix heap、単一の目的地に対する早期終了 |
| [404592 / Rohan_Kapri](https://judge.yosupo.jp/submission/404592) | 40 ms | CSR、構造による分岐、radix heap、単一の目的地への最短路 |
| [403027 / nandhagk](https://judge.yosupo.jp/submission/403027) | 40 ms | 同系統の CSR・radix heap による実装 |

これらは負辺や全成分の矛盾判定を扱わず、今回のライブラリとの直接の速度比較にはなりません。全頂点への上限取得を提供するため、目的地だけに特化した処理は追加していません。取得時の一覧と参照ソースの SHA256 は `results/difference_constraints/references.json` に保存しています。公開 judge への提出や、これらの提出との時間比較は行っていません。

## 検証

- 小さい有向グラフ 39,450 通りを全探索し、Floyd–Warshall による独立した全点対距離・負閉路判定と比較しました。64 bit 版と 128 bit 版の両方について、全始点の上限・全頂点対の最大差と最小差を検証しています。
- 不等号・等式・区間・上下限・固定値の変換を、具体的な整数の割り当てと照合しました。ランダムな制約系 1,000 個に加え、2 変数の系 500 個を整数解の直接列挙でも検証しました。
- 空の系、定数頂点だけの矛盾、多重辺、離れた成分の矛盾、非有界、有限な答え `-1`、負閉路の辺の存在・連結・総和を確認しました。
- `INT64_MIN`・`INT64_MAX` とその隣接値、厳密不等号の変換、64 bit を超える解、64 bit に収まる大きな距離を検証しました。制約の変換・負閉路探索・有限距離・解の復元・符号反転が 64 bit であふれる場合は例外になり、128 bit 指定では大きい有限な答えや矛盾を返せることを確認しました。
- 20 万変数の負辺のパス、結果のコピー・move・制約追加前の結果の独立性を確認しました。
- GCC・Clang の単独・二重 include、単体テスト、Clang ASan・UBSan を通過しました。constexpr 上限は通常 CI と同じです。
- AOJ GRL_1_B の公開 40 ケースを GCC・Clang でそれぞれ実行し、計 80 回の出力を公式出力と比較しました。この問題は指定始点から到達する負閉路だけを対象とするため、検証コードでは到達可能な制約だけを渡しています。全成分の矛盾判定は単体テストで確認しています。AtCoder のテストは追加していません。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2 --selection test/benchmark/difference_constraints_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/difference_constraints_selection.json
oj download --system --directory /tmp/difference-cases https://onlinejudge.u-aizu.ac.jp/problems/GRL_1_B
python3 test/benchmark/difference_constraints.py --cases /tmp/difference-cases --build-dir /tmp/difference-build --output /tmp/difference-measurements.json
```
