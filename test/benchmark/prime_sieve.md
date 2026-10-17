# prime_sieve の区間篩・走査高速化

## 条件と対象

- 測定日: 2026-10-17。
- Linux x86_64 の共有クラウド環境、Intel Xeon Platinum 8573C、GCC 14.2.0。
- `-std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero`。AtCoder に合わせた最適化オプションだが、AtCoder 本番の GCC 15.2・CPU での実測ではない。
- 変更前は `0a96edcbfda903acd7dcadd0e788be263c9589cf` の `math/prime_sieve.hpp`。同じ実装を `prime_sieve.cpp` の `baseline` として固定した。
- 改変前にも baseline を測定した。その後は同一バイナリ内の関数ポインタ経由で変更前後を交互に実行し、双方を一度 warm-up してから中央値を比較する。時間には返り値の構築・解放と軽い checksum を含める。
- `n` 未満の素数をすべて `std::vector<std::size_t>` に格納する。Library Checker の出力間引きと同じメモリ条件ではない。
- Apple Silicon / Intel Mac、Clang、AtCoder 本番での実行は未検証。実装は標準 C++ と GCC/Clang 共通の `__builtin_ctzll` のみを使い、x86 固有 ISA は要求しない。

## 変更

1. 小さい入力は奇数 byte 篩のまま、合成数を消すループと素数を列挙するループを分離し、全素数ごとの整数除算を除く。
2. 大きい入力は通常 33495 byte の区間に分ける。篩の素数の次の倍数を次区間に引き継ぎ、区間ごとの除算もしない。
3. 3・5・7・11 の倍数は、1155 byte の compile-time 周期パターンで先に除く。
4. little-endian では 8 byte のフラグを `memcpy` で読み、立っている bit の位置だけを列挙する。unaligned access / aliasing の仮定を置かず、他の byte order は byte 走査に戻す。
5. 出力容量を概算して reserve し、増設時のコピーを減らす。この概算が不足しても通常の vector 拡張で正しく動作する。
6. 区間幅は `sqrt(n-1)` 以上に広げられるため、固定幅区間ごとに全ての小素数を走査する余分な漸近計算量を生じない。時間 `O(n log log n)`、補助空間 `O(sqrt n)` を維持する。

## 再現

リポジトリのルートで実行する。

```sh
g++ -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -I . \
  test/benchmark/prime_sieve.cpp -o /tmp/prime-sieve-bench
/tmp/prime-sieve-bench 1000000 11 10
/tmp/prime-sieve-bench 10000000 11 1
/tmp/prime-sieve-bench 100000000 9 1
```

引数は `n`、測定回数、1 測定あたりの反復回数。出力は `n,baseline_ms,optimized_ms,baseline_checksum,optimized_checksum`。4 番目の引数に `0` または `1` を付けると、その実装だけを 1 回実行して `n,kind,count,last_prime` を出す。プロセス単位のメモリ測定に利用できる。

主な最終測定値を以下にまとめる。生ログや機械出力の CSV は commit しない。共有ホストであるため個々の wall-clock 値には変動があり、別の CPU やジャッジで同じ倍率を保証するものではない。

メモリ測定は `python3 test/benchmark/prime_sieve_memory.py /tmp/prime-sieve-bench 100000000`。各実装を別プロセスで実行し、`wait4` の peak RSS を KiB で記録する。

## 最終結果

以下は他の作業の CPU 実行を止めた区間で測定した中央値。共有ホスト全体の負荷までは制御していない。小入力を含め、測定した全ケースで変更後が速かった。

| n | 変更前 ms | 変更後 ms | 速度比 |
| ---: | ---: | ---: | ---: |
| 10 | 0.000181 | 0.000137 | 1.32x |
| 100 | 0.000231 | 0.000189 | 1.22x |
| 1,000 | 0.000875 | 0.000674 | 1.30x |
| 10,000 | 0.010365 | 0.005978 | 1.73x |
| 65,536 | 0.159813 | 0.048461 | 3.30x |
| 65,538 | 0.152309 | 0.034880 | 4.37x |
| 100,000 | 0.286214 | 0.080436 | 3.56x |
| 1,000,000 | 4.563151 | 1.331983 | 3.43x |
| 10,000,000 | 59.859547 | 16.305658 | 3.67x |
| 100,000,000 | 594.136829 | 153.188024 | 3.88x |

