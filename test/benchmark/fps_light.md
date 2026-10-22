# 64 KiB 制限向けの軽量 FPS

この資料は追加時の `5e7f6ec` の記録です。現在の実装の速度・ソースサイズは [高速化後の測定資料](fps_light_speed.md) を参照してください。

2026-10-21。既存の高速 FPS を変更せず、別の型 `LightFormalPowerSeries<T>`・別名 `light_fps` / `LightFPS` を追加しました。通常のソース整形のまま、基本演算と追加演算をまとめて include し、高速 I/O を加えても 32 KiB 未満です。64 KiB = 65,536 bytes の上限に対して、利用者の解法を書く余裕を残します。

## 実装と使い方

```cpp
#include "lib/fps/fps_light.hpp"

light_fps f{1, 2, 3};
auto inverse = f.inv(10);
auto logarithm = f.log(10);
```

- 基本演算は積・二乗・逆数・対数・指数・平方根・冪・級数除算・多項式除算・微積分・シフト・一点評価です。係数は既存版と同じ `mint`（998244353）を使います。
- `fps/fps_light_algorithms.hpp` を追加すると、`light_fps_algorithms` 名前空間で Taylor shift・Bostan–Mori・線形漸化式・多点評価・補間を使えます。評価と補間の積木は共通部品に分けました。
- 小さい DIF/DIT NTT、Montgomery 乗算、根の表のキャッシュを共有します。AVX2 は実行時に確認し、非対応 CPU にはスカラー経路があります。逆数の Newton 更新では既知の逆数の変換を使い回し、上半分だけを補正します。
- 既存版の大きな融合カーネル・演算別ブロック変換は取り込みません。合成・合成逆元・power projection・多項式 GCD は軽量版に含めていません。
- 既存版と軽量版は同じソースで併用できますが、両方を提出コードに展開すると 64 KiB を超えます。64 KiB 制限用の提出では軽量版のヘッダーを選びます。

基本演算は最大入力・出力長を `n` として $$O(n\log n)$$ 時間・$$O(n)$$ 作業空間です。加減算・微積分などは $$O(n)$$、多点評価と補間は積木を使う $$O(n\log^2 n)$$ です。根の表は、これまでの最大変換長分の領域を保持します。共有キャッシュを同時に拡張する並列呼び出しには対応しません。

返す係数数を指定する基本演算の上限は $$2^{22}$$、多項式除算の被除数も末尾の零を除いて $$2^{22}$$ 係数以下です。積は結果が $$2^{23}$$ 係数以下である必要があります。各追加演算でも途中の積の NTT 長が $$2^{23}$$ 以下であることが必要です。詳しい API は [基本演算](../../docs/fps_light.md)・[追加演算](../../docs/fps_light_algorithms.md) を参照してください。

## 通常の oj-bundle でのソースサイズ

online-judge-verify-helper 5.6.0 の `oj-bundle -I .` で実測しました。全プラットフォーム向けの分岐を残し、追加の minify・マクロ圧縮・関数削除は行っていません。ヘッダーの行は empty main を加えたコードです。

| 対象 | bytes | 65,536 bytes までの余裕 |
| --- | ---: | ---: |
| `fps/fps_light.hpp` | 18,531 | 47,005 |
| `fps/fps_light.hpp ＋ Fast I/O` | 26,130 | 39,406 |
| `fps/fps_light_algorithms.hpp` | 23,537 | 41,999 |
| `fps/fps_light_algorithms.hpp ＋ Fast I/O` | 31,136 | 34,400 |
| 最大の提出例 `test/fps_light_linear_recurrence.test.cpp` | 31,664 | 33,872 |

基本演算 7 件・追加演算 4 件の全 11 提出例も、すべて 32 KiB 未満でした。一般の利用者のコードや、さらに追加する別ライブラリまでの総サイズを保証するものではないため、提出時には完成したファイルの `wc -c` を確認してください。`test/fps_io.hpp` は既存の高速 FPS も読むため、この軽量版用には使いません。提出例は `test/fps_light_io.hpp` を使っています。

