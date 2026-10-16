# Fraction の実装・測定

2026-10-16 に [`math/fraction.hpp`](../../math/fraction.hpp) を追加した。
使い方は [`docs/fraction.md`](../../docs/fraction.md)。
`Fraction<Int = std::int64_t>` は、分母が正で常に既約な有限分数を扱う。
整数型は64 bit以下とし、演算結果の既約な分子・分母がその型に収まることを前提にする。

## 調査した実装

直接の汎用分数演算の問題に代えて、Library Checker の
[Rational Approximation](https://judge.yosupo.jp/problem/rational_approximation) の最速付近の AC 提出を調べた。
公式 API の `problem=rational_approximation&status=AC&order=%2Btime&limit=4` を使った。

| 提出 | ユーザー | judge 表示時間 |
| --- | --- | ---: |
| [290530](https://judge.yosupo.jp/submission/290530) | kgprac | 0.032 s |
| [290528](https://judge.yosupo.jp/submission/290528) | kgprac | 0.032 s |
| [290527](https://judge.yosupo.jp/submission/290527) | kgprac | 0.033 s |
| [405751](https://judge.yosupo.jp/submission/405751) | Rohan_Kapri | 0.034 s |

これらは Stern–Brocot 木上で境界を探し、比較には `a*y <= b*x` を使う。
上位の kgprac の提出は移動距離を倍々に広げる探索を使っている。
専用の入出力も含んでおり、汎用的な分数の四則演算を実装したものではない。
本実装でも比較は交差積を使うが、64 bit 全域の分子・分母を扱うため128 bitに広げて計算する。
judge の表示時間と以下のローカル測定は比較しない。

四則演算と正規化は
[Boost.Rational の公式資料](https://www.boost.org/doc/libs/1_85_0/libs/rational/rational.html) と
[実装](https://raw.githubusercontent.com/boostorg/rational/boost-1.85.0/include/boost/rational.hpp)
も参考にした。
分母の GCD を使う加減算、乗除算での交差約分、連分数を使う比較を確認した。
今回の型では倍幅の積が利用できるので、比較は除算の繰り返しを省ける。

取得した一覧、提出情報、ソースの SHA256、Boost の出典は
[`fraction_reference.json`](results/fraction_reference.json) に保存した。
第三者のソース全文は一時キャッシュに置き、リポジトリには追加していない。

## 実装上の選択

- コンストラクタで約分と符号の正規化を行い、以降も既約性を保つ。値は2個の整数だけで保持し、動的確保しない。
- 加減算は `g=gcd(b,d)` として、`a*(d/g) ± c*(b/g)` を求める。最後の約分はこの分子と `g` の GCD だけでよい。
- 乗除算は積の前に交差約分する。既約な結果をそのまま作るため、積全体の GCD を取り直さない。
- 32 bit以下の型では64 bit、64 bitの型では128 bitを中間計算と比較に使う。結果が格納型に収まる場合は中間の積が格納型を超えても扱える。
- 負の最小値の絶対値を符号付き型に作らず、符号なしの大きさを使う。GCD が `2^63` になる除算も扱う。
- GCD の一方が1なら即座に返す。通常は標準ライブラリの GCD を使い、定数評価にも対応する。
- 小さい中間の分子の余りは格納型の演算で求め、不要な128 bit除算を減らす。
- 整数の加減算、逆数、丸め、累乗、入出力を提供する。累乗では不要な最後の2乗を行わない。
- CPU 固有の命令や最適化 pragma は追加しない。外部の整数・分数ライブラリへ依存しない。

## 比較した候補と条件

比較コードは [`fraction.cpp`](fraction.cpp)、実行用は [`fraction.py`](fraction.py)。

- `baseline`: 毎回128 bitの交差積を作り、全体のユークリッド互除法で約分する単純な分数実装。
  [`fraction_baseline.hpp`](fraction_baseline.hpp) に保存した。比較だけは採用版と同じ倍幅の交差積。
- `library`: 採用版。
- `euclid`: 採用版の GCD だけを `%` を使うユークリッド互除法に変更した候補。
- `hybrid`: 採用版の GCD の最初に大きい方を小さい方で一度割り、その後は標準の GCD を使う候補。
- `continued`: Boost と同じ連分数比較の手法を独立に実装した比較処理。Boost のパッケージ全体を呼び出した測定ではない。

Intel Core i7-13700F、WSL2 Linux x86_64、GCC 15.2、CPU 2 固定で測定した。
汎用設定は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
native 設定はさらに `-march=native -ftrivial-auto-var-init=zero` を加えた。
20万入力、seed 713、順序をシャッフルして各5回の中央値を採用した。
探索は10万入力・各3回。
生成・入出力・検証は計測外で、結果を消費する checksum の計算を計測に含む。
構築以外の演算は、事前に正規化したオブジェクトを用意してから測定する。

| 入力 | 内容 |
| --- | --- |
| random | 分子の絶対値・正の分母が10億以下の独立な2分数 |
| same_den | 入力の分母を等しくする。構築時の約分後には異なることもある |
| integer | 分母が1の整数 |
| cancel | 互いに逆数で、積が1になる2分数 |
| wide | 64 bit全域の分子と正の分母。構築・比較だけを測定 |
| fibonacci | 大きな連続フィボナッチ数の比。構築・比較だけを測定 |

算術を測定する入力では、すべての結果が64 bitに収まる。
`wide` / `fibonacci` に対して、格納できない四則演算の時間を測らない。

## 結果

単位は ms、20万回の合計。通常範囲の `random` の結果:

| 操作 | 単純な baseline | 採用版・汎用設定 | 採用版・native |
| --- | ---: | ---: | ---: |
| 構築 | 15.568 | 14.450 | 14.597 |
| 加算 | 28.397 | 15.394 | 14.966 |
| 減算 | 28.098 | 16.149 | 15.203 |
| 乗算 | 27.776 | 28.477 | 27.724 |
| 除算 | 28.033 | 30.657 | 29.687 |

この条件の加算は約1.84倍、減算は約1.74倍だった。
乗除算の交差約分はこの入力では baseline より速くならなかった。
入力の大きさ・約分の起こり方によって差が変わるため、四則演算すべての高速化を主張しない。
整数の加算は baseline が1.828 ms、採用版が0.468 msだった。

比較の結果:

| 入力 | 交差積・汎用設定 | 連分数・汎用設定 | 交差積・native | 連分数・native |
| --- | ---: | ---: | ---: | ---: |
| random | 0.321 | 1.715 | 0.381 | 2.016 |
| same_den | 0.433 | 2.062 | 0.485 | 2.221 |
| integer | 0.374 | 1.179 | 0.399 | 1.444 |
| cancel | 0.384 | 0.837 | 0.418 | 0.847 |
| wide | 0.436 | 1.738 | 0.402 | 1.999 |
| fibonacci | 0.330 | 135.847 | 0.322 | 143.562 |

連分数が長く続く入力で差が大きくなる。交差積方式も、直接 Fraction を使ったソート全体の時間を保証する値ではない。
baseline と採用版は同じ比較手法だが、コード配置などによる測定差もある。

GCD 候補の例（汎用設定）:

| 操作 | 標準 GCD の採用版 | `%` の Euclid | 1回割ってから標準 GCD |
| --- | ---: | ---: | ---: |
| random 構築 | 14.450 | 15.042 | 14.721 |
| random 加算 | 15.394 | 15.579 | 15.292 |
| random 乗算 | 28.477 | 29.820 | 29.613 |
| fibonacci 構築 | 6.277 | 66.752 | 7.093 |

一部の操作で hybrid が速いものの、標準 GCD からの一律の変更は採用しなかった。
これらは今回の Linux/GCC 環境での実測であり、AtCoder や judge の実行時間を保証する数値ではない。

全標本、中央値、フラグ・コンパイラ・CPU 情報・ソースの SHA256:

- [探索](results/fraction_exploration/summary.csv)
- [汎用設定](results/fraction_generic/summary.csv)
- [native 設定](results/fraction_native/summary.csv)

各ディレクトリに `samples.csv` と `environment.json` も保存した。

## 検証

- 小さい分数の全組合せ、符号付き8 / 32 / 64 bit、正負の分母、0を検証した。
- 四則演算は、128 bit の交差積をすべて作ってから全体を約分する独立した素朴解と比較した。結果が格納型に収まる演算を対象とする。
- 10億以下のランダム4万組、64 bit全域3万組、型の境界、自己代入、逆数による相殺、丸め、累乗、入出力を検証した。
- `INT64_MIN`、分子が中間で64 bitを超える加算、正の分母の上限、EOF直前の整数入力も確認した。
- 各入力種類2万組で baseline・採用版・GCD 候補の演算結果を分子・分母ごとに比較した。連分数比較も交差積と照合した。本測定では checksum を照合した。
- GCC 15.2 / Clang 21.1.8 でヘッダー単体、Fraction の単体テスト、依存する `standalone.cpp` が成功した。
- Clang の AddressSanitizer / UBSan、ARM64 GCC 15.2 + QEMU aarch64 でも単体テストが成功した。
- `test/fraction.test.cpp` は Fraction の正規化・比較を使う Rational Approximation ドライバ。公式28ケースで GCC / Clang の両方が AC、計56回成功した。四則演算自体の検証には上の単体テストを使う。
- `oj-bundle` で展開したヘッダーも単体コンパイルに成功した。
- Mac 実機では追加実行していない。Apple Clang は既存の macOS portability CI で確認する。

対象は [selection](results/fraction_selection.json)、結果は
[unit](results/fraction_unit_check.log)、[sanitize](results/fraction_sanitize_check.log)、
[judge](results/fraction_judge_check.log)、[validation](results/fraction_validation.json) に記録した。

## 再現

Linux のリポジトリルートで実行する。CPU 番号は `--cpu` で変更できる。
出典の取得だけネットワークを使い、ベンチマーク本体は外部ライブラリを必要としない。

```sh
python3 test/benchmark/fraction_reference.py --cache /tmp/fraction-sources --output /tmp/fraction-references.json
python3 test/benchmark/fraction.py --output /tmp/fraction-generic --count 200000 --trials 5
python3 test/benchmark/fraction.py --output /tmp/fraction-native --count 200000 --trials 5 --native --skip-check
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/fraction_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/fraction_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/fraction.test.cpp
```

無関係なライブラリのテストは再実行していない。測定用ブランチは作成せず、commit・push も行っていない。
