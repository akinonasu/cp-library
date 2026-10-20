# 定数時間 mod pow / mod inv

`math/mod_inv.hpp` と `math/mod_pow.hpp` を測定した。法は実行時に固定し、底・指数はクエリごとに変更できる。逆元専用の構築では離散対数を計算しない。ModPow は逆元も提供し、ModInv の分数表と逆元表を共有する。

## 参考資料・提出

[maspy: O(1) mod inv, mod pow](https://maspypy.com/o1-mod-inv-mod-pow) の有理数近似と小さい値の表による手法、および大きい素数の対数を既知の小さい値から求める改善を参考にした。[実装](https://maspypy.github.io/library/mod/modfast.hpp) では固定サイズの分数表・逆元表・対数表と、原始根の累乗表を使っている。

2026-10-17 に、直接対応する離散対数の [Discrete Logarithm (Fixed Mod)](https://judge.yosupo.jp/problem/discrete_logarithm_fixed_mod) の AC を実行時間順に取得した。

| 提出 | 取得時のジャッジ時間 | 確認した方法 |
| --- | ---: | --- |
| [370158](https://judge.yosupo.jp/submission/370158) | 0.071 秒 | Farey 近似、正の小さい値の対数表、小さい法の全表、Pohlig–Hellman による初期素数の対数、滑らかな整数の表、固定乗算の高速化 |
| [354636](https://judge.yosupo.jp/submission/354636) | 0.078 秒 | 同系統の段階的な対数表構築と Farey 近似 |
| [355759](https://judge.yosupo.jp/submission/355759) | 0.102 秒 | 固定サイズの分数表・正負の対数表、原始根の累乗表、Barrett 縮約、BSGS と小さい素因数の利用 |

取得結果とソースのハッシュは `results/mod_fast_reference.json` に保存した。これらのジャッジ時間と下のローカル測定値は比較できない。上位提出とのローカルの時間比較は行っていない。

```sh
python3 test/benchmark/mod_fast_reference.py --cache .verify-helper/cache/mod-fast-references --output test/benchmark/results/mod_fast_reference.json
```

## 実装

分母上限を法の三乗根程度、バケット幅を二進数のべき乗に選び、Farey 数列を順に走査する。バケット中央で分子の絶対値が小さい側の近似分数を選ぶ。分母上限を D、バケット幅を W とすると、必要な分子の絶対値は p/D+DW/2 の切り上げで抑えられる。法に応じて表を確保し、バケットは右シフトで求める。小さい入力は分数への変換を省いて正の表を直接参照する。

mod pow は原始根に対する対数を取り、指数を p-1 で縮約して二つの累乗表を参照する。対数の前計算では小さい素数で分解する乱択を 64 回まで試し、成功しなければ BSGS を使う。seed は法と原始根から固定し、構築の再現性と有限回での終了を確保する。クエリにはループも乱択も使わない。

32 bit の法に対する縮約は 64 bit の逆数と積の上位 64 bit を使う。指数には UINT64_MAX まで渡せる。法 2・0 の累乗・0^0 も個別に扱う。x86 専用命令・AVX2 は不要。

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。法は 998244353 / 1000000007、各 1000000 クエリ、seed 731、各条件 7 回、実装の実行順は毎回シャッフルした。測定中は他のテストやベンチマークを同時実行していない。各実行は別プロセスで、前計算はコンストラクタ、クエリは入力済み配列の処理を測定し、時間を分けた。クエリ時間には共通の分岐・チェックサムの計算を含む。基準実装の縮約器はクエリループの前に一度構築する。

底は 1 以上 p 未満から一様に生成した。`pow` は指数が 0 以上 p-1 未満、`pow64` は 64 bit 全域から一様に生成した。全実装・全試行でチェックサムの一致を確認した。

- `library`: ModInv / ModPow。
- `binary`: 64 bit の積の剰余による通常の繰り返し二乗法。指数はそのまま処理する。
- `barrett`: 乗算を Barrett 縮約する二乗法。累乗では指数も p-1 で先に縮約する。
- `euclid`: 64 bit の係数を使う拡張 Euclid の互除法。
- `mint`: 既存 `math/mint.hpp`。998244353 固定。逆元は既存の加算鎖による実装。符号付き指数の範囲を超える `pow64` には使わない。

generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native はさらに `-march=native -ftrivial-auto-var-init=zero` を指定した。AtCoder 実機の測定ではない。以下は中央値、単位はミリ秒。

### generic

| 法 | 処理 | 実装 | 前計算 ms | 100 万クエリ ms |
| ---: | --- | --- | ---: | ---: |
| 998244353 | inv | barrett | 0.001 | 68.853 |
| 998244353 | inv | binary | 0.000 | 143.687 |
| 998244353 | inv | euclid | 0.000 | 75.208 |
| 998244353 | inv | library | 10.969 | 14.037 |
| 998244353 | inv | mint | 0.000 | 54.358 |
| 998244353 | pow | barrett | 0.001 | 97.480 |
| 998244353 | pow | binary | 0.000 | 114.867 |
| 998244353 | pow | library | 34.524 | 28.023 |
| 998244353 | pow | mint | 0.000 | 87.974 |
| 998244353 | pow64 | barrett | 0.001 | 97.392 |
| 998244353 | pow64 | binary | 0.000 | 281.757 |
| 998244353 | pow64 | library | 34.527 | 24.074 |
| 1000000007 | inv | barrett | 0.001 | 65.439 |
| 1000000007 | inv | binary | 0.000 | 118.749 |
| 1000000007 | inv | euclid | 0.000 | 76.284 |
| 1000000007 | inv | library | 11.333 | 15.097 |
| 1000000007 | pow | barrett | 0.001 | 98.040 |
| 1000000007 | pow | binary | 0.000 | 116.990 |
| 1000000007 | pow | library | 36.075 | 37.859 |
| 1000000007 | pow64 | barrett | 0.001 | 95.622 |
| 1000000007 | pow64 | binary | 0.000 | 286.316 |
| 1000000007 | pow64 | library | 35.285 | 25.038 |

### native

| 法 | 処理 | 実装 | 前計算 ms | 100 万クエリ ms |
| ---: | --- | --- | ---: | ---: |
| 998244353 | inv | barrett | 0.000 | 68.936 |
| 998244353 | inv | binary | 0.000 | 145.727 |
| 998244353 | inv | euclid | 0.000 | 75.389 |
| 998244353 | inv | library | 11.743 | 17.410 |
| 998244353 | inv | mint | 0.000 | 53.598 |
| 998244353 | pow | barrett | 0.001 | 95.924 |
| 998244353 | pow | binary | 0.000 | 112.901 |
| 998244353 | pow | library | 33.598 | 23.623 |
| 998244353 | pow | mint | 0.000 | 89.798 |
| 998244353 | pow64 | barrett | 0.001 | 99.143 |
| 998244353 | pow64 | binary | 0.000 | 284.392 |
| 998244353 | pow64 | library | 36.213 | 28.232 |
| 1000000007 | inv | barrett | 0.001 | 65.421 |
| 1000000007 | inv | binary | 0.000 | 117.814 |
| 1000000007 | inv | euclid | 0.000 | 76.309 |
| 1000000007 | inv | library | 11.740 | 14.588 |
| 1000000007 | pow | barrett | 0.001 | 96.639 |
| 1000000007 | pow | binary | 0.000 | 115.689 |
| 1000000007 | pow | library | 35.207 | 26.256 |
| 1000000007 | pow64 | barrett | 0.001 | 96.601 |
| 1000000007 | pow64 | binary | 0.000 | 283.103 |
| 1000000007 | pow64 | library | 34.799 | 24.492 |

前計算は使い回す。少数のクエリでは前計算を含む時間が不利になる場合がある。生の試行値・コンパイラ・CPU・入力条件・ソースのハッシュは `results/mod_fast_{generic,native}/` に保存した。

```sh
python3 test/benchmark/mod_fast.py --output test/benchmark/results/mod_fast_generic
python3 test/benchmark/mod_fast.py --native --output test/benchmark/results/mod_fast_native
```

## 検証

独立した通常の二乗法との照合、既存 mint との照合、逆元の積、原始根の対数と累乗の往復、64 bit 全域の縮約、UINT64_MAX の指数、小さい法の全要素、有理数近似の分子・分母の合同と上限、法 2・4096 付近・65536 付近・998244353・1000000007・1000000009・2147483647 を検証した。

GCC 15 / Clang 21 の単独 include・単体・他ヘッダーとの共存、Clang の ASan / UBSan、GCC 13、ARM64 / QEMU を通過。固定法離散対数の公式 24 ケースは GCC・Clang の両方で AC。全 API 表と公開用の結合ヘッダーも確認した。小さい値の直接参照を追加した後は、変更した二つのヘッダーと依存先だけを再検証した。ログと検証対象のハッシュは `results/mod_fast_*` に保存した。