`n=10^8` の peak RSS は 115988 KiB → 46592 KiB (約 59.8% 減)。両実装とも素数個数 5761455、末尾 99999989。

`n=5*10^8` は他の検証と同時実行した初回のみで、12920.889486 ms → 1950.117812 ms。負荷の大きい参考値であり、上表の比較には混ぜていない。

## 正しさ

```sh
g++ -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -Wall -Wextra -Wconversion -I . \
  test/unit/prime_sieve.cpp -o /tmp/prime-sieve-unit
/tmp/prime-sieve-unit large
```

- `n=0..8192` を全件比較。
- 小入力分岐と区間境界の両方について、`2*32768*b`、`2*33495*b` (`b=1..8`、extended は `1..32`) の前後 17 を比較。
- 平方の前後 2 を比較。通常は `p<=257` と `509, 521, 1021, 2039`、extended は `p*p < 5000000` の全素数 `p`。
- seed `20261017` の 64 個 (extended は 300 個) の乱数入力と `n=5000000` を比較。
- 比較相手は独立した全整数 `vector<bool>` Eratosthenes 篩で、返り値の全要素を一致確認。
- `large` などの追加引数で extended に切り替わり、 `10^7`、`10^8`、`5*10^8` 以下の既知素数個数 (664579, 5761455, 26355867) と整列状態・上限も確認。
- 最終コードで上記を通過した。標準ライブラリを読み込んでから `__BYTE_ORDER__` を undef したビルドでも、byte 走査 fallback の通常テストを通過した (実機 big-endian の検証ではない)。推移的依存は `divisor_zeta` と `gcd_lcm_convolution`。これらと最終の通常テストは GCC の全体検証でも通過。ASan/UBSan の全体検証も成功した (`ASAN_OPTIONS=detect_leaks=0`)。再現手順は [検証のまとめ](portable_optimization.md) を参照。

## Library Checker / AC 実装との比較

2026-10-17 に次を確認した。

- [Enumerate Primes](https://judge.yosupo.jp/problem/enumerate_primes): `N <= 5*10^8`、上限は inclusive。ライブラリ呼び出しは `prime_sieve(N+1)` が相当する。
- [AC #77249, Nachia](https://judge.yosupo.jp/submission/77249): 公開ページで AC を確認。奇数だけを `vector<bool>` で持つ Eratosthenes 篩と、増築して保持する素数列。今回の API は呼び出し間でキャッシュしないため、同じインターフェイスの最適化としては cache-local な区間 byte 篩を採用した。
- [公式 correct.cpp](https://github.com/yosupo06/library-checker-problems/blob/master/number_theory/enumerate_primes/sol/correct.cpp): odd bitset を使う Atkin 篩。二次形式による反転と平方因子の除去を行う。今回の変更では乗り換えず、既存 Eratosthenes の不要な除算と大きな作業集合を減らした。
- [公式 eratosthenes.cpp](https://github.com/yosupo06/library-checker-problems/blob/master/number_theory/enumerate_primes/sol/eratosthenes.cpp): 全整数 bitset の Eratosthenes 篩。公式比較用実装で、ここでは AC / fastest と断定しない。
- [fastest 順の AC 一覧](https://judge.yosupo.jp/submissions/?order=%2Btime&problem=enumerate_primes&status=AC) も開いたが、利用可能なページ抽出では一覧行を取得できなかった。#77249 を fastest と呼ばず、現在の最速提出・順位・時間の保証はしない。

公開提出のジャッジ時間と今回のローカル時間は、CPU・コンパイラ・入出力・保存する素数列の量が異なるため直接比較していない。第三者コードはライブラリへコピーしていない。
