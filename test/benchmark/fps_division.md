# 多項式除算と GCD の専用変換

2026-10-19（Asia/Tokyo）に `ce81108` と比較した。公開 API は維持する。

参照は Division of Polynomials の [409561](https://judge.yosupo.jp/submission/409561)、Inv of Polynomials の [409589](https://judge.yosupo.jp/submission/409589)。原本の SHA-256 はそれぞれ `fd06ec63bfe326a8f8c2b25292ecf38d77c4fd0114fbaa2565e4c667b8b4eeb1`、`274d6c6107676bf6043aa5984e4b5788617ae7c8ff0ae1f157b6b0954e895306`。全体時間の比較では原本を書き換えていない。

## 不足していた処理と変更

従来の `div_series()` はおよそ 32 ブロックに分け、残差の葉の積、逆変換、次の商を求める積を別々に実行していた。多項式の `divmod()` でもこの級数除算を使い、反転した入力・商・積のために配列を複製していた。

専用実装は入力の長さに応じて商のブロック長を選び、逆数と窓の変換を共有する。残差は最大 8 項の積和と逆変換を融合して求める。剰余は短い商の直接計算、巡回積、分割した中央積、商の末尾を使う中央積から選ぶ。`fps_division_blocked.hpp` に分離した。FPS の一般的な `div_series()` にも同じブロック処理を接続する。

従来の Half-GCD は計算手順と変換キャッシュを持つものの、通常の多項式配列・NTT・成分ごとの積を使っていた。専用実装では一つの Arena に係数と変換を置き、LIFO の領域を再利用する。必要な行列成分を mask で指定し、同じ行の窓を共有した葉の積和、変換の倍増、3 項の積和をまとめた Montgomery 還元を使う。`polynomial_gcd_blocked.hpp` に分離した。GCD と拡張 GCD は最終的な正規化を維持する。

入力の Montgomery ワードを一様な $R=2^{32}$ 倍の通常の剰余値として使い、入口で Montgomery 表現を解除しない。商は $R$、法多項式に対する逆元は $R^2$ を掛けて戻す。結果は SIMD で変換し、既に存在する `Mint` オブジェクトへ表現をコピーしてから `vector::insert()` する。vector の要素外へ書き込まない。

内部の整列・padding は所有する作業領域で確保する。短い互除法が SIMD で読む前後と内部配列を初期化する。Linux x86_64 / GCC と AVX2 対応時に使い、それ以外は既存の実装へ戻る。係数型が内部型と異なる場合の変換は従来どおりである。

## 計測

AMD EPYC 9V74、GCC 14.2.0、Linux x86_64、CPU 2 固定。全版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でビルドし、assert を有効にした。各版 1 回予備実行した後、seed 20261019 で順序をシャッフルして 9 回測定した。計測中は排他ロックにより他のコンパイル・検証を停止した。以下は中央値、単位 ms。

計算部分は入力作成・出力を除き、API 内の確保と結果の作成を含む。

| 処理 | 入力係数数 | 比較元 | 専用版 |
| --- | --- | ---: | ---: |
| divmod | 274010 / 248523 | 5.691 | 2.526 |
| divmod | 463046 / 412907 | 10.160 | 4.451 |
| divmod | 455174 / 92222 | 14.978 | 7.347 |
| divmod | 429249 / 25839 | 13.235 | 5.623 |
| inv_mod | 50000 / 50000 | 44.229 | 13.452 |
| GCD | 50000 / 50000 | 45.617 | 14.243 |
| div_series | 500000 / 500000 | 14.385 | 10.830 |

プロセス全体は通常ファイルからの読み込み、計算、通常ファイルへの書き込み、起動・終了を含む。

| 処理・入力 | 比較元 | 専用版 | 最速原本 |
| --- | ---: | ---: | ---: |
| divmod v_random_00 | 12.047 | 9.497 | 6.250 |
| divmod v_random_01 | 18.369 | 13.721 | 7.967 |
| divmod v_random_02 | 22.402 | 15.602 | 11.627 |
| divmod v_random_03 | 20.102 | 12.986 | 9.503 |
| inv_mod max_random_00 | 48.194 | 16.814 | 16.062 |

この測定の I/O は作業中の snapshot である。共通 I/O はその後も改良されるため、全体の最終結果は総合計測を参照する。逆元の全体時間ではこの時点で最速原本との差が約 0.75 ms 残る。多項式除算には入力コピー・結果作成を含む API 側の時間と、I/O の差が残る。最速達成とは主張しない。

追加の診断では原本だけ通常の `main()` に変更し、`poly::divide()` の作業 Arena 確保から計算終了までを計測した。対応する専用 API の計算時間は、v_random_01 で 4.316 ms、原本の計算部分は 3.220 ms だった。入力・出力配列をあらかじめ確保する原本と、入力を保存し戻り値配列を作成する API の差も含むので、この約 1.10 ms を NTT カーネル単体の差とは扱わない。

全 9 試行、環境、入力・ヘッダー・バイナリのハッシュは [測定記録](results/fps_division.json)。計測時の [ドライバー](results/fps_division_measurement_driver.cpp) と [ハーネス](results/fps_division_measurement_harness.py) も保存した。ハーネスに含まれる `/tmp/fps-division` は当時の作業領域で、再実行には次の汎用版を使う。

```sh
python3 test/benchmark/fps_division.py --baseline ce81108 --official-root /tmp/library-checker-problems --references /tmp/fps-reference --output division.json --trials 9 --cpu 2
```

## 検証

多項式除算の公式 35 ケースと法多項式に対する逆元の公式 18 ケースを、`checker input actual official` の順で照合した。[全結果](results/fps_division_judge.json)に記録した。計測中の全出力も checker で確認し、GCD と級数除算は比較元とトークン列を照合した。

`fps_division_blocked.cpp` は 2 冪前後の長さ、短い除数、入力長と要求次数の相違、Montgomery ワードの上限側、末尾の 0、入力の保存を検査する。商と剰余から入力を復元し、級数除算は除数との積を要求次数で切って照合する。既存の `polynomial_gcd.cpp` は共通因子、疎な入力、零多項式、拡張 GCD の Bézout 恒等式、汎用 modint、逆元の積を検査する。両方を通常および ASan/UBSan で実行した。ローカルの LeakSanitizer は既存の ptrace 制約により `detect_leaks=0` とした。
