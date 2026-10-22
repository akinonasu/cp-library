# 乗法的関数の prefix sum

2026-10-22。素数上の prefix sum と素数冪の値を与えて、乗法的関数の和を計算する `MultiplicativePrefixSum<T>` を追加しました。商の列挙と添字計算は `FloorQuotients` に分離し、既存の `prime_sieve` を再利用しています。

## 参考

Library Checker の AC 時間順一覧と先頭 2 件ずつのソースを確認しました。順位・言語・表示時間・ソースのハッシュは `results/multiplicative_prefix_sum/references.json` に保存しています。

- [Sum of Multiplicative Function](https://judge.yosupo.jp/problem/sum_of_multiplicative_function)：`N <= 10^11`。[400957](https://judge.yosupo.jp/submission/400957) と [364205](https://judge.yosupo.jp/submission/364205) は表示 202 ms です。小さい素因数と中間の素因数を分けた和の計算、Dirichlet 畳み込みの復元、powerful number 上の補正を組み合わせます。単純な Min_25 篩より速い構成ですが、今回の実装とは異なります。
- [Large 版](https://judge.yosupo.jp/problem/sum_of_multiplicative_function_large)：`N <= 10^13`。[405757](https://judge.yosupo.jp/submission/405757) は表示 2746 ms、[295675](https://judge.yosupo.jp/submission/295675) は 2800 ms です。環の型と除算を抽象化した実装で、AVX2 による更新や powerful number の列挙を使います。今回は 405757 を `N = 10^12` の独立した照合にも使いました。
- [Sum of Totient Function](https://judge.yosupo.jp/problem/sum_of_totient_function)：`N <= 10^10`。[199449](https://judge.yosupo.jp/submission/199449) は表示 8 ms、[405756](https://judge.yosupo.jp/submission/405756) は 9 ms です。μ の和と畳み込みに特化した方法です。φ を汎用の乗法的関数として扱う場合より速く動作します。
- [公式の Min_25 実装](https://github.com/yosupo06/library-checker-problems/blob/master/number_theory/sum_of_multiplicative_function/sol/min_25_sieve.cpp)：素数上の表、Black algorithm の列挙、逆順の素数から全 prefix sum を復元する方法を確認しました。今回の構成と比較するため、元のソースをそのままコンパイルしました。

今回の素数上の表は、完全乗法的な補助関数の整数上の和から作ります。`f(p) = a+bp` は 1 と恒等関数の表を組み合わせます。素数の検出は独立した篩で行うため、値が 0 となる素数や、負の値・合成数 mod にも対応します。

表を更新するときは大きい商と小さい整数を分け、商の添字から参照先を直接計算します。小さい整数側では同じ商を持つ範囲をまとめます。最終的な和だけの `sum` は Black algorithm、全ての商に対する `prefix_sum` は逆順の Min_25 篩を使います。逆数や環での除算は使いません。整数平方根は 128 bit の積で補正し、素数冪を増やす前には除算による上限確認を行います。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2、CPU 0 に固定、`-std=c++23 -O2`。他のコンパイル・検証と重ねず、ウォームアップ 1 回と、固定 seed で実行順を入れ替えた 5 回の中央値です。`f(p^e) = 7e + 11p` を法 469762049 で計算しました。計算結果は全試行で照合しています。

まず、構築・素数上の表・集計の合計時間です。持続する計算領域の終了時の解放、プロセス起動・入出力は時間外です。各段階の全試行は `results/multiplicative_prefix_sum/measurements.json` に保存しています。

| N | 最終値だけ `sum` | 全商の表 `prefix_sum` |
| ---: | ---: | ---: |
| 10^6 | 0.226 ms | 0.370 ms |
| 10^8 | 4.797 ms | 8.004 ms |
| 10^10 | 119.906 ms | 183.913 ms |
| 10^11 | 636.323 ms | 918.182 ms |
| 10^12 | 3412.957 ms | 4643.896 ms |

次はプロセス起動・入出力・終了を含む時間です。参考実装は取得したソースを変更せずにコンパイルし、元のコンパイラ指定も維持しました。400957 の診断出力は `/dev/null` に送っています。

| N | 今回の `sum` | 公式 Min_25 | 最速 400957 |
| ---: | ---: | ---: | ---: |
| 10^8 | 6.287 ms | 7.992 ms | 4.988 ms |
| 10^10 | 123.059 ms | 148.604 ms | 53.152 ms |
| 10^11 | 641.107 ms | 723.549 ms | 196.904 ms |

φ の和も、汎用の指定で求めたものと特化した参考提出を比較しました。

| N | 今回 | φ 専用の 199449 |
| ---: | ---: | ---: |
| 10^8 | 7.493 ms | 1.685 ms |
| 10^10 | 151.890 ms | 8.217 ms |

公式の一般的な Min_25 実装より速く動作していますが、上の最速提出や φ 専用の計算法より速いとは主張しません。Library Checker の表示時間と、この環境での測定時間は直接比較できません。Large 版の全ケースを通過したという主張もしていません。

## 検証

- 小さい入力を直接素因数分解して計算した和と比較しました。φ・μ・約数個数・約数和・平方数の判定関数、素数上の二次多項式と高い冪に独立した値を持つ関数を含みます。
- 5 種の型で計 2525 問を照合しました。`int64_t`、既存 `mint`、法 469762049・1000000007・36 の独立した modint です。全ての商の prefix sum と、最終値の専用計算をそれぞれ検証しています。入力の表が変わらないことと再求解も確認しました。
- 法 4 の Dirichlet 指標、`__int128_t` による `N=10^8` の恒等関数の和と約数個数の和を確認しました。約数個数の和の正解は独立した双曲線法で計算しています。
- 比較演算・デフォルト構築を持たない環の型で動作を確認しました。Boost `cpp_int`、ACL の `modint1000000007`・合成数 mod 36 の `dynamic_modint` でも、GCC・Clang の `NDEBUG` 利用例を確認しました。
- 商の表は上限 0〜5000 の全列を直接生成した列と比較し、`10^12` の表の添字をランダムに 10000 回照合しました。コピー・ムーブ後の状態も確認しました。
- GCC・Clang の単独・二重 include とユニットテスト、Clang ASan・UBSan、両コンパイラでのドキュメント例が成功しました。constexpr 上限は通常 CI と同じです。
- Library Checker の公開ケースは、乗法的関数和 30 ケース、φ の和 48 ケースを GCC・Clang の両方で確認しました。さらに `N=10^12, a=7, b=11` の答え 331072216 は Large 版最速の 405757 と一致しました。AtCoder 用テストは追加していません。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2 --selection test/benchmark/multiplicative_prefix_sum_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/multiplicative_prefix_sum_selection.json
python3 test/benchmark/multiplicative_prefix_sum_reference.py --cache /tmp/multiplicative-refs --output /tmp/multiplicative-refs/references.json
python3 test/benchmark/multiplicative_prefix_sum.py --references /tmp/multiplicative-refs --checker-root "$LC_ROOT" --output /tmp/multiplicative-measurements.json
```

`LC_ROOT` は Library Checker の問題リポジトリのパスです。公開ケースは同リポジトリの `generate.py` で生成できます。検証記録とソースのハッシュは `results/multiplicative_prefix_sum/validation.json` に保存しています。
