# 整数 k 乗根

## 参考

2026-10-17 に Library Checker Kth Root (Integer) の実行時間順の上位 AC を取得しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [229185](https://judge.yosupo.jp/submission/229185) | 18 ms | 指数ごとの定数化、平方根専用処理、逆立方根の多項式反復、12 乗根以降の閾値比較、下方に寄せた逆数による `pow` |
| [209944](https://judge.yosupo.jp/submission/209944) | 19 ms | `sqrtl`、`nextafter` で下方に寄せた逆数による `pow`、オーバーフローを検出する累乗、32 乗根以降の閾値比較 |
| [393720](https://judge.yosupo.jp/submission/393720) | 27 ms | 標準平方根・`pow` と整数による両方向の補正、32 乗根以降の分岐、AVX2 入出力 |

取得時刻・順位一覧・原文の SHA256 は `results/kth_root_reference.json` にあります。取得は `kth_root_reference.py` で再現できます。judge の記録は、下の手元の測定値と比較できません。

## 実装

`kth_root(n,k)` は、非負の `uint64_t` 全域と任意の正の `uint64_t` の指数に対応します。`k >= bit_width(n)` なら答えは 1 です。0 と 1、指数 1 は先に処理します。

- 平方根は `double` の `sqrt` を初期値とし、128 bit の平方で補正。
- 4 乗根・8 乗根の初期値は平方根の反復。他の 3〜15 乗根には `pow` を使用。
- 3〜15 の指数をテンプレートで定数化し、繰り返し二乗法のループを最適化しやすくする。
- 候補と隣接値の累乗を、オーバーフローを検出する 64 bit 乗算で比較。浮動小数点の推定が正解に隣接しない場合も、ビット長から求めた範囲内を二分探索して確定。
- 16〜40 乗根では、コンパイル時に生成した `a^k - 1` の表を二分探索。表は 2,600 byte で、範囲外の累乗は `UINT64_MAX` を番兵にする。
- 41 乗根以降はビット長だけで判定。

上位実装から指数の定数化と閾値比較を採用しました。`pow` の誤差方向に依存して補正を一方向に限定する方法は使わず、両方の不等式を確認します。ヘッダーに SIMD・CPU 指定・グローバルな最適化 pragma はありません。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の実測ではありません。

参考提出は原文のままです。229185 は元から `Ofast`・ループ展開・AVX2 の pragma を持ちます。GCC 15 が未使用のテンプレート内の未定義メンバーを診断するため、この提出だけ `-Wno-template-body` を付けています。`-UNDEBUG` は原文内の再定義警告を避けるためで、原文が直後に `NDEBUG` を定義します。393720 は AVX2 入出力のため `-mavx2` を追加しています。全追加フラグは生データにも記録しています。

各入力は 50 万クエリです。全プログラムに同じ通常ファイルを標準入力として渡し、全ての答えを出力させました。プロセス起動・入力・計算・出力・終了を含む壁時計時間です。ウォームアップ 1 回の後、実行順を混ぜた 7 回の中央値を採用しました。測定中に他のコンパイル・テストは実行していません。Python の多倍長整数による累乗比較で全出力を確認しています。

`binary` は同じ Fast I/O とオーバーフロー検出を使う整数二分探索です。`pow_corrected` は、全指数に `pow` を使って両方向に整数で補正する比較版です。上位提出は独自の入出力を使うため、表の差は計算関数だけの差ではありません。

| 入力 | 内容 |
| --- | --- |
| mixed | 指数 1〜64。整数の 3/4 は 64 bit 一様、1/4 はランダムなビット長 |
| small_k | 同じ整数分布、指数 2〜15 |
| square | 同じ整数分布、指数 2 |
| cube | 同じ整数分布、指数 3 |
| boundaries | 各指数の最大の根付近とランダムな根を使い、完全 k 乗数の前後 1 を抽出 |

## 結果

単位は ms です。

| 入力 | 今回 generic | binary | pow_corrected | 229185 | 209944 | 393720 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| mixed | 45.965 | 52.365 | 50.924 | 47.909 | 41.795 | 50.920 |
| small_k | 56.061 | 63.607 | 54.119 | 42.134 | 43.742 | 48.485 |
| square | 44.601 | 101.099 | 50.287 | 36.555 | 36.199 | 38.902 |
| cube | 51.703 | 79.009 | 46.349 | 38.879 | 41.134 | 41.704 |
| boundaries | 30.044 | 32.773 | 33.717 | 27.709 | 26.973 | 33.176 |

| 入力 | 今回 native | binary | pow_corrected | 229185 | 209944 | 393720 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| mixed | 45.954 | 58.600 | 54.815 | 37.620 | 41.355 | 43.855 |
| small_k | 51.375 | 62.995 | 50.430 | 41.545 | 44.971 | 46.354 |
| square | 42.348 | 97.267 | 49.173 | 33.967 | 35.007 | 37.645 |
| cube | 48.338 | 80.928 | 49.156 | 36.903 | 40.035 | 43.183 |
| boundaries | 32.003 | 36.223 | 37.633 | 30.957 | 30.579 | 34.788 |

整数二分探索より全ての入力で短い結果でした。最速の提出を全ての入力で上回ってはいません。小さい指数では、単純な `pow` 補正版より遅い場合もあります。

全サンプル・コンパイラ・入力/出力/ソースの SHA256 は `results/kth_root_generic/results.json` と `results/kth_root_native/results.json` にあります。ソースの SHA256 は LF に正規化しています。

## 検証

- 0〜10,000 と指数 1〜70 の全組合せを、除算でオーバーフローを避ける独立実装に照合。
- 指数 2〜63 ごとに 4,000 個の完全 k 乗数とその前後を検証。最大の根付近も含める。
- 25 万個の 64 bit ランダム値とランダムなビット長の値を検証。
- `UINT64_MAX`、`2^63`、最大指数、指数 64 以上、0・1 を検証。
- 4 種類の浮動小数点丸めモードでランダム値・平方数の前後・最大値を検証。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、ヘッダー単独 include、リリース時の `-Werror` ビルド。
- Library Checker の 15 ケースを GCC・Clang で検証。

## 再実行

```sh
python3 test/benchmark/kth_root_reference.py --ids 229185 209944 393720 \
  --cache .verify-helper/cache/kth-root-reference \
  --output test/benchmark/results/kth_root_reference.json
python3 test/benchmark/kth_root.py --references .verify-helper/cache/kth-root-reference \
  --output test/benchmark/results/kth_root_generic
python3 test/benchmark/kth_root.py --references .verify-helper/cache/kth-root-reference \
  --output test/benchmark/results/kth_root_native --native
python3 test/verify.py --unit-only --selection test/benchmark/results/kth_root_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/kth_root_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/kth_root.test.cpp
```
