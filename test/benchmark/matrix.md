# 行列

2026-10-17 に Linux x86_64 / GCC 15.2 で作成・測定した。公開 API は [使い方](../../docs/matrix.md) にまとめた。

## 参考にした AC 提出

Library Checker 公式 API の `status=AC&order=%2Btime&limit=3` で8問題の上位を取得し、ソースを調べた。各問題で最も速い C++ 提出をローカル比較にも使った。取得日時・順位一覧・ソース SHA256 は [matrix_reference.json](results/matrix_reference.json) に保存した。ソース全文は一時キャッシュに置き、リポジトリには追加していない。

| 問題 | 参照した C++ 提出 | judge 表示時間 |
| --- | --- | ---: |
| matrix_product | [408926](https://judge.yosupo.jp/submission/408926) / adamant | 49 ms |
| matrix_det | [405307](https://judge.yosupo.jp/submission/405307) / adamant | 9 ms |
| inverse_matrix | [408951](https://judge.yosupo.jp/submission/408951) / adamant | 16 ms |
| matrix_rank | [405300](https://judge.yosupo.jp/submission/405300) / adamant | 10 ms |
| system_of_linear_equations | [406222](https://judge.yosupo.jp/submission/406222) / Rohan_Kapri | 11 ms |
| pow_of_matrix | [401296](https://judge.yosupo.jp/submission/401296) / adamant | 11 ms |
| characteristic_polynomial | [404541](https://judge.yosupo.jp/submission/404541) / adamant | 32 ms |
| adjugate_matrix | [401318](https://judge.yosupo.jp/submission/401318) / adamant | 34 ms |

judge の表示時間と以下のローカル時間は別の値として扱う。

上位の行列積は Strassen–Winograd と AVX2 のタイル処理を使う。行列式・rank・方程式ではブロックごとの消去、逆行列では Schur complement と消去へのフォールバックも確認した。累乗・特性多項式では Frobenius form を使っている。提出には問題専用の入出力・領域管理・最適化 pragma も含まれる。

## 実装と比較

- 行優先の連続領域を使い、行列積と消去を `components/matrix_arithmetic.hpp` に分離した。
- 固定法の modint は `val()` で標準表現へ変換する。内部フィールドの直接参照や型の読み替えに依存しない。
- 行列積は64 bitの積和を16項ずつまとめる。法が `2^30` より大きい場合は4項に減らし、累積値と前回の余りの和が `2^64` 未満になるようにした。AVX2 版は変換と同時に B を8列のパネルへ詰め、4行・8列ずつ計算する。残り2行は SIMD、最後の1行はスカラーで処理する。
- 行の定数倍・加算は Shoup 乗算を使う。AVX2 では8要素を同時に計算し、比較用に同じ算術のスカラー版も残した。CPU 対応は実行時判定。
- 行列積の積和項数が128以下なら通常の演算を使う。小行列では内部表現への変換と領域確保の方が重かったためで、閾値を含む最終測定は [small_final](results/matrix_small_final/summary.csv) にある。
- 消去は前進消去と逆代入を分け、不要な列を更新しない。逆行列は単位行列を併合して計算する。
- 特性多項式は相似変形で Hessenberg 形にして計算する。乱数を使わない。累乗は環の要素にも使える二分累乗とした。Frobenius form による上位提出より遅い測定結果も下表に載せる。
- 余因子行列は単位行列を併合した消去を1回使う。rank が `n-1` なら右核と消去の最終行から左核を求め、1個の余因子で正規化する。rank が `n-2` 以下なら零行列。
- 符号付き整数の行列式は Bareiss 法、有理数は体の演算、浮動小数点は絶対値による部分ピボット選択を使う。

## 処理本体の測定

Intel Core i7-13700F / WSL2 Linux x86_64 / GCC 15.2、CPU 2 に固定。係数は既存の `mint`、seed 713。`-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native 設定では `-march=native -ftrivial-auto-var-init=zero` を加えた。実行順を固定 seed でシャッフルし、各7回の中央値を採用した。

`baseline` は同じ行列 API で、通常の `mint` の演算だけを使うラッパーを指定したもの。既存ライブラリからの改善率ではない。`scalar` は専用算術を有効にして AVX2 の明示的な呼び出しを無効にしたもの。入力生成と結果の出力は計測外、内部表現の変換・コピー・作業領域の確保は計測内。全結果を毎回比較した。

単位は ms。`pow` の指数は `10^18`、`singular` は最終行を先頭行と同じにしたもの。

| 操作・形 | baseline | scalar | 採用版 | native 採用版 |
| --- | ---: | ---: | ---: | ---: |
| product / dense / 64×64×64 | 0.302 | 0.098 | 0.055 | 0.044 |
| product / dense / 256×256×256 | 17.713 | 5.669 | 2.255 | 2.174 |
| product / dense / 512×512×512 | 139.057 | 44.393 | 18.133 | 18.575 |
| product / rectangular / 32×512×65 | 1.201 | 0.422 | 0.223 | 0.195 |
| det / dense / 512×512 | 47.191 | 41.271 | 9.159 | 8.822 |
| det / singular / 512×512 | 47.679 | 40.917 | 8.888 | 8.950 |
| inverse / dense / 384×384 | 88.122 | 76.339 | 16.128 | 16.115 |
| rank / rectangular / 256×512 | 14.689 | 13.013 | 2.966 | 2.986 |
| solve / rectangular / 256×512 | 26.561 | 23.086 | 5.208 | 5.248 |
| characteristic / dense / 256×256 | 25.068 | 18.824 | 13.123 | 13.000 |
| adjugate / singular / 192×192 | 13.881 | 11.967 | 2.914 | 2.830 |
| pow / dense / 64×64 | 24.856 | 7.261 | 2.603 | 2.487 |

標本・中央値・環境とソース SHA256 は [generic](results/matrix_generic/)・[native](results/matrix_native/) に保存した。初期の探索は [exploration](results/matrix_exploration/)・[small_exploration](results/matrix_small_exploration/) に保存した。探索は機能テストと並行して実行したため、速度の比較には上の最終測定を使う。

## 最速付近の提出とのローカル比較

CPU 2 固定、GCC 15.2、`-std=c++2b -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero` を両方に指定。最速提出は BMI 命令を含むため native 条件でコンパイルし、提出内の pragma や入出力は変更していない。プログラム起動・入出力・構築を含む経過時間であり、上の処理本体だけの時間とは直接比較しない。

seed 713 の入力で各7回の中央値。正方行列は384次、rank は384×421、累乗は128次・指数 `10^18`。方程式は384次の一意解を持つ入力とし、全出力を照合した。

| 問題 | 本ライブラリ | 参照提出の原文 |
| --- | ---: | ---: |
| matrix_product | 13.512 ms | 4.888 ms |
| matrix_det | 6.436 ms | 3.647 ms |
| inverse_matrix | 20.328 ms | 7.277 ms |
| matrix_rank | 7.080 ms | 4.171 ms |
| system_of_linear_equations | 8.155 ms | 4.186 ms |
| pow_of_matrix | 24.609 ms | 2.925 ms |
| characteristic_polynomial | 47.899 ms | 11.381 ms |
| adjugate_matrix | 20.921 ms | 12.800 ms |

汎用 API・決定的な消去・通常の二分累乗と、問題に特化した実装を比較している。この結果から AtCoder・Library Checker 上での最速や同じ速度比は断定しない。全標本・入力の SHA256 は [reference_local](results/matrix_reference_local/) にある。

## 検証

- 行列積を素朴解と全要素比較。1〜65付近の SIMD・積和の境界、最大に近い剰余、空の行・列を含む。
- 小行列の行列式・全余因子を置換の全探索と比較。mod 2 の3次行列512通りも検査。
- 逆行列の両側の積、解の代入、rank と核の次元、特性多項式の Cayley–Hamilton 恒等式と評価値を検査。
- GCC / Clang の単体テスト、ASan / UBSan、AVX2 無効、ARM64 GCC の静的バイナリを QEMU 上で実行して成功。
- 新規ヘッダーの単独コンパイルと公開用の展開コードも成功。Apple Clang 15 / macOS ARM64 の通常 CI と18個の機能別テストも成功。GCC 13.4 の全単体テストも成功。

| Library Checker | 各コンパイラのケース数 |
| --- | ---: |
| matrix_product | 22 |
| matrix_det | 25 |
| inverse_matrix | 26 |
| matrix_rank | 36 |
| system_of_linear_equations | 27 |
| pow_of_matrix | 43 |
| characteristic_polynomial | 28 |
| adjugate_matrix | 44 |

GCC / Clang の合計502ケースが成功した。[unit](results/matrix_unit_check.log)・[sanitize](results/matrix_sanitize_check.log)・[judge](results/matrix_judge_check.log)・[validation](results/matrix_validation.json) に記録した。変更箇所と依存するテストのみを実行した。Mac の切り分けに `codex/matrix-macos-check` を使った。ローカル・リモートともに削除済み。

## コンパイラ互換性

Apple Clang 15 が modint の compound requires による型判定でクラッシュしたため、`void_t` と型特性に変更した。[機能別18件](https://github.com/qandori/cp-library/actions/runs/39004747315) と [通常の macOS CI](https://github.com/qandori/cp-library/actions/runs/39004747319) でコンパイル・実行を確認した。

GCC 13.4 では、整数の Bareiss 消去を単体テストの `main` にインライン展開すると、MMX 命令を生成して `emms` を挟まず、その後の x87 計算が NaN になった。整数の行列式を `integer_determinant` に分離し、インライン展開を抑えて回避した。修正後は全単体テストが通り、実行ファイルにも MMX レジスタを使う命令がないことを確認した。許容誤差や modint の計算方法は変更していない。

上の性能測定は互換性修正前のもの。修正前後の GCC 15.2 のベンチマーク実行ファイルについて、generic・native ともに `.text` セクションがバイト単位で一致した。測定時のソース SHA256 は保持し、最終ソースの SHA256・コード一致・再現条件を [matrix_compatibility.json](results/matrix_compatibility.json) に記録した。

## 再現

Linux のリポジトリルートで実行する。参照提出は指定した一時キャッシュに保存する。

```sh
python3 test/benchmark/matrix_reference.py --cache /tmp/matrix-references --output /tmp/matrix-reference.json
python3 test/benchmark/matrix_reference_benchmark.py --cache /tmp/matrix-references --references /tmp/matrix-reference.json --output /tmp/matrix-reference-local --trials 7
python3 test/benchmark/matrix.py --output /tmp/matrix-generic --trials 7
python3 test/benchmark/matrix.py --output /tmp/matrix-native --trials 7 --native
python3 test/benchmark/matrix.py --output /tmp/matrix-small --trials 7 --quick
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/matrix_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/matrix_selection.json
oj-verify run --timeout 1800 --tle 10 -j 2 test/matrix_*.test.cpp
```

## 列パネル化の比較候補

2026-10-18 の main `056f9f366f0d1aed0401b5845b9294b61b33c926` に対し、既存の `val()` 変換と B の列パネル化をまとめ、AVX2 の葉を4行・8列へ変更した。公開 API、128項以下の通常演算、法に応じた16項/4項の安全な積和、汎用環・浮動小数点の演算順、ARM/スカラー経路は変えていない。Strassen や新しい剰余算術は導入していない。

[408926](https://judge.yosupo.jp/submission/408926) の packed 4×8 の葉を手法として参考にした。2026-10-18 02:48 UTC 時点の公式表示は49 ms。提出には Strassen、専用入出力、奇数法などの条件もあり、汎用 API と同条件ではない。今回、参照提出のコードは実行していない。

内部の GCC 14.2 / AMD EPYC 9V74 の短い探索では、初期候補の1024次積が309.912→128.101 msだった。変換・領域確保・パネル化・結果変換を含み、公開 judge 上の速度ではない。2行の SIMD 余りを残した後も、3×512×512 と6×512×512には main 比7.7%・6.6%遅い中央値があり、ばらつく範囲も重なった。これらを除外せず、最終比較の対照に含める。この探索だけでは最終計測・最新版 CI・実 Mac の合格を主張しない。

固定比較は次で再現する。両方に既存の `mint` の最適化経路を使い、`GenericMint` は比較対象にしない。

```sh
python3 -B test/benchmark/matrix.py --base-ref 056f9f366f0d1aed0401b5845b9294b61b33c926 --candidate-ref "$EXACT_CANDIDATE_SHA"
```

GCC 15.2、同一の native フラグ、利用可能な CPU 1個に固定し、12形状を各6組、AB/BA の順で交互に実行する。64/256/512/1024次、長方形・端数列・1列・1行・2/3/6行、4次の小行列反復を含む。変換・パネル化・領域確保を含む演算時間と、生成・全出力を含むプロセス時間を分ける。全要素を一時メモリ内で比較し、通常ログへは時間・形状・ハッシュ・ソース条件のみ出力する。生の標本をリポジトリへ追加しない。

### 3行以下の修正

[初回の固定比較](https://github.com/qandori/cp-library/actions/runs/39098123050) は `31f3158ec8e3e758707cecd933cf353f38345ba0`、GCC 15.2 / AMD EPYC 9V45 で実施した。全12形状・72組の全出力が一致し、256/512/1024次の演算時間中央値は main 比1.26/1.72/3.86倍速だった。1024次のプロセス時間は370.402→133.782 ms。

一方、3×512×512 は0.734672→0.791628 msで7.75%遅く、範囲も重ならなかった。6行は8.77%速く、2行は0.71%遅い中央値だった。変更していない1列のスカラー経路には、範囲が重なる18.82%遅い中央値も記録された。良い形状だけで合否を決めず、この初回結果を保持する。

4行の葉を一度も使えない `n < 4` では、main と同じ行優先の `val()` 変換と既存2行 AVX2・1行スカラー余りへ戻した。`n >= 4` の packed 4行・2行余りは変更していない。他の閾値は調整していない。この修正後は同じ12形状・6組の固定比較と必要な最新版 CI を再実行し、初回の速度値を修正後の実測値として使わない。
