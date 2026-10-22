# 軽量な二項係数ライブラリの検証

2026-10-22。通常版に依存せず使える `light_combinatorics` 名前空間を追加した。通常版のヘッダーは変更せず、軽量版と同時に include できる。新しいヘッダーは通常の ASCII C++ で、圧縮マクロや bundle 後の加工を使わない。

- `comb_light.hpp` は型ごとに階乗・逆階乗を遅延生成する。上限の既定値は 1e7、負の添字は 0。動的 modint の法変更時には表を作り直す。
- `binomial_prefix_sum_incremental_light.hpp` は元と同じ差分更新を使う。階乗と逆階乗は通常のループで構築し、FPS への依存はない。
- `binomial_prefix_sum_light.hpp` は元と同じ平方分割のオンライン版と、降階乗基底の多項式を使うオフライン版を提供する。既定の `mint` の多項式演算だけを `light_fps` に置き換え、他の modint の Karatsuba 法は保った。

オフライン版の手法と参照資料は[元の比較記録](binomial_prefix_sum_polynomial.md)を参照。新しいアルゴリズムへの変更は行っていない。

## コード長

`oj-bundle -I . source.cpp` の無加工の出力に、軽量版サイトの正規ページ URL をコメントで付けて計測した。全プラットフォーム分岐を残している。

| ヘッダー | 空の main と URL | さらに FastInput/Output を include |
| --- | ---: | ---: |
| `math/combinatorics/comb_light.hpp` | 6,672 B | 12,908 B |
| `math/combinatorics/binomial_prefix_sum_incremental_light.hpp` | 8,373 B | 14,625 B |
| `math/combinatorics/binomial_prefix_sum_light.hpp` | 50,457 B | 56,659 B |

最大のオフライン版と基本 I/O を組み合わせても 64 KiB まで 8,877 B 残る。利用者の main や他のライブラリは、この残りに加わる。テスト用 driver の本体を含む場合も、5 種類すべて 64 KiB 未満だった。

## 正当性

GCC 14.2 と Clang 19、`-std=c++23 -O2 -Wall -Wextra -Werror` を使用した。コンパイル・実行中は FPS と Dynamic Wavelet Matrix の測定ロックを shared で取得した。

- unit は、Pascal の三角形による照合、法 2・3・17・101・1e9+7、`mint`、符号付き 64 bit の境界、オンライン表の作り直し、差分更新のコピー、通常版との同時利用、8,292 件の大きなランダム問い合わせの一致を確認した。
- 階乗では `main` より前のアクセス、負の添字、型ごとの前計算 1 回、無効な問い合わせでは前計算しないこと、動的法の変更による再構築を確認した。
- 同じ unit を Clang の ASan/UBSan でも実行した。
- Library Checker `binomial_coefficient_prime_mod` の公式 20 ケースを両コンパイラで検証した。
- Yukicoder 2206 のオンライン版・オフライン版について、各々 `n <= 16` の全部分集合から求めた 136 通りの答えと公開サンプルを両コンパイラで照合した。
- ARC190 B の差分更新版は、直接二項係数を足した oracle に対し 30 入力・各 100 問い合わせを両コンパイラで照合した。
- Yukicoder 3172 の driver は `n=0,1,2,9,200` を整数の二項係数と照合した。

コマンド、各 bundle のバイト数、コンパイラ情報、公式入力・出力と対象ソースの SHA-256 は [combinatorics.json](results/fps_light_natural/combinatorics.json) に保存した。unit は次の形で再実行できる。

```sh
g++ -std=c++23 -O2 -Wall -Wextra -Werror -I. test/unit/combinatorics_light.cpp -o /tmp/combinatorics_light
/tmp/combinatorics_light
```
