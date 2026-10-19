# 約数列挙

2026-10-20。`math/number_theory/factorize.hpp` に `divisors(n, sorted = true)` と、`factorize_count` 形式の分解結果を受け取る overload を追加した。

素数の累乗と既存の約数の積を順に追加する。約数の個数を指数から求めて返り値の領域を確保し、最後に必要ならソートする。`false` はソートを省き、分解済みの入力を渡せば Pollard Rho も省く。1 は `[1]`、素因数分解の空配列も `[1]` を返す。

## 参考実装

Library Checker の Factorize の AC 実行時間順の先頭 3 件を公式 API から取得した。取得日時・一覧・各ソースの SHA-256 は `results/divisors/reference.json`、取得スクリプトは `divisors.py` に保存した。

| 提出 | 取得時の時間 | 確認した手法 |
| --- | ---: | --- |
| [405743 / Rohan_Kapri](https://judge.yosupo.jp/submission/405743) | 13 ms | 2 系列の Pollard Rho、600 個単位のまとめた GCD、素因数と指数を保持 |
| [317402 / aqxa2k](https://judge.yosupo.jp/submission/317402) | 13 ms | 同じソース。素因数の昇順ソートを引数で選択 |
| [317398 / aqxa2k](https://judge.yosupo.jp/submission/317398) | 13 ms | 同系統の処理。GCD の単位は 512 個 |

参考提出は素因数分解を返すもので、約数列挙そのものの比較対象ではない。2 系列の Pollard Rho とまとめた GCD を使う実装を確認したが、既存の 64 bit 全域対応の素因数分解・素数判定は変更していない。新機能は既存の素因数分解を呼ぶ。

参考提出との時間比較や、ジャッジ上の最速性は主張しない。以下は平方根まで試し割りする約数列挙と、新しい API の各呼び出し方の比較。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定。検証・コンパイル・他の測定と時間を分ける。
- seed `20261020`。`small` は 1～1,000,000 の乱数 10,000 個、`random32` は 1～4,294,967,295 の乱数 2,000 個。
- 入力生成・入出力を除き、返り値の確保・列挙・必要なソート・破棄・checksum を測定。`reused` と `reused_unsorted` は測定前に分解済みとし、その前処理時間を含まない。
- `trial` は平方根まで試し割りして約数の組を追加しソートする。`sorted` / `unsorted` は整数入力、`reused` / `reused_unsorted` は分解結果入力。`unsorted` の 2 種類はソートを行わない。
- 順序を seed で入れ替え、各 7 回実行した中央値。各試行で、約数の個数と総和から求めた checksum が全方式で一致することを確認。
- AtCoder・Library Checker 実機、macOS 実機での速度は未測定。

## 結果

単位は ms。各行は入力全体の合計時間。

| 条件・入力 | trial | sorted | unsorted | reused | reused_unsorted |
| --- | ---: | ---: | ---: | ---: | ---: |
| generic・small | 28.107 | 4.236 | 3.425 | 1.119 | 0.441 |
| generic・random32 | 338.818 | 2.049 | 1.710 | 0.423 | 0.102 |
| native・small | 27.020 | 4.046 | 3.511 | 1.058 | 0.408 |
| native・random32 | 338.801 | 2.059 | 1.755 | 0.407 | 0.103 |

今回の入力では試し割りより短い時間で列挙できた。既に素因数分解が必要な処理と組み合わせる場合は、分解結果を渡す版で重複した分解を省ける。ソートを省く効果も確認した。64 bit の大きい整数と約数の多い整数は正しさを検証しているが、速度は未測定。

条件とソースの SHA-256 は `results/divisors/generic/environment.json`・`native/environment.json`、全試行は `samples.csv`、中央値は `summary.json` に保存した。

## 検証

- GCC 15.2 / Clang 21.1.8 で変更ヘッダーと推移的な依存先をコンパイル・実行。
- 1～20,000 の全数を、各約数の倍数へ追加して作った独立した約数リストと比較。
- 1～100,000,000 の乱数 1,000 個を、平方根までの試し割りと比較。
- 各素数の累乗を `uint64_t` の上限まで確認。最大級の素数、32 bit 素数 2 個の積、約数の多い整数、`UINT64_MAX` を含む。
- 昇順・順不同・分解結果の再利用・素因数の並び順の反転・空の分解結果を検査。
- 約数の多い整数では、個数・昇順・重複なし・割り切れること・両端の積が元の数になることを確認。
- 既存の素因数分解・素数判定・離散対数・畳み込みと同時 include の単体テスト。
- 新しい約数列挙の単体テストを Clang ASan / UBSan で実行。
- 影響を受けるヘッダーと judge driver の Bundle を両コンパイラで確認し、ドキュメントの使用例もコンパイル・実行。

範囲と結果は `selection.json`・`check.log`・`sanitize.log`・`validation.log`・`validation.json` に保存した。macOS 実機はローカルでは未検証で、macOS CI の対象。

## 再現

```sh
flock /tmp/cp-wm-opt.lock python3 test/benchmark/divisors_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/divisors.py --output test/benchmark/results/divisors/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/divisors.py --native --output test/benchmark/results/divisors/reproduced_native
```

`reference.json` がなければ、測定スクリプトが参考提出を取得する。既存の記録があれば再取得しない。