実際の 15 bundle のサイズ・SHA-256・コマンドと、展開済み指数・線形漸化式コードの GCC / Clang コンパイル結果は [bundle.json](results/fps_light/bundle.json) に保存しました。[sizes.json](results/fps_light/sizes.json) は別途使った独自 include-once 展開の記録で、こちらを oj-bundle の値とは扱いません。

```sh
oj-bundle -I . a.cpp > submit.cpp
wc -c submit.cpp
```

## 既存版との速度差

Linux x86_64 / AMD EPYC 9V74 の仮想環境、GCC 14.2、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効、各 9 回の中央値です。既存版は作業開始時の `6bb650a` の実装で、そのソースは変更していません。

同じ係数型・乱数 seed 731・入力で、既存版と軽量版を別の実行ファイルにします。実行順をシャッフルし、各測定は新しいプロセスで、NTT を事前実行しません。入力生成・結果の検証・入出力・プロセス起動を除き、演算中のメモリ確保・根の表の初期構築は含みます。全測定は他のコンパイル・テストと 2 つの lock で排他実行しました。

### 基本演算：524,288 係数

| 演算 | 既存版 ms | 軽量版 ms | 軽量版 / 既存版 |
| --- | ---: | ---: | ---: |
| 積 | 5.141 | 36.561 | 7.11 倍 |
| 二乗 | 4.996 | 26.154 | 5.23 倍 |
| 逆数 | 7.386 | 53.855 | 7.29 倍 |
| 対数 | 9.082 | 96.615 | 10.64 倍 |
| 指数 | 11.803 | 231.289 | 19.60 倍 |
| 平方根 | 7.449 | 168.474 | 22.62 倍 |
| 冪 | 19.725 | 321.281 | 16.29 倍 |
| 級数除算 | 9.863 | 92.239 | 9.35 倍 |
| 多項式の商・余り | 7.713 | 108.786 | 14.10 倍 |

積と二乗は入力が両方とも 524,288 係数で、出力は 1,048,575 係数です。冪の指数は 123456789、多項式除算の除数は 262,144 係数です。平方根は先頭係数を 17² にしており、計測前に多項式を二乗する方法による NTT の事前構築を避けています。

1,024・32,768・131,072・131,073 係数も含め、9 演算 × 5 サイズ、合計 810 サンプルを測定しました。131,073 は 2 冪でない長さです。[全 45 条件の集計](results/fps_light/benchmark.json)・[生データ](results/fps_light/benchmark.csv) に保存しています。

### 追加演算：32,768 係数または点

| 演算 | 既存版 ms | 軽量版 ms | 軽量版 / 既存版 |
| --- | ---: | ---: | ---: |
| Taylor shift | 0.381 | 2.660 | 6.98 倍 |
| 多点評価 | 2.887 | 96.298 | 33.36 倍 |
| 補間 | 3.813 | 122.926 | 32.24 倍 |
| 線形漸化式の第 k 項 | 12.683 | 326.674 | 25.76 倍 |

評価・補間の点は 0..n-1、Taylor shift は 17、線形漸化式の項番号は 10^18 です。補間の値は両版とも同じ乱数で用意し、計測前の多点評価は行いません。1,024 の場合も含む 4 演算 × 2 サイズ、144 サンプルの [集計](results/fps_light/algorithms.json)・[生データ](results/fps_light/algorithms.csv) を保存しました。

軽量版は既存版より遅く、最大サイズの基本演算では今回約 5.2〜22.6 倍の所要時間でした。ソースサイズ上限に応じて使い分けます。これはローカルの計算部分の測定で、Codeforces・AtCoder・Library Checker 実機での時間や実行時間制限内の完了を保証する値ではありません。

すべてのサンプルで返る長さと全係数を使う checksum が一致しました。多項式除算は商と余りの両方を含みます。使用したソース・コンパイラ・フラグ・実行ファイルのハッシュも各 JSON に記録しています。

