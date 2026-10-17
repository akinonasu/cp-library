# Power Projection

`fps/power_projection.hpp` に重み付き版と係数版を追加しました。法は 998244353 です。`deg` は返す要素数で、他の FPS 関数と揃えています。`f` と重みの長さは異なっても構いません。

## 実装

定数項を引いた多項式に対して、二変数 Bostan–Mori に相当する合成の転置を使います。既存の `NTT::reversion_projection` に重みを渡せるようにし、作業領域・変換の倍化・AVX2 の積和を共用しました。重みの最初の変換には逆向きの根を使い、合成の転置と同じ順序で処理します。逆関数用の従来の初期化も残しています。

重みの末尾の零係数を除き、定数・定数項を除いた単項式には専用の処理を使います。小入力と出力 8 要素以下では逐次積を使います。定数項が非零なら、最後に階乗付き畳み込みで二項変換します。出力が次数より長い場合にも対応します。外部の modint は必要な係数だけ内部の Montgomery 表現に変換します。

`NDEBUG` と `-Werror` を併用した検証で見つかった、既存の NTT の assert 専用変数には `[[maybe_unused]]` を付けました。

## 参考

2026-10-17 に Library Checker の合成逆元と合成逆元 Large の AC 一覧を実行時間順に取得しました。専用の Power Projection 問題の代わりに、projection を用いた合成逆元で公式ケースを検証します。取得時の一覧、時刻、ソース SHA-256 は `results/power_projection_reference.json` に保存しています。

