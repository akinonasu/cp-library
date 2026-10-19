# 二項係数 prefix sum の offline 多項式評価

offline 版 `binomial_prefix_sum(queries)` を、記事の多項式分割統治に統一した。Mo 法との切り替えは行わない。オンライン版 `BinomialPrefixSum` のクラス本体と公開 API は維持する。

## 参考資料

- [zhoukangyang「组合数前缀和」](https://www.cnblogs.com/zkyJuruo/p/16995141.html)、2022-12-20。取得情報は [article_source.json](results/binomial_prefix_sum_polynomial/article_source.json)。記事には実装コードがなく、記載された分割統治と剰余木の考え方から実装した。
- 直接対応する Library Checker 問題は確認できなかった。同じ prefix sum を使う [yukicoder No.2206](https://yukicoder.me/problems/no/2206) の AC 一覧を再取得した。[1110156](https://yukicoder.me/submissions/1110156) は 307 ms、[1110155](https://yukicoder.me/submissions/1110155) は 308 ms、[1110477](https://yukicoder.me/submissions/1110477) は 320 ms で、いずれも Mo 法だった。取得記録とソースのハッシュは [reference.json](results/binomial_prefix_sum_polynomial/reference.json)。公開ジャッジ時間と今回のローカル時間は直接比較しない。

記事の和は右端を含むが、公開 API は従来どおり $$S(n,k)=\sum_{i=0}^{k-1}\binom ni$$ とする。記事の $$n\leq10^{18}$$ への対応は追加せず、従来の非負 `int`、法未満という入力条件で比較した。

## アルゴリズム

対称性で $$m=\min(k,n+1-k)$$ に折り返し、

$$P_{a,b}(x)=\prod_{j=a}^{b-1}(x-j),\qquad H_{a,b}(x)=\sum_{i=a}^{b-1}\frac{1}{i!}\prod_{j=a}^{i-1}(x-j)$$

を使う。$$H_{0,m}(n)=S(n,m)$$ であり、境界 $$c$$ で

$$P_{a,b}=P_{a,c}P_{c,b},\qquad H_{a,b}=H_{a,c}+P_{a,c}H_{c,b}$$

と合成できる。問い合わせを $$m$$ 順に並べ、子へ渡す prefix と降階乗積を、その子の問い合わせに対応する $$\prod(x-n)$$ で剰余化する。部分問題ごとに独立した多点評価を繰り返さず、剰余を木に沿って伝播させる。

同じ $$(m,n)$$ はまとめる。根で不要な基底と剰余多項式、右端の経路で不要な基底は構築しない。葉は最大 16 点とし、Horner 法と残りの短い降階乗和を直接評価する。空・全体・端・中央の処理と反転は従来どおり行う。

既定の `mint` では既存の FPS 乗算・多項式剰余を使う。その他の modint と NTT の上限を超える演算は Karatsuba 乗算と Newton 法で処理する。`val()`・整数変換・単項マイナス・等値比較を追加要件にせず、非 const の `inv()` でも利用できる。いずれも同じ多項式の分割統治を使う。

$$N=\max n$$、入力数を $$Q$$、直接処理後の重複を除いた問い合わせ数を $$U$$、折り返した $$m$$ の最大値を $$M$$、$$D=M+U$$ とする。既定の `mint` で NTT を使える大きさなら $$O(N+Q\log Q+D\log^2(D+1))$$ 時間、$$O(N+Q+D\log(D+1))$$ 空間。Karatsuba 演算を使う場合の時間の上界は $$O(N+Q\log Q+D^{\log_2 3}\log(D+1))$$。前計算は逆元計算 1 回を含む。

## N=Q=200000 の比較

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0。CPU 2 に固定し、他のコンパイル・テストと測定を同時に実行していない。コンパイルは `-std=c++23 -O2 -DNDEBUG -march=haswell -ftrivial-auto-var-init=zero`。FPS と新しい内部部品は既存の方針に合わせて GCC/x86 で O3 を指定する。

旧 Mo 法は `444ac7f` の関数本体を測定用に保存し、スクリプトが元の関数との一致を検査する。階乗・modint・Mo 順の部品も元の commit と同一であることを確認する。両方式を同じ実行ファイルに入れ、FastInput/FastOutput を共通にする。

各方式にウォームアップ 1 回、測定 21 回。各ラウンドで実行順をシャッフルする。下表は API 呼び出しの実時間の中央値、単位は ms。階乗の前計算、フィルタ、並べ替え、計算、破棄を含み、入力生成と入出力は含めない。プロセス起動と共通入出力を含む時間も生データに保存した。全ウォームアップ・全試行で出力全体が一致した。

全分布で最大 $$n$$ と問い合わせ数を 200000 とするため、最後の問い合わせの $$n$$ は 200000 にする。`small_rows` も最後の 1 件はこの値になる。固定 $$n$$ は利用想定から除外し、測定していない。

| 分布 | 生成条件 | 旧 Mo 法 | 新多項式 | 新 / 旧 |
| --- | --- | ---: | ---: | ---: |
| `uniform` | n を一様、k を 0..n+1 から一様 | 296.858 | 259.077 | 0.873 |
| `rectangle` | n を N/2..N、k を 0..N/2 から一様 | 272.044 | 272.470 | 1.002 |
| `independent` | n と k を独立に一様 | 197.464 | 152.699 | 0.773 |
| `narrow_n` | n を N-2047..N、k を 0..n+1 から一様 | 69.485 | 275.391 | 3.963 |
| `edges` | k が 0, 1, 2, n-1, n, n+1 | 10.625 | 16.242 | 1.529 |
| `duplicates` | n が異なる 256 組から反復 | 71.778 | 32.600 | 0.454 |
| `small_rows` | n を 0..256、k を 0..n+1 から一様 | 28.612 | 25.796 | 0.902 |

`uniform` では 12.7%、`independent` では 22.7% の時間短縮だった。`rectangle` の差は小さい。$$n$$ の値域が狭い `narrow_n` と端付近が多い `edges` は旧 Mo 法が速かった。入力分布による切り替えは設けず、指定どおり offline 版を提案手法に統一した。AtCoder・yukicoder 実機でのこの版の実行時間は未測定。

生データ、各入力・出力、ソース依存先、コンパイラ、実行ファイルのハッシュは [result.json](results/binomial_prefix_sum_polynomial/result.json)。seed と全試行の測定値を含み、測定前後で依存ファイルと実行ファイルの不変性を確認する。

```sh
python3 test/benchmark/binomial_prefix_sum_polynomial.py --work /tmp/binomial-prefix-200k --output test/benchmark/results/binomial_prefix_sum_polynomial --trials 21 --cpu 2
```

`--work` は存在しないディレクトリを指定する。入力はスクリプトから再生成できる。

## 検証

- Pascal の三角形との全件比較、法 2・3・17・101・1000000007、空入力、INT64_MIN/MAX、反転、入力の不変性。
- 葉の境界 1・16・17・32・33・64・65・257 点、同じ評価点、少数クエリと大きい次数、重複クエリ。
- $$N=Q=200000$$ で旧 Mo 法と全件一致。既定と異なる表現で、等値比較などを持たない modint の大きい剰余計算も旧 Mo 法と一致。
- GCC 14.2・Clang 19.1.7 の単体テスト、独立 include、ASan/UBSan、影響を受ける結合コードの構文チェック。
- GCC・Clang のオンライン・offline 検証プログラムで、整数を全列挙した 136 ケースと yukicoder 公開サンプル。

対象と結果は [selection.json](results/binomial_prefix_sum_polynomial/selection.json)・[validation.json](results/binomial_prefix_sum_polynomial/validation.json)、ログは同じディレクトリに保存する。オンライン版のクラス本体が旧 commit と同一であることも確認した。ローカル環境には非公開の全ケース取得用トークンがなく、全ケースと macOS は push 後の GitHub Actions で確認する。
