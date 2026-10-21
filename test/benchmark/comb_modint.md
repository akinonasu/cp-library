# 二項係数の modint 型指定

2026-10-21。`math/combinatorics/comb.hpp` に `comb<Mint>(n,k)` と `Comb<Mint>` を追加しました。既定型では従来の `comb(n,k)`・`fact[i]`・`finv[i]` を使えます。`fact`・`finv` は負の添字を 0 とする読み取り用オブジェクトに変更し、既定の `COMB_MAX` は 10000000 にしました。

## 実装

- 自作 `mint` は起動時の前計算を継続し、既存の SIMD 階乗生成を使います。`Comb<mint>` もこの表を再利用します。
- 別型は初回の有効な問い合わせで前計算します。階乗・逆階乗・二項係数の問い合わせは型ごとに同じ表を共有します。未使用の型や、負の添字など 0 を返すだけの問い合わせでは表を生成しません。
- `Comb<Mint>::fact[i]`・`Comb<Mint>::finv[i]` と `Comb<Mint>::comb(n,k)` を使えます。局所変数で `const auto &fact = Comb<Mint>::fact` と束縛すれば、別型でも `fact[i]` と書けます。
- 前計算の部品は `components/factorial_table.hpp` に分離しました。素数法 `p` に対し、上限を `min(COMB_INF,p-1)` とします。乗算と `inv()` を使う汎用経路では、末尾の逆元を 1 回だけ計算します。
- 動的 modint では現在の法と構築時の法を比較し、法が変わった場合だけ作り直します。返す値はコピーなので、再構築後に古いテーブルへの参照が残ることはありません。

既定の 2 表は合計 80000008 バイトです。`constinit` とゼロ表現の `bit_cast` で領域を定義し、Clang の constexpr 評価で 20000002 個の modint のデフォルト構築を逐次評価する必要をなくしました。SIMD 生成へ渡す領域は従来と同じ modint の連続領域です。実行時の生成処理は変更していません。

`fact`・`finv` は C 配列ではなくなります。添字による読み取りを維持し、負の添字を扱うための変更です。配列の型検査・ポインタへの変換・テーブルへの書き込みは公開 API に含めません。既存の検証コード・測定コードも新しいアクセサーに合わせました。測定コードは従来のソースを比較元として使う場合にも対応します。

## 参考提出

