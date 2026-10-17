# 二項係数の prefix sum

`math/binomial_prefix_sum.hpp` は半開区間の和 `S(n,k)` を計算する。オンライン版は平方分割、一括版は Mo 法。

## 参考提出・資料

直接対応する Library Checker 問題は見つからなかったため、同じ prefix sum を使う [yukicoder No.2206 Popcount Sum 2](https://yukicoder.me/problems/no/2206) の AC 一覧を実行時間順で確認した。2026-10-17 の取得結果は `results/binomial_prefix_sum_reference.json`。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [1110156](https://yukicoder.me/submissions/1110156) | 307 ms | 幅 447 の Mo 法、階乗・逆階乗、通常の整数剰余 |
| [1110155](https://yukicoder.me/submissions/1110155) | 308 ms | 上と同一ソース |
| [1110477](https://yukicoder.me/submissions/1110477) | 320 ms | 問い合わせ数から幅を決める Mo 法 |

[maspy の prefix_sum_of_binom](https://maspypy.github.io/library/mod/prefix_sum_of_binom.hpp) の平方分割・Mo 法、[emthrm の multipoint_binomial_prefix_sum](https://emthrm.github.io/cp-library/include/emthrm/math/twelvefold_way/multipoint_binomial_prefix_sum.hpp) の Mo 法も確認した。

オンライン版は `m = min(k,n+1-k)` に折り返し、ブロック境界 `m` の値を `(m-1)!` 倍して保存する。中央の値から始めて `n >= 2m-1` の部分だけを前計算するため、各境界の行長は `N-2m+2`。配列を平坦化し、問い合わせでは近いブロック境界または中央から残りを直接足す。既定の mint では逆階乗の積を 128 bit に累積してから剰余を取る。

一括版は折り返した `(m,n)` を Mo 法で並べる。空・全体・端・中央の prefix は先に処理する。処理順は再利用できる `components/mo_order.hpp` に分離した。既存の `COMB_MAX` や comb の初期化は不要。

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。seed 20261017、各条件 7 回、実行順はシャッフル。全実装でウォームアップを 1 回行った。測定中は他のテストを実行していない。

同じ yukicoder 問題を解くオンライン版・一括版の検証プログラムと、取得した提出 1110156 のソースを比較した。入力は問題の制約内で N の上限・問い合わせ数とも 200000。`random` は n と 1..n の m がランダム、`fixed_n` は n=200000 で m がランダム、`edges` は n がランダムで m が 1、n、max(1,n-1) のいずれか。最後の問い合わせの n は上限に固定した。

子プロセスの起動、標準入出力、前計算、全問い合わせ、終了を含む実時間を測定した。入力生成・コンパイル・出力比較は時間外。今回の二つのプログラムは共通の FastInput/FastOutput、参考提出は元の cin/cout を使う。したがってアルゴリズムだけを分離した測定ではない。全入力・全試行で出力全体が一致した。

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。以下は中央値、単位は ms。AtCoder 実機での速度は未測定で、公開ジャッジ時間とローカル時間は直接比較しない。

| 条件 | 入力 | オンライン版 | 一括版 | 提出 1110156 |
| --- | --- | ---: | ---: | ---: |
| generic | random | 104.743 | 146.614 | 171.261 |
| generic | fixed_n | 86.337 | 28.806 | 34.872 |
| generic | edges | 75.296 | 14.267 | 119.416 |
| native | random | 106.938 | 146.692 | 171.383 |
| native | fixed_n | 86.001 | 28.891 | 34.944 |
| native | edges | 76.323 | 13.932 | 119.544 |

オンライン版は N=199999、B=447 の前計算を毎回含む。一括版は入力に応じて動かす距離が変わる。端の入力ではほぼ全てを Mo 法の前に処理できる。測定した条件では両 API とも参考提出より短い中央値だったが、入力・I/O・CPU に依存する。

```sh
python3 test/benchmark/binomial_prefix_sum_reference.py --cache .verify-helper/cache/binomial-prefix-references --output test/benchmark/results/binomial_prefix_sum_reference.json
python3 test/benchmark/binomial_prefix_sum.py --reference .verify-helper/cache/binomial-prefix-references/1110156.cpp --output test/benchmark/results/binomial_prefix_sum_generic
python3 test/benchmark/binomial_prefix_sum.py --reference .verify-helper/cache/binomial-prefix-references/1110156.cpp --output test/benchmark/results/binomial_prefix_sum_native --native
```

## 検証

Pascal の三角形による全件比較、複数のブロック幅、2・3・17・101・1000000007 の法を使う通常表現の modint、再構築、空入力、範囲外の k、INT64_MIN/MAX、ランダムなオンライン・一括比較、Mo 順での区間和を確認した。GCC・Clang、ASan/UBSan、GCC 13、ARM64/QEMU で通過した。

yukicoder 用の両プログラムを、小さい n の整数の全列挙による 136 ケースと公開サンプルで検証した。ローカルの全ケース取得は YUKICODER_TOKEN 未設定のため実行できず、`results/binomial_prefix_sum_judge_check.log` に記録した。GitHub CI は既存の secret を使って全ケースを検証する。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/binomial_prefix_sum_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/binomial_prefix_sum_selection.json
oj download -d .verify-helper/cache/binomial-prefix-samples https://yukicoder.me/problems/no/2206
python3 test/benchmark/binomial_prefix_sum_check.py --samples .verify-helper/cache/binomial-prefix-samples
```