## 公開 AC 実装との比較

既存の最速・上位提出の調査資料を再確認しました。取得時点は 2026-10-18 で、今回ランキングを取り直したとはしていません。11 問題各 2 件の ID・表示時間・ソースの SHA-256・元資料は [reference.json](results/fps_light/reference.json) にあります。

| 演算 | 参考にした提出 | 軽量版との違い |
| --- | --- | --- |
| 積 | [409465](https://judge.yosupo.jp/submission/409465)・[409433](https://judge.yosupo.jp/submission/409433) | 上位版の融合 radix-8/4 カーネル全体を読み込まず、小さい共有 DIF/DIT NTT を使用 |
| 逆数 | [409370](https://judge.yosupo.jp/submission/409370)・[409242](https://judge.yosupo.jp/submission/409242) | 同じ Newton 倍増・上半分の循環積補正を、小さい共通 NTT で実行 |
| 対数 | [409452](https://judge.yosupo.jp/submission/409452)・[409363](https://judge.yosupo.jp/submission/409363) | 専用のブロック対数微分を使わず、微分・逆数・積・積分を共有 |
| 指数 | [409662](https://judge.yosupo.jp/submission/409662)・[409661](https://judge.yosupo.jp/submission/409661) | 専用の変換再利用ではなく、共有 log を使う Newton 更新 |
| 平方根 | [409728](https://judge.yosupo.jp/submission/409728) | 上位版の逆数・部分変換の保持を使わず、共有の級数除算で Newton 更新 |
| 多項式除算 | [409561](https://judge.yosupo.jp/submission/409561) | 逆順の級数除算と積で商・余りを求める |

公開 judge の表示時間と今回の時間は比較しません。judge への提出や fastest の獲得確認は行っていません。

## 正しさ・互換性

- 基本演算は独立した uint64 の剰余演算と愚直配列による 152,676 チェックを GCC / Clang で通過しました。空入力、長さ 1、2 冪の前後、定数項が 1 でない逆数、指定精度が入力より長い場合、零・先頭の零・平方非剰余、`LLONG_MAX` の冪指数、末尾の零を含む除算を検証しました。
- 追加演算も愚直 Taylor shift・逐次生成した漸化式・Horner 評価・独立した補間などと照合しました。
- 両方の単体テストは Clang ASan/UBSan で成功しました。NTT は別途 1,400 組の愚直な積と、65,536 点までの往復・スカラー / AVX2 の一致を確認しました。
- 実際の ACL `modint998244353` を係数型にした基本演算も、既定型と一致しました。既存 FPS と同じ翻訳単位で include する比較も成功しました。
- 新規 4 ヘッダーは GCC / Clang の単独・二重 include と `-DNDEBUG -Werror`、利用者マクロへの非依存を確認しました。ドキュメント 3 例も両コンパイラで実行して結果を確認しました。
- 公式生成器の全 314 入力を GCC / Clang で実行し、計 628 回すべて公式 checker を通過しました。基本演算は積 53・逆数 25・対数 25・指数 26・冪 37・平方根 35・多項式除算 35、追加演算は Taylor shift 38・多点評価 11・補間 9・線形漸化式 20 です。

公式入力・正解・checker・ソースのハッシュ、使用した生成器のコミット、コンパイラ・コマンド・結果は [基本演算の検証](results/fps_light/validation_core.json)・[追加演算の検証](results/fps_light/validation_extras.json) に保存しました。Mac 実機での確認は行っていません。

## 再現

```sh
python3 test/verify.py --unit-only --selection test/benchmark/fps_light_selection.json --cxx g++ clang++
python3 test/verify.py --unit-only --sanitize --selection test/benchmark/fps_light_selection.json --cxx clang++
python3 test/benchmark/fps_light_bundle.py --bundle /path/to/oj-bundle
python3 test/benchmark/fps_light.py --sizes 1024 32768 131072 131073 524288 --trials 9
python3 test/benchmark/fps_light_algorithms.py --sizes 1024 32768 --trials 9
```
