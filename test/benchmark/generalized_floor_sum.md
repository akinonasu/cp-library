# Generalized Floor Sum の GCC 比較

## 参照

- [遭難者: floor sum アルゴリズムとその一般化](https://qiita.com/sounansya/items/51b39e0d7bf5cc194081)
- [yukicoder No.2362](https://yukicoder.me/problems/no/2362)
- [同問題の統計](https://yukicoder.me/problems/no/2362/statistics)。調査時点の最速は [akakimidori の Rust AC 1082005](https://yukicoder.me/submissions/1082005)、27 ms。monoid floor sum で多項式の和を集約している。
- [作問者の AC 882950](https://yukicoder.me/submissions/882950) は二次までの直接漸化式。[hos.lyric の AC 883884](https://yukicoder.me/submissions/883884) はモノイド方式。
- [maspypy の floor_sum_of_linear_polynomial](https://maspypy.github.io/library/mod/floor_sum_of_linear_polynomial.hpp) の分離した平行移動も確認した。

外部提出は手法の調査に用いた。比較コードと採用コードは独立に実装した。No.2362 の同種問題を Library Checker で探したが、通常の floor sum しか確認できなかったため、指定問題の AC 実装を調べた。

## 方式

- `monoid`: 既存 `monoid_floor_sum` に、総次数以下のモーメントと移動量を載せる。合成時に y・x を別々に二項展開する。
- `rectangle`: 単一の $$f_{p,q}$$ に必要な長方形のモーメントだけを持つモノイド。配列の縦横の要素数を $$P,Q$$ として、合成は $$O(PQ(P+Q))$$。
- `euclidean`: 記事の Euclid 型漸化式。べき和の係数と二項係数の二重和を分離し、1 段を $$O(D^3)$$ にする。整数では分母をまとめてから整除し、modint では係数の逆数を初回に前計算する。
- `baseline`: 二次までの漸化式を配列全体で返す特殊化。
- `current`: 一次は反復する通常の floor sum、二次は再帰内部の返り値を 3 種類の和に絞る。二次の三角数は整数引数の片方を先に半分にし、一次と一般次数は長さ 1 で早期終了する。一般次数の本体は `euclidean`。

## 測定条件

GitHub Actions の Ubuntu 24.04、Linux x86_64 / GCC 13.3、`-std=c++23 -O2`。アサート有無を別々に計測し、5 回の中央値を使用した。同一ジョブ内で実装の順序を毎回シャッフルする。入力生成・出力は計測外、戻り値全体を checksum に入れ、方式間の一致を検査する。Mac の時間は判断に使わない。

`random` は $$n,m\le10^9, a,b<m$$、`fibonacci` は Euclid の反復が多い $$m=701408733,a=433494437,n=10^9$$、`quotient` は $$n,m\le10^5,a,b<10^9$$、`small` は `random` の $$n<32$$ とした。次数 1・2 は各 20,000 問、次数 3 以上と単一モーメントは各 2,000 問を同じ入力で比較する。整数版は `__int128_t`、modint 版は本ライブラリの `mint` を用いる。

## 初回測定

[実行 38855397283](https://github.com/qandori/cp-library/actions/runs/38855397283)、ソース `9196ed7285e890196809955cc6cc9ed2edb6bef1`、AMD EPYC 7763。以下は `-DNDEBUG` の中央値（ms）。

| 入力 | 次数・型 | monoid | euclidean | 二次特殊化 |
| --- | --- | ---: | ---: | ---: |
| random | 1 / int128 | 37.109 | 8.318 | 3.197（一次） |
| random | 2 / int128 | 195.325 | 32.779 | 7.911 |
| fibonacci | 2 / int128 | 270.547 | 80.811 | 19.092 |
| random | 2 / mint | 211.590 | 43.842 | 29.107 |
| random | 4 / mint | 69.156 | 14.235 | — |
| random | 8 / mint | 331.533 | 66.394 | — |
| fibonacci | 8 / mint | 444.784 | 162.766 | — |
| quotient | 8 / mint | 276.434 | 36.777 | — |

二次の整数では 3 要素の返り値が配列全体を返す方式より約 1.5 倍速かった。一方、mint では外側でべき和を再計算する最初のラッパーが遅くなる場合があったため、外側の呼び出しだけ必要な全要素を返す形へ変更した。

## 次数別の比較

[実行 38855996028](https://github.com/qandori/cp-library/actions/runs/38855996028)、ソース `118f6bea44d573ce3a100e343d41b8ff717e2371`、Intel Xeon Platinum 8573C。以下は `-DNDEBUG` の中央値（ms）。

| 入力 | p:q | rectangle | euclidean | 採用版 |
| --- | --- | ---: | ---: | ---: |
| random | 0:2 | 11.735 | 4.753 | 3.290 |
| random | 1:1 | 15.201 | 4.711 | 3.321 |
| random | 0:4 | 24.058 | 15.428 | 15.396 |
| random | 1:3 | 38.933 | 15.395 | 15.380 |
| random | 2:2 | 43.417 | 15.443 | 15.396 |
| random | 0:8 | 67.651 | 64.954 | 64.790 |
| random | 1:7 | 118.533 | 64.876 | 64.931 |
| random | 4:4 | 172.131 | 65.356 | 64.835 |
| fibonacci | 0:2 | 15.068 | 11.521 | 8.137 |
| fibonacci | 1:1 | 19.904 | 11.538 | 8.184 |
| fibonacci | 0:4 | 31.721 | 37.605 | 37.633 |
| fibonacci | 1:3 | 50.944 | 37.571 | 37.541 |
| fibonacci | 2:2 | 56.648 | 37.579 | 37.755 |
| fibonacci | 0:8 | 90.310 | 158.346 | 158.125 |
| fibonacci | 1:7 | 154.327 | 158.476 | 158.174 |
| fibonacci | 4:4 | 224.887 | 158.379 | 158.622 |
| quotient | 0:2 | 9.763 | 2.623 | 1.800 |
| quotient | 1:1 | 12.695 | 2.623 | 1.813 |
| quotient | 0:4 | 19.841 | 8.521 | 8.468 |
| quotient | 1:3 | 32.467 | 8.480 | 8.500 |
| quotient | 2:2 | 36.819 | 8.522 | 8.537 |
| quotient | 0:8 | 56.428 | 35.988 | 35.902 |
| quotient | 1:7 | 97.536 | 36.027 | 35.951 |
| quotient | 4:4 | 143.698 | 35.927 | 36.039 |
| small | 0:2 | 1.180 | 0.614 | 0.524 |
| small | 1:1 | 1.511 | 0.613 | 0.526 |
| small | 0:4 | 2.375 | 1.822 | 1.824 |
| small | 1:3 | 3.843 | 1.825 | 1.821 |
| small | 2:2 | 4.237 | 1.834 | 1.833 |
| small | 0:8 | 6.590 | 7.323 | 7.333 |
| small | 1:7 | 11.537 | 7.646 | 7.622 |
| small | 4:4 | 16.492 | 7.340 | 7.320 |

長方形モノイドは `p=0,q=8` の Fibonacci 入力で約 1.75 倍速い一方、商が大きい入力では約 1.57 倍遅い。通常入力でも差が小さく、全体として漸化式を採用した。二次特殊化の root ではべき和を再計算しないようにし、mint で配列全体を返す基準実装との性能差を解消した。

## 小次数の追加候補

[実行 38856366450](https://github.com/qandori/cp-library/actions/runs/38856366450)、ソース `e38042a7aa26b7d9762dfdbecad7543e80624601`、AMD EPYC 7763。`raw` は二次の三角数を $$\lfloor n/2
floor(n-1+(nmod2))$$ で計算して modint の乗算を 1 回減らす。`one` は $$n=1$$ で再帰を止める。`rawone` は両方を適用する。以下は `-DNDEBUG` の中央値（ms）。

| 入力 | 次数・型 | 調整前 | raw | one | rawone |
| --- | --- | ---: | ---: | ---: | ---: |
| random | 1 / int128 | 3.198 | 3.222 | 3.144 | 3.216 |
| random | 2 / int128 | 8.005 | 7.954 | 8.021 | 8.047 |
| random | 2 / mint | 30.531 | 29.236 | 32.561 | 30.619 |
| fibonacci | 1 / int128 | 6.354 | 6.449 | 6.240 | 6.315 |
| fibonacci | 2 / int128 | 19.418 | 19.290 | 19.584 | 19.588 |
| fibonacci | 2 / mint | 75.315 | 72.843 | 79.173 | 76.108 |
| quotient | 1 / int128 | 1.907 | 1.888 | 1.807 | 1.867 |
| quotient | 2 / int128 | 4.491 | 4.481 | 4.475 | 4.476 |
| quotient | 2 / mint | 16.710 | 15.855 | 16.148 | 16.541 |
| small | 1 / int128 | 0.698 | 0.677 | 0.544 | 0.540 |
| small | 2 / int128 | 1.653 | 1.651 | 1.544 | 1.551 |
| small | 2 / mint | 5.127 | 5.087 | 4.626 | 4.572 |

二次は `raw` を採用する。二次の `one` は Fibonacci / mint で遅くなるため採用しない。一次は `one` を採用し、`raw` は差が小さいため採用しない。一般次数の `one` は小さい入力で次数 4 が 1.816 → 1.613 ms、次数 8 が 7.821 → 6.820 ms となるため採用する。

## 最終版

[実行 38856739715](https://github.com/qandori/cp-library/actions/runs/38856739715)、ソース `699cdc06456bdec1800c2699756d02a7ef759b2a`、Intel Xeon Platinum 8370C。採用した改善を組み合わせ、元の実装を固定したまま比較した。`-DNDEBUG` の中央値（ms）。

| 入力 | 次数・型 | monoid | euclidean | 最終版 |
| --- | --- | ---: | ---: | ---: |
| random | 1 / int128 | 61.377 | 10.145 | 4.214 |
| random | 2 / int128 | 156.089 | 29.258 | 8.410 |
| random | 2 / mint | 181.484 | 36.674 | 26.066 |
| random | 4 / mint | 54.171 | 11.517 | 11.411 |
| random | 8 / mint | 249.401 | 49.406 | 48.658 |
| fibonacci | 1 / int128 | 83.395 | 26.762 | 8.638 |
| fibonacci | 2 / int128 | 218.360 | 73.799 | 22.864 |
| fibonacci | 2 / mint | 249.324 | 91.934 | 67.622 |
| fibonacci | 4 / mint | 72.504 | 28.387 | 28.252 |
| fibonacci | 8 / mint | 329.099 | 120.577 | 119.192 |
| quotient | 1 / int128 | 50.526 | 5.224 | 2.426 |
| quotient | 2 / int128 | 127.809 | 15.435 | 4.524 |
| quotient | 2 / mint | 147.583 | 19.869 | 13.950 |
| quotient | 4 / mint | 45.041 | 6.355 | 6.204 |
| quotient | 8 / mint | 206.933 | 27.277 | 26.454 |
| small | 1 / int128 | 6.645 | 1.597 | 0.695 |
| small | 2 / int128 | 16.363 | 4.114 | 1.589 |
| small | 2 / mint | 18.468 | 4.977 | 4.344 |
| small | 4 / mint | 5.390 | 1.453 | 1.294 |
| small | 8 / mint | 24.691 | 5.928 | 5.056 |

一般次数の最終版は三角形モノイドに対し約 2.4〜8.1 倍、二次整数版は汎用漸化式に対し約 2.6〜3.5 倍速い。この値は同一 runner 内の比較であり、yukicoder 本番で最速順位を取ったという意味ではない。

## 検証

No.2362 の転倒数を、$$f_{p,q}(n,m,a,b)=\sum_{i<n}i^p\lfloor(ai+b)/m\rfloor^q$$ として次で計算する。

$$2f_{1,1}(n,m,a,b)-(n-1)f_{0,1}(n,m,a,b)+f_{1,1}(n,m,a,0)-nf_{0,1}(n,m,a,0)$$

最終版は公式 10 ケースを GCC・Clang の両方で全件 AC、単体テストと ASan・UBSan も通過した。ベンチマーク中の checksum 比較に加え、独立した直接和・転倒数の全探索、全 64 bit 幅の引数、整数と modint、GCC / Clang、ASan / UBSan を検査する。

再現コマンド:

```sh
python3 test/benchmark/generalized_floor_sum_variants.py /tmp/floor-variants
g++ -std=c++23 -O2 -DNDEBUG -I . -I test/benchmark -I /tmp/floor-variants test/benchmark/generalized_floor_sum.cpp -o /tmp/floor-bench
/tmp/floor-bench 20000 5
```

`.github/workflows/generalized-floor-sum-benchmark.yml` でも同じ比較と公式ケース検証を実行できる。`benchmark`・`verify` で測定と検証を個別に切り替えられる。単体検証は `verify_selected.py` から既存の検証スクリプトを呼び出し、測定用ワークフローの追加が通常 CI の既存キャッシュを無効化しないことも確認した。API キーは既存の GitHub Secret `YUKICODER_TOKEN` からのみ渡す。

2026-10-20 に負の係数対応と No.2996 の検証を追加した。変更内容・参考提出・検証・非負入力の測定は [負の係数の資料](generalized_floor_sum_signed.md) を参照。