Library Checker の [Binomial Coefficient (Prime Mod)](https://judge.yosupo.jp/problem/binomial_coefficient_prime_mod) の AC 実行時間順で先頭 3 件のソースを取得・確認しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [223248](https://judge.yosupo.jp/submission/223248) | 182 ms | 静的・動的 modint の分岐、法で前計算範囲を制限、順方向の階乗と末尾の逆元からの逆階乗 |
| [196412](https://judge.yosupo.jp/submission/196412) | 186 ms | 静的・動的 modint の階乗テーブル、逆元 1 回と逆方向の積 |
| [196634](https://judge.yosupo.jp/submission/196634) | 187 ms | modint を型引数に取るテーブルと同様の逐次積 |

汎用経路は同じ階乗・逆階乗の構成です。本実装では任意の modint の公開演算を使い、内部表現を仮定しません。既定型だけ既存の SIMD 経路を維持します。取得時刻・一覧・ソースの SHA-256 は `results/comb_modint/reference.json` に保存しています。上記の時間は公開 judge の値です。公開 judge への提出は行っていません。

## SIMD と愚直版の比較

既定の自作 `mint`（法 998244353）を使い、0 から `N` までの階乗・逆階乗を両方生成する時間を比較しました。

- 愚直版は `fact[i] = fact[i-1] * mint(i)` と末尾の逆元 1 回、後ろ向きの逆階乗生成です。
- スカラー版は `fps_detail::factorials` の既存のスカラーフォールバックと同じ内部ワード処理です。これを別に測ることで、modint の構築処理などと SIMD の効果を区別します。
- SIMD 版は実際の `fps_detail::factorials` を呼び、AVX2 の処理を使います。

Linux x86_64 / AMD EPYC 9V74 の仮想環境、CPU 2 固定、GCC 14.2、`-std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero -DATCODER -DONLINE_JUDGE`。9 回の中央値です。各試行で実行順をシャッフルし、全試行を他の検証・測定と排他実行しました。

| 上限 N | 愚直版 ms | スカラー版 ms | SIMD 版 ms | 愚直版 / SIMD 版 |
| ---: | ---: | ---: | ---: | ---: |
| 1,000 | 0.006260 | 0.006216 | 0.003265 | 1.92 倍 |
| 100,000 | 0.614510 | 0.613331 | 0.169060 | 3.63 倍 |
| 1,000,000 | 6.156040 | 6.134991 | 1.679235 | 3.67 倍 |
| 10,000,000 | 61.561214 | 61.379543 | 16.931453 | 3.64 倍 |

既定上限 10000000 では、愚直版に対して約 3.64 倍、所要時間は約 72.5% 減りました。スカラー版との比較でも約 3.63 倍です。

`-march=native` を省いた同じ CPU では、上限 10000000 で愚直版 67.692627 ms、スカラー版 61.555424 ms、SIMD 版 23.158813 ms でした。愚直版比は約 2.92 倍、スカラー版比は約 2.66 倍です。こちらも実行時判定で AVX2 を使います。

時間に含めるのは確保済み領域への全要素の生成です。メモリ確保・初期ゼロ化・プロセス起動・入出力・検証は除外します。初回に 1 回生成した後、`max(1,min(10000,10000000/(N+1)))` 回の平均を 1 試行の値にしました。したがって、プログラムの起動全体の測定ではありません。初回生成の時間も生データには別に記録しています。

各プロセスの測定後、通常の 64 bit 剰余演算で全ての階乗・逆階乗を要素ごとに照合しています。全方法・全試行で一致しました。[測定条件・集計](results/comb_modint/simd.json)と[全試行の CSV](results/comb_modint/simd.csv)を保存しています。AtCoder 実機での測定ではありません。

## 検証

- 独立した Pascal の三角形との全比較。法 2・3・17・998244353・1000000007、負の `n`・`k`・添字、`INT_MIN`、上限と法の境界を含みます。
- `inv()` 呼び出し回数の計測で、未使用型と負の添字では前計算なし、同じ型の全操作で初回 1 回だけ、異なる型では独立することを確認しました。
- 動的 modint の法を 1000000007 → 17 → 998244353 → 1000000007 と変更し、それぞれ再構築後の結果とキャッシュの再利用を確認しました。
- 実際の ACL の `modint1000000007`・`modint998244353`・`static_modint<2>`・`static_modint<17>`・`modint` でも値を照合しました。ライブラリ本体・通常の単体テストに ACL の依存は追加していません。
- 既定上限 10000000 の全要素について、64 bit の剰余演算で求めた階乗と照合し、階乗と逆階乗の積も確認しました。GCC・Clang・Clang ASan/UBSan で成功しています。
- 複数 cpp を両方のリンク順で実行し、起動時の初期化・`main` より前の参照・型別キャッシュの共有・逆元計算 1 回を確認しました。
- 公式生成器の全 20 ケースを、追加した検証コード `test/comb_prime_mod.test.cpp` で実行しました。GCC・Clang とも公式出力に一致しました。
- 従来の `test/comb.test.cpp` も GCC・Clang で実行し、23 入力について Python の整数の `math.comb` を使う別実装と照合しました。
- 全対象の GCC・Clang 単体テストと、Clang ASan/UBSan を実行しました。実 ACL の検証も ASan/UBSan で成功しています。
- 既定上限の `-DNDEBUG` と GCC の AtCoder 用 constexpr 上限、単独・二重 include、ドキュメント 2 例、従来版・現行版の測定コードのコンパイルを確認しました。

実行環境は Linux x86_64 / AMD EPYC 9V74、GCC 14.2・Clang 19 です。公式入力・実装のハッシュ、ACL と公式生成器のコミット、検証結果は `results/comb_modint/validation.json` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/comb_modint_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/comb_modint_selection.json
oj-verify run test/comb_prime_mod.test.cpp test/comb.test.cpp
g++ -std=c++23 -O2 -I . -I /path/to/ac-library test/benchmark/comb_modint_atcoder.cpp -o /tmp/comb-modint-acl
/tmp/comb-modint-acl
python3 test/benchmark/comb_simd.py --cpu 2 --trials 9
```
