# FPS 基本演算の調整

対象は `fps/fps.hpp` と `components/fps_operations.hpp` です。公開 API と mod 998244353 を維持します。比較元は `fd41ae5df33960b93288ae98f9edac4e9fdb166f` です。

## 参考提出

2026-10-18 JST に取得した AC 上位のうち、以下を確認しました。一覧とソース SHA-256 は `results/fps_tuning_reference.json` にあります。

| 操作・提出 | 取得時の実行時間 | 確認した手法・今回の扱い |
| --- | ---: | --- |
| [inv #409370](https://judge.yosupo.jp/submission/409370) | 11 ms | 逆元の Newton 法、変換の再利用、必要な半分だけの逆変換。既存の融合した逆元更新を維持 |
| [log #409452](https://judge.yosupo.jp/submission/409452) | 13 ms | 最大 4 区間で `f'/f`、隣接区間をまとめた窓の NTT と積和。窓の共有を採用、4 分割への単純な変更は不採用 |
| [exp #409662](https://judge.yosupo.jp/submission/409662) | 16 ms | 逆元と変換を共有する Newton 法、最終段を半分長の変換で処理。既存 Newton 経路との比較は改善せず、区間法を維持 |
| [sqrt #409728](https://judge.yosupo.jp/submission/409728) | 12 ms | 平方根と逆元を同時に更新し、最終段の変換を縮小。既存区間法の幅選択を調整 |
| [pow #409718](https://judge.yosupo.jp/submission/409718) | 23 ms | `f'/f` から指数関数を計算し、積分と微分を省略。この相殺と定数項による事前正規化の省略を採用 |
| [division #409561](https://judge.yosupo.jp/submission/409561) | 12 ms | 商を区間ごとに計算し、次数に応じて余りの計算方法を選択。巡回畳み込みと既知の高次係数による補正を採用 |

これらの提出には、AVX2 に特化した入出力、固定幅の出力、起動・終了処理の省略も含まれます。上記の Judge 上の数値と、以下のローカル kernel の数値は別の測定です。

## 実装

- `log`・級数除算・`exp` で、隣接 2 区間の変換を先に合成して保存します。従来の各積ごとの和差・追加ロードを省き、最大 7 項の積和をまとめて還元します。整数版のフォールバックも備えます。
- 係数数が切り上げた 2 冪の 3/4 未満なら区間幅を半分にします。2 冪の直後で変換が大きくなるのを抑えます。積和の項数 3・4・6・7 と、区間数 4・8・16・32・64・128 を比較しました。
- `pow` は内部で `x f'/f` を使い、`log` の積分・入力の正規化・`exp` の微分を省きます。通常の `exp` と実装を共用します。
- 商を `q`、除数を `b`、元の多項式を `a` とすると、`b*q` の高次係数は `a` と等しくなります。通常の畳み込みより変換長を縮められる場合にだけ巡回畳み込みを選び、折り返した係数を `a[i+L]` で補正します。
- 小さい `deg` を指定した級数除算では、分子の不要な高次係数を先に切ります。

7 項積和では、一方の入力を mod 未満、もう一方を 2 mod 未満に保ちます。Montgomery 還元の加算を含めても 64 bit に収まり、7 項の場合の追加還元も行います。単体テストで上限付近の生値とスカラー経路を確認します。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。`-std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero`。入力生成とチェックサム計算を除き、関数の実行を測ります。各試行は別プロセスで、NTT の初期化も測定に含みます。係数生成は `mt19937(731)`、`pow` の指数は 123456789 です。多項式除算の除数の係数数は `n/2` です。

1 回の予備実行の後、候補の実行順を seed 403023 でシャッフルし、原則 7 回または 9 回の中央値を取ります。一次選別のみ 3 回です。コンパイルごと・操作とサイズの測定群ごとに `/tmp/cp-fps-tuning.lock` を取得し、他担当のコンパイルや測定と重ならないようにします。候補は同じ NTT・modint のコピーでコンパイルし、全試行のチェックサムを比較します。

NTT 自体の調整も並行して行ったため、異なる JSON 間の数値は直接比較しません。各 JSON に使用したヘッダーの SHA-256、コンパイル条件、生の試行値を保存し、窓を共有する版以降の比較には候補ごとの FPS ヘッダーの SHA-256 も保存します。最終的な全変更と上位提出の比較は全体ベンチマークを参照してください。AtCoder の実機での速度や最速性は確認していません。

## 採用案の比較

窓を共有する変更単独の比較です。区間幅選択・`pow` の相殺を入れる前で、`results/fps_tuning_basic_windows.json` にあります。

| 操作・係数数 | 変更前 | 窓を共有 |
| --- | ---: | ---: |
| log / 524288 | 16.605 ms | 15.850 ms |
| exp / 524288 | 19.442 ms | 19.025 ms |
| 級数除算 / 524288 | 14.547 ms | 13.417 ms |

2 冪の直後の幅選択です。`results/fps_tuning_basic_window_groups.json` の変更前と `windows_nearest` を比較します。

| 操作・係数数 | 変更前 | 窓を共有・幅を選択 |
| --- | ---: | ---: |
| log / 131073 | 3.840 ms | 3.706 ms |
| exp / 131073 | 4.341 ms | 4.023 ms |
| sqrt / 131073 | 2.637 ms | 2.465 ms |
| 級数除算 / 131073 | 3.255 ms | 2.947 ms |

幅選択の効果はサイズによります。同じファイルの log / 524288 は変更前 15.806 ms、`windows_nearest` は 16.387 ms で改善していません。小さい差をすべて有意な改善とは扱いません。

`pow` の積分と微分を省く変更は `results/fps_tuning_basic_power.json` にあります。窓を共有する版同士で、相殺前 33.016 ms、相殺後 32.647 ms（524288 係数）でした。

巡回畳み込みによる余り計算は `results/fps_tuning_basic_remainder.json` にあります。

| 係数数・除数の係数数 | 変更前 | 窓を共有 | 窓を共有・巡回畳み込み |
| --- | ---: | ---: | ---: |
| 500000 / 250000 | 13.432 ms | 13.096 ms | 10.571 ms |
| 524288 / 262144 | 14.301 ms | 13.941 ms | 13.984 ms |

後者では巡回畳み込みを選択せず、従来の余り計算を使います。

## 不採用案

| 案・操作・係数数 | 変更前 | 候補 | 記録 |
| --- | ---: | ---: | --- |
| 逆元を級数除算へ統一 / inv / 524288 | 9.750 ms | 16.066 ms | `fps_tuning_basic_screen.json` |
| 平方を通常の融合畳み込みへ / square / 524288 | 9.014 ms | 10.144 ms | 同上 |
| 64 分割 / exp / 524288 | 19.155 ms | 21.280 ms | 同上 |
| 128 分割 / exp / 524288 | 19.155 ms | 25.156 ms | 同上 |
| 4 分割 / exp / 524288 | 18.776 ms | 23.990 ms | `fps_tuning_basic_newton.json` |
| 既存 Newton 経路 / exp / 524288 | 18.776 ms | 28.471 ms | 同上 |
| 既存 Newton 経路 / sqrt / 524288 | 11.140 ms | 22.115 ms | 同上 |

従来の和差付き積和だけを 6 項にする案は、一次選別では改善しましたが、追試では log / 524288 が 16.447 ms → 16.570 ms となり、安定した改善を確認できませんでした。窓を共有した後の積和の 3・4・6・7 項の比較も `fps_tuning_basic_window_groups.json` に残します。

## 検証と再現

`test/unit/fps_tuning_basic.cpp` で、小規模な愚直計算、log と exp の往復、逆元、平方根、多項式除算、短い入力の延長、2 冪前後、先頭の零、最大 signed 64 bit 指数を確認します。7 項積和の端値、スカラー経路と AVX2 経路も確認します。`NDEBUG` でも検証を省略しません。

GCC 15.2、GCC 13.4、Clang 21.1.8、Clang ASan/UBSan が成功しました。詳細は `results/fps_tuning_basic_check.log` と `results/fps_tuning_basic_validation.json` に保存します。依存先と公式ケースは全体の検証で確認します。

```sh
python3 test/benchmark/fps_tuning_basic.py --variants before windows pow_unfused --operations pow exp --sizes 32768 131073 524288 --trials 9 --output results-power.json
python3 test/benchmark/fps_tuning_basic.py --variants before windows divmod_full --operations divmod --sizes 131071 500000 524287 524288 --trials 9 --output results-remainder.json
```

`windows` は実行時の作業版です。最終版には窓・区間幅選択・`pow`・余りの変更を含みます。`before` は比較元 commit の FPS を、実行時の共通 NTT と組み合わせます。全変更を含む commit 単位の比較には全体ベンチマークを使います。
