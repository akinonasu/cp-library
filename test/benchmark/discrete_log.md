# 離散対数

## 参考

2026-10-17 に Library Checker Discrete Logarithm の実行時間順の上位 AC を取得しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [361590](https://judge.yosupo.jp/submission/361590) | 31 ms | 非周期部分を分離、法と Euler 関数を素因数分解して位数を計算、位数全体に BSGS、固定長の線形探索ハッシュ表 |
| [361591](https://judge.yosupo.jp/submission/361591) | 32 ms | 同系統の実装。出力の改行処理が異なる |
| [405763](https://judge.yosupo.jp/submission/405763) | 33 ms | 同系統の位数計算と BSGS |

順位一覧・取得時刻・原文の SHA256 は `results/discrete_log_reference.json` に保存しました。参考提出の記録は手元の実測と比較できません。

## 実装

公開 API は `discrete_log(a,b,mod)` です。`a^x=b mod mod` を満たす最小の非負整数を `optional<uint32_t>` で返します。解がなければ `nullopt`、`0^0=1`、法が 1 の場合は 0 です。`uint32_t` の正の法全域と、`uint64_t` の未正規化の底・値に対応します。

- 最大公約数で非周期部分を取り除き、途中にある小さい解も確認。
- 底と法が互いに素になった後、目標値が単元でなければ終了。
- 最初の 32 個の累乗を調べ、小さい解や短い周期では前計算・表確保を省略。
- 既存の `factorize_count` で法と Euler 関数を素因数分解し、底の正確な位数を計算。
- 位数の素数冪ごとに Pohlig–Hellman 法で指数の桁を求め、中国剰余定理で復元。
- 素数位数の部分問題には BSGS。32 個以内なら直接探索し、それ以外は 64 bit に値と指数をまとめた連続配置のハッシュ表を使用。
- 剰余乗算は全 32 bit の偶数・奇数の法で使える `Barrett32`。逆元は拡張 Euclid 法。
- 復元した指数を最後に照合。合成数の非巡回な単元群でも、底が生成する部分群への所属を確認。

位数全体への BSGS と比べ、特に 998244353 などの滑らかな位数で探索量を削減できます。AVX2・共有の可変状態・固定サイズの大きな配列は使いません。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の時間ではありません。参考提出内の `Ofast`・unroll 指定は変更していません。

全実装に同じ通常ファイルを標準入力として渡し、プロセス起動・入力・計算・出力・終了を含む壁時計時間を測定しました。ウォームアップ 1 回の後、順番を混ぜた 7 回の中央値です。計測中に他のコンパイル・テストは実行していません。参考提出の大きいローカル配列に合わせ、全プログラムのスタック上限を 64 MiB にしています。

`unordered_map` は同じ Fast I/O を使う、64 bit の `%` と `std::unordered_map` による通常の拡張 BSGS です。参考提出は原文をビルドしました。361591 は調査のみです。

| 入力 | クエリ数 | 条件 |
| --- | ---: | --- |
| prime_yes | 600 | 998244353、1000000007、999999937、985661441 から法を選び、ランダムな底・指数から解のある値を生成 |
| composite_mixed | 600 | `10^8 <= mod < 10^9` のランダムな法。半分は既知の累乗、半分はランダムな値 |
| short_order | 3,000 | 底は 0・1・法から 1 を引いた値。解のある値と解のない値を混在 |
| even_impossible | 3,000 | 偶数の底と法、1 以外の奇数の値。解なし |
| full32 | 300 | `2^31 <= mod < 2^32` のランダムな法。半分は既知の累乗、半分はランダムな値 |

`full32` は Library Checker の制約外なので、参考提出を比較対象から除外しています。全出力を `%` による Python の累乗で確認し、独立した通常の拡張 BSGS と最小指数を一致させています。既知の累乗では、その指数以下であることも確認しています。

## 結果

単位は ms です。これらの入力・環境での値であり、全問題での最速を示すものではありません。

| 入力 | 今回 generic | unordered_map | 361590 | 405763 |
| --- | ---: | ---: | ---: | ---: |
| prime_yes | 32.642 | 561.275 | 114.517 | 112.493 |
| composite_mixed | 5.800 | 329.368 | 36.917 | 37.393 |
| short_order | 3.136 | 165.139 | 22.750 | 22.312 |
| even_impossible | 3.325 | 3.208 | 6.455 | 6.442 |
| full32 | 5.213 | 411.118 | — | — |

| 入力 | 今回 native | unordered_map | 361590 | 405763 |
| --- | ---: | ---: | ---: | ---: |
| prime_yes | 32.701 | 554.201 | 123.130 | 117.903 |
| composite_mixed | 5.685 | 320.763 | 38.120 | 36.312 |
| short_order | 3.760 | 163.702 | 22.515 | 22.514 |
| even_impossible | 3.251 | 3.097 | 6.303 | 6.357 |
| full32 | 5.065 | 404.384 | — | — |

通常の BSGS が最大公約数だけで終了する入力では、今回の実装も同程度です。これ以外の測定入力では、位数の分解と表の縮小によって探索時間を減らせました。生データ・コンパイラ・全オプション・入力と出力の SHA256・ソースの SHA256 は `results/discrete_log_generic/results.json` と `results/discrete_log_native/results.json` にあります。

## 検証

- 法 1〜160 の底・値の全組合せを、累乗列の直接列挙で求めた最小指数と比較。法以上の入力も同じ値になることを確認。
- 全 32 bit 範囲から選んだ 400 件を、ハッシュの向き・剰余演算が異なる拡張 BSGS と比較。
- 0、1、法から 1 を引いた底、2 冪の法、非周期部分だけに存在する解、非巡回群で部分群に属さない値を確認。
- 998244353 の大きい指数と、法 `2^31` における位数 `2^29` の最大指数を確認。
- Barrett 演算は 30,000 個の法で、全 `uint64_t` 範囲の剰余、未正規化の乗算、0・64 bit 指数の累乗を `%` と比較。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、リリース時の `-Werror` ビルド。
- Library Checker の公式 20 ケースを GCC・Clang で実行。
- 新しいヘッダーの単独 include、API 表と Bundle を検証。

## 再実行

```sh
python3 test/benchmark/discrete_log_reference.py --ids 361590 361591 405763 \
  --cache .verify-helper/cache/discrete-log-reference \
  --output test/benchmark/results/discrete_log_reference.json
python3 test/benchmark/discrete_log.py \
  --references .verify-helper/cache/discrete-log-reference \
  --output test/benchmark/results/discrete_log_generic
python3 test/benchmark/discrete_log.py --native \
  --references .verify-helper/cache/discrete-log-reference \
  --output test/benchmark/results/discrete_log_native
python3 test/verify.py --unit-only --cxx g++ clang++ \
  --selection test/benchmark/results/discrete_log_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/discrete_log_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/discrete_log.test.cpp
```