| 参考 | 取得時の実行時間 | 手法・比較 |
| --- | ---: | --- |
| [通常版 #409399](https://judge.yosupo.jp/submission/409399) | 3 ms | Kinoshita–Li の projection と Lagrange 反転。AVX2、枝刈りした NTT、共有 arena。係数版の projection 部分を同じ入力で測定 |
| [Large #409549](https://judge.yosupo.jp/submission/409549) | 33 ms | 最初の段をまとめ、中間段を枝刈りし、最後の段をまとめる。係数版の projection 部分を同じ入力で測定 |
| [通常版 #409398](https://judge.yosupo.jp/submission/409398)、[#409373](https://judge.yosupo.jp/submission/409373) | 9 / 9 ms | 同系列の projection・冪による逆関数。初期・中間・終端の段の分け方を確認 |
| [Large #409548](https://judge.yosupo.jp/submission/409548)、[#409541](https://judge.yosupo.jp/submission/409541) | 42 / 47 ms | 同系列の AVX2 実装。大きい入力向けの変換と入出力の最適化を確認 |
| [maspypy の power_projection](https://maspypy.github.io/library/poly/power_projection.hpp) | 未測定 | 重み付きの二変数 Bostan–Mori、非零の定数項を二項変換で処理。本実装もこの帰着を採用。API の引数順・出力数の意味は本ライブラリに合わせて変更 |

参考提出の表示時間には合成逆元・入出力も含まれるため、下記の projection 単独の測定とは比較できません。本実装は任意の重み・定数項を扱い、参考提出の projection は定数項 0 の係数版です。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、1 回のウォームアップの後、各 7 回の実行順を seed 固定でシャッフルした中央値を取ります。測定同士・他のテストは同時実行しません。

- generic: `-std=c++2b -O2 -DNDEBUG`
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加
- 参考提出は AVX2 専用のため、追加で `-mavx2` を指定

入力生成・結果の通常剰余への変換・チェックサム・入出力を除き、公開 API の呼び出しを測ります。割り当て・根の準備は含みます。参考提出は最後の `solve`・`main`・preinit だけを除去し、`poly::power_projection` を直接呼びます。参考の arena 破棄は測定外です。取得したソースを変更せずにコンパイル用のコピーを作り、変更前後のハッシュを残します。

各試行は別プロセスで 1 回呼び出します。実行ファイルは Linux の `/tmp` に置き、測定前に共通の AVX2 の積・加算を 4,096 回実行します。Windows 側のファイルシステムに実行ファイルを置いた予備測定では、小入力に実行ファイルの読み込みの影響が出たため、その試行は結果に含めていません。

係数版はすべての出力のチェックサムを参考 2 実装と照合します。重み付き版の小入力は逐次畳み込みとも照合します。環境・フラグ・全試行・ハッシュは `results/power_projection_generic/results.json` と `results/power_projection_native/results.json` に保存します。AtCoder の実機での測定ではありません。

## 結果

係数版は $$[x^{n-1}]f(x)^i$$ を $$0\le i<n$$ について計算します。時間は ms です。

| 係数数 | generic 本実装 / #409399 / #409549 | native 本実装 / #409399 / #409549 |
| ---: | ---: | ---: |
| 8,000 | 1.468 / 1.145 / 0.986 | 1.435 / 1.171 / 0.980 |
| 32,768 | 7.512 / 5.702 / 4.383 | 7.609 / 5.669 / 4.489 |
| 131,072 | 37.007 / 29.050 / 23.131 | 35.854 / 29.157 / 22.881 |
| 262,144 | 86.024 / 66.799 / 53.039 | 84.238 / 66.693 / 52.106 |

重み付き版の係数数は 131,072、重みは乱数です。通常は同じ長さを返し、非零の定数項では 131,089 要素、短い出力では 8 要素を返します。

| 入力 | generic | native |
| --- | ---: | ---: |
| $$f_0=0$$、密な多項式 | 35.805 | 37.196 |
| $$f_0=19$$、密な多項式 | 42.716 | 42.109 |
| $$f_0=0$$、出力 8 要素 | 13.093 | 12.915 |
| $$f(x)=5x^3$$ | 0.368 | 0.391 |

小さい重み付き版を、前の冪に `f` を掛けて次数を切り詰める方法と比較しました。すべての出力が一致しています。

| 係数数・出力数 | generic 本実装 / 逐次積 | native 本実装 / 逐次積 |
| ---: | ---: | ---: |
| 256 | 0.037 / 0.585 | 0.036 / 0.576 |
| 512 | 0.087 / 2.539 | 0.064 / 2.436 |

この測定では参考 #409549 の係数版が最速でした。任意の重みについては参考提出との直接の速度比較を行っていません。

## 検証

- 重み付き版・係数版を愚直計算と比較。空入力、定数、単項式、零重み、非零の定数項、長さの不一致、非 2 冪、出力が入力より長い場合を含む。
- 内部 modint と通常剰余表現の modint、入力の不変性を確認。
- 大きい入力で合成との転置関係、非零の定数項は Taylor shift との関係を確認。
- GCC 13.4・15.2、Clang、ASan / UBSan、単独 include、既存の依存先、公開用 Bundle を検証。
- 新しい関数を Lagrange 反転に使う検証プログラムで Library Checker の合成逆元 Large 28 ケースを GCC・Clang ともに AC。既存の合成逆元 23 ケース・合成 27 ケースも両方で AC。
- 依存先の cyclic / ordered convolution が使う yukicoder No.1307 はローカルでの公式ケース取得に認証が必要です。対応する単体テストを確認し、公式ケースは認証済み CI で検証します。
- CI の依存先選択・API 表・公開ページを確認。Apple Clang の検証は macOS CI で実施。

## 再現

```sh
python3 test/benchmark/power_projection_reference.py --cache .verify-helper/cache/power-projection-reference --output test/benchmark/results/power_projection_reference.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/power_projection_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/power_projection_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/power_projection.test.cpp test/fps_compositional_inverse.test.cpp test/fps_compose.test.cpp
python3 test/benchmark/power_projection.py --reference-cache .verify-helper/cache/power-projection-reference --output test/benchmark/results/power_projection_generic
python3 test/benchmark/power_projection.py --native --reference-cache .verify-helper/cache/power-projection-reference --output test/benchmark/results/power_projection_native
```
