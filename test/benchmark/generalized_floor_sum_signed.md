# 一般化 floor sum の負の係数

2026-10-20。一般化 floor sum の両 API で、`a,b` に負の整数を渡せるようにした。`n,m` と非負の `a,b` の `uint64_t` 全域対応を維持し、符号付き・符号なしを混在させられる。yukicoder No.2996 の検証も追加した。

## 参考実装

[No.2996 の統計](https://yukicoder.me/problems/no/2996/statistics)の最速欄にある [1081475 / akakimidori](https://yukicoder.me/submissions/1081475) を確認した。取得時は Rust / AC / 49 ms。取得日時・ソースの SHA-256 は `results/generalized_floor_sum_yuki/reference.json`。

この提出は負の `a` に対して添字を逆順にし、`b` をユークリッド除算で正規化して、モノイドによる一般化 floor sum と二項展開を使う。今回は既存の全 64 bit 幅の引数に対応するため、`n*a` を作らずに `a,b` をそれぞれ正規化する方式を選んだ。参考提出との時間比較や、ジャッジ上の最速性は主張しない。

## 実装

$$a=u_am+r_a,\quad b=u_bm+r_b,\quad 0\le r_a,r_b<m$$

とすると、床関数は次のように分解できる。

$$\left\lfloor\frac{ai+b}{m}\right\rfloor=u_ai+u_b+\left\lfloor\frac{r_ai+r_b}{m}\right\rfloor$$

余りの部分を既存の非負版で計算し、`u_a*i` と `u_b` の加算を二項展開でモーメント表へ反映する。この処理は既存のユークリッド縮約で使う平行移動と共通化した。負でない入力は直接従来の経路へ渡す。整数の最小値をその型のまま符号反転せず、符号なしの絶対値で商と余りを計算する。

空の和は直ちにゼロを返し、項数 1 では `b` の床関数の累乗だけを求める。不要な `a` の高次の累乗を作らない。scalar API の `Q=0` も `a,b` を計算に使わない。

次数の計算量と型に関する制約は従来通り。整数型では中間値を含めて収まる必要があり、modint の標数は `D+1` より大きい必要がある。

## 検証

- GCC 15.2 / Clang 21.1.8、ヘッダー単体と既存の単体テストを含む検証。
- 符号付き係数の全探索 20,808 組を、128 bit の符号付き整数による直接の床関数・和と比較。C++ の除算の余りが負なら商から 1 を引く独立した計算を使う。
- 負を含む乱数係数、次数 6 の整数・次数 8 の modint、scalar / moments の一致。
- `INT64_MIN`・`INT64_MAX`・`UINT64_MAX` の符号混在、空の和、項数 1、次数 0、`Q=0`。
- 項数 `UINT64_MAX` でも、`a=-1,b=0,m=1` と `a=0,b=-1,m=UINT64_MAX` の符号の恒等式を確認。
- [No.2996](https://yukicoder.me/problems/no/2996) の公開サンプル 2 個と、生成した 8,218 ケースを両コンパイラで照合。7,492 ケースは直接和、726 ケースは床関数が整数の一次式になる入力を独立した多項式補間で検査した。
- `N=0`・`N=1`・`N=10^9`・法 998244353 の前後、指数 `p,q=0`～`10` を含む。各入力のテストケース数は問題の制約に合わせた。
- 単体テストと No.2996 の検証コードを Clang ASan / UBSan で実行。
- No.2996 の Bundle を GCC・Clang でコンパイル。

No.2996 は `test/generalized_floor_sum_yuki.test.cpp` の `PROBLEM` に登録し、通常の CI と一般化 floor sum の専用ワークフローの両方で検証する。ローカルでは公式全ケースを実行していない。CI では既存の `YUKICODER_TOKEN` を使う。未変更の他ライブラリの単体テストは再実行していない。

結果は `results/generalized_floor_sum_yuki/` の `check.log`・`sanitize.log`・`validation.log`・`validation.json`。Mac 実機はローカルでは未検証で、macOS CI の対象。

## 非負入力の測定

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。検証・コンパイルと同時に測定しない。generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`、native は `-march=native -ftrivial-auto-var-init=zero` を追加した。

`n,a,b` は 0～`10^9`、`m` は 1～`10^9` の乱数。seed は `20261020`、返り値は modint。入力生成・初回係数生成・入出力を除き、全モーメントの checksum を含む。順序を seed で入れ替え各 7 回実行した中央値を比較し、全試行の checksum が一致することを確認した。

`base` は commit `73dff6c` の変更前、`unsigned` は最終版に符号なし係数、`signed` は同じ非負の値を符号付き係数として渡す。単位は ms、入力全体の合計。

| 次数・個数 | generic base / unsigned / signed | native base / unsigned / signed |
| --- | ---: | ---: |
| 4・10,000 | 42.732 / 42.331 / 42.747 | 40.881 / 42.530 / 42.422 |
| 8・2,000 | 35.246 / 35.166 / 34.995 | 35.909 / 35.601 / 35.646 |
| 20・100 | 18.297 / 18.369 / 18.169 | 18.137 / 17.411 / 17.359 |

共通処理を単に切り出した最初の候補は汎用ビルドで約 4～8% 遅くなったため、平行移動をインライン化し、作業配列を呼び出し側に戻した。最終版の汎用ビルドは全次数で差が 1% 未満だった。native は次数 4 が約 4% 遅く、次数 8 は差が 1% 未満、次数 20 は約 4% 短縮した。全ての条件で同速・高速とは主張しない。負の係数そのものの速度、AtCoder・yukicoder 実機、Mac 実機は未測定。

初期候補は `initial_candidate.patch.gz`・`initial_generic`・`initial_native`、最終版は `generic`・`native` に保存した。各 `environment.json` は条件とソースの SHA-256、`samples.csv` は全試行、`summary.json` は中央値。

```sh
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/generalized_floor_sum_yuki/selection.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/generalized_floor_sum_yuki/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/generalized_floor_sum_yuki.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/generalized_floor_sum_signed.py --output test/benchmark/results/generalized_floor_sum_yuki/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/generalized_floor_sum_signed.py --native --output test/benchmark/results/generalized_floor_sum_yuki/reproduced_native
```
