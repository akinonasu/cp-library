# 二項係数の prefix sum（差分更新）

`math/combinatorics/binomial_prefix_sum_incremental.hpp` は、前回の `(n,k)` と値を保持し、呼び出し順に差分を処理する。並べ替えや平方分割の表は持たない。

## 参考資料

直接対応する Library Checker 問題はなく、既存の [二項係数 prefix sum の調査](binomial_prefix_sum.md) と [maspy の prefix_sum_of_binom](https://maspypy.github.io/library/mod/prefix_sum_of_binom.hpp) を確認した。既存調査の yukicoder No.2206 最速 C++ 提出は [1110156](https://yukicoder.me/submissions/1110156) で、Mo 法を使う。今回の API は並べ替えをせず、同じ漸化式で利用者が指定した順に更新する。

## 実装

`n` を増やすときは `S(n+1,k)=2S(n,k)-C(n,k-1)`、減らすときは `S(n-1,k)=(S(n,k)+C(n-1,k-1))/2` を使う。`n` の増加、`k` の変更、`n` の減少の順で処理し、区間和の添字を有効範囲に保つ。

`k` の変更は共通部品の `range` でまとめて加減算する。既定の mint では積を128 bitに累積し、階乗の乗算と剰余処理をまとめる。空・先頭1項・全体の和は直接設定し、空の状態からの行移動も省略する。

クエリ間の距離で実行時間が変わるため、任意順の大量クエリに対して既存の Mo 版・平方分割版より速いとは限らない。今回は対象環境での速度比較を行っていない。

## 検証

Pascal の三角形による小さい入力の全遷移・ランダムな順序との比較、同一クエリ、行と右端の両方向の変更、範囲外の k、INT64_MIN/MAX、再構築、複数インスタンス、法2・3・17・101・1000000007、n=500000 の中央・端の恒等式を確認した。

macOS arm64、GCC 15・Clang の `-std=c++2b -O2 -Wall -Wextra -DDEBUG=1` で通過した。Clang の ASan/UBSan と `oj-bundle` 後の GCC 構文検査も通過した。速度の測定ではない。

```sh
python3 test/verify.py --unit-only --cxx g++-15 clang++ --selection test/benchmark/results/binomial_prefix_sum_incremental_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/binomial_prefix_sum_incremental_selection.json
```
