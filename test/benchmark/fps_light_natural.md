# 軽量 FPS の可読性と依存関係

2026-10-22。比較元は `cbcecdee84583cdac750006b438c5491e80cac85` です。

## 通常の C++ に戻す

`components/fps_light_backend.hpp` を、通常の整形と ASCII の識別子を使う直接編集可能な実装にしました。特殊文字の辞書マクロ、トークン列の置換、利用時の追加圧縮はありません。軽量版の旧 `.inc` 原本は削除し、`tools/update_fps_headers.py` の対象からも外しました。高速版 FPS と入出力の配布コードは変更していません。

軽量 FPS・NTT・積木・従来の追加演算の公開ヘッダーと API は維持しています。いずれを include しても共通実装は一度だけ読み込みます。汎用 modint の積では既存の係数変換関数を再利用し、同じオブジェクトの二乗では変換結果も共有します。

係数変換の小さな共通化以外は、旧原本と C++ のトークンが一致します。28 個の assert、SIMD の実行時判定、スカラー経路、`target` 属性、GCC の最適化範囲を保持しています。

## 派生ライブラリとサイズ

軽量版サイトでは FPS 自体に加え、FPS を使う公開ライブラリすべてに軽量実装を対応付けました。ページの見出しだけでなく、実際に配布するヘッダーとその依存先を変更しています。

| 分類 | 軽量実装 |
| --- | --- |
| FPS | 基本演算、Taylor shift、Bostan–Mori、線形漸化式、多点評価、補間、合成、合成逆元、射影、多項式 GCD・拡張 GCD・逆元 |
| 畳み込み | 通常、巡回、乗法、順序制約、max-min / min-max、オンライン、多変数 |
| 二項係数 | `comb`・階乗表、prefix sum、prefix sum の差分更新 |

合成・射影・合成逆元は軽量 FPS の畳み込みを使う二変数 Bostan–Mori とその転置、GCD は軽量 FPS の多項式演算を使う再帰 Half-GCD です。単なる二次時間の実装へ置き換えず、それぞれ $$O(n\log^2 n)$$ 時間で計算します。畳み込みの任意係数型への対応、二項係数の型別キャッシュ・負の添字・法変更時の再構築も保持します。

通常版と同時に include できるよう、FPS 関数は `light_fps_algorithms`、畳み込みは `light_convolution` と `LightOnlineConvolution`、二項係数は `light_combinatorics` を使います。`_light.hpp` の実装を直接選べるため、サイトや提出用の専用ツールは不要です。

サイズは通常の `oj-bundle` の出力に軽量サイトの参照 URL を加えた UTF-8 バイト数で、プラットフォームごとの条件分岐も含みます。追加圧縮はしていません。

| 条件 | バイト数 | 64 KiB までの余裕 |
| --- | ---: | ---: |
| 軽量 FPS 全ヘッダー＋全 API 使用例＋基本 Fast I/O | 54,173 | 11,363 |
| 軽量の二項係数 prefix sum＋基本 Fast I/O＋main | 56,659 | 8,877 |
| 単独軽量ヘッダー24件の最大＋基本 Fast I/O＋main | 59,314 | 6,222 |

全24ヘッダーの実際の依存閉包も検査し、通常版 `fps.hpp`・`fps_backend.hpp` が含まれず、通常の ASCII の C++ だけであることを確認しました。通常版専用の SIMD カーネルは代替の API がないため、軽量実装のあるふりをせず、一覧上で通常版へ案内します。

通常版全 API の 256 KiB 検査と、軽量版全 API の 64 KiB 検査は分けています。従来の通常版10ヘッダーと軽量版基本2ヘッダーを混在させる検査も保持しており、基本 Fast I/O と通常版全 API を含めて258,327 Bです。旧 fixture の `test/fps_io.hpp` は実際に使われていなかったので除去し、使用している基本 Fast I/O は維持しました。新しい軽量派生も含めて通常版と軽量版の全16ヘッダーを同時に読む例は 268,816 B となり、256 KiB を超えます。両実装をすべて同梱する場合まで上限内だとは主張しません。

軽量版だけを選べば派生を含めた全 FPS 演算と基本 Fast I/O が64 KiB以内に収まり、解答本体にも領域を残せます。ほかのライブラリや一括 I/O を追加する場合は、最終ソース全体を確認してください。

各実装の資料は [合成・射影](fps_light_composition.md)、[多項式 GCD](polynomial_gcd_light.md)、[二項係数](combinatorics_light.md)、[畳み込み](convolution_light.md) にあります。

[全26サイズ条件と回帰検査の原票](results/fps_light_natural/bundle-regression.json)に、入力例・出力・依存ソースのハッシュと、10件・66 subtest の成功を記録しています。

## 実行時間

Linux x86_64 / AMD EPYC 9V74、GCC 14.2、CPU 2 固定、`-std=c++23 -O2 -march=haswell -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -Wall -Wextra`。旧軽量版・今回の軽量版・通常版を同じ入力で各 7 回実行し、中央値を比較しました。毎回別プロセスで起動し、根表の初回構築と演算中の確保を含め、入力生成・入出力・結果検証は除いています。ヘッダー内部の最適化指定も含みます。

基本演算は 1,024・32,768・131,073・524,288 係数の 9 演算、追加演算は 1,024・32,768 係数または点の 4 演算です。44 条件・924 試行で、結果の長さと checksum が全実装で一致しました。測定順は固定 seed で並べ替え、FPS と他の最適化測定の排他ロックを取得しています。

旧軽量版に対する最大の増加は積・32,768 係数の **3.56%**、最大の減少は補間・32,768 点の **2.82%** でした。表記を自然に戻すことで、演算量が増える変更はありません。

### 基本演算：524,288 係数

| 演算 | 旧軽量版 (ms) | 今回 (ms) | 今回 / 旧版 |
| --- | ---: | ---: | ---: |
| 積 | 14.947 | 14.794 | 0.990 |
| 二乗 | 9.762 | 10.079 | 1.033 |
| 逆数 | 19.199 | 19.433 | 1.012 |
| 対数 | 33.362 | 33.788 | 1.013 |
| 指数 | 38.800 | 38.717 | 0.998 |
| 平方根 | 23.476 | 23.678 | 1.009 |
| 冪 | 67.699 | 67.399 | 0.996 |
| 級数除算 | 24.363 | 24.262 | 0.996 |
| 多項式の商・余り | 21.671 | 21.504 | 0.992 |

積・二乗の出力は 1,048,575 係数、多項式除算の除数は 262,144 係数、冪の指数は 123456789 です。

### 追加演算：32,768 係数または点

| 演算 | 旧軽量版 (ms) | 今回 (ms) | 今回 / 旧版 |
| --- | ---: | ---: | ---: |
| Taylor shift | 1.191 | 1.204 | 1.011 |
| 多点評価 | 32.259 | 32.912 | 1.020 |
| 補間 | 43.920 | 42.681 | 0.972 |
| 線形漸化式の第 k 項 | 99.917 | 99.821 | 0.999 |

Taylor shift は 17、評価・補間の点は `0..n-1`、線形漸化式の項番号は 10^18 です。通常版との比較表は [利用説明](../../docs/fps_light.md) にあります。judge 上での実行時間や順位を表すものではありません。

- [基本演算の生データ](results/fps_light_natural/core.json) / [CSV](results/fps_light_natural/core.csv)
- [追加演算の生データ](results/fps_light_natural/algorithms.json) / [CSV](results/fps_light_natural/algorithms.csv)
- [ハッシュ・中央値の再計算結果](results/fps_light_natural/performance_audit.json)

計測中のソースハッシュが変わっていないことも確認しています。アルゴリズムと過去の fastest / AC 実装の調査は [従来の高速化資料](fps_light_speed.md) を参照してください。

## 正しさ

- GCC・Clang の単独 include 10 件、unit 6 件、通常版との併用・include 順 4 件。
- Clang ASan/UBSan 3 件。
- `atcoder::modint998244353` の汎用係数型 2 件。同じオブジェクトの二乗も検査。
- Library Checker の畳み込み・逆数・対数・指数・冪・平方根・多項式除算・Taylor shift・線形漸化式・多点評価・補間。11 問・314 ケースを GCC と Clang で実行し、628 実行すべて成功。
- 最大変換長 $$2^{23}$$ の全係数 round trip も [別途確認](results/fps_light_natural/max_ntt.json)。
- 新規19ヘッダーの [単独 include 38件](results/fps_light_natural/standalone-extensions.json)、全カテゴリの通常版・軽量版の [併用と include 順4件](results/fps_light_natural/coexist-extensions.json) も成功。
- 新規派生演算は各資料の公式テストと独立した愚直計算で別途検証。

[検証結果](results/fps_light_natural/validation.json) にコマンド、コンパイラ、入力・期待出力・checker・ソースの SHA-256 を記録しています。通常版の依存 7 ファイルが比較元と同一であることも検査しています。

228 ヘッダーと200提出例のサイトを生成し、204項目の配置一致、183の軽量実装、29組の相互リンク、軽量ページへの通常 FPS の非混入を検査しました。変更した24 Bundle の構文検査も成功しています。[サイト検査の記録](results/fps_light_natural/site_validation.json)。

## 再現

```sh
python test/benchmark/fps_light_speed.py --baseline cbcecde --sizes 1024 32768 131073 524288 --trials 7 --suite core --output test/benchmark/results/fps_light_natural/core.json
python test/benchmark/fps_light_speed.py --baseline cbcecde --sizes 1024 32768 --trials 7 --suite algorithms --output test/benchmark/results/fps_light_natural/algorithms.json
python test/benchmark/fps_light_speed_check.py --baseline cbcecde --cxx g++ clang++ --sanitize-cxx clang++ --acl /path/to/ac-library --problems /path/to/library-checker-problems --output test/benchmark/results/fps_light_natural/validation.json
python test/benchmark/fps_light_bundle.py --cxx g++ clang++ --output test/benchmark/results/fps_light_natural/bundle.json
```
