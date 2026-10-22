# 全ライブラリの提出コードを 256 KiB 未満に生成する

2026-10-21。基準は `cc11463`。C++ のライブラリ本体・公開 API・高速カーネルを変更せず、提出コードの生成とサイトの Bundle を更新しました。通常の `oj-bundle` は引き続き元の大きいコードを返します。256 KiB 制限向けには次のコマンド、またはサイトの Bundle を使います。

```sh
python3 tools/bundle.py a.cpp -I . -o submit.cpp
```

既定値は 256 KiB = 262,144 bytes 未満です。小さい結合結果はそのまま出力し、上限以上の場合だけトークン列を内部マクロへまとめます。最終結果が上限以上なら出力前にエラーとし、既存の出力先も更新しません。出力は UTF-8 です。

## 検査した範囲

- 公開ヘッダー 120 個、内部部品 86 個、計 206 ヘッダー。各ヘッダー単独と Fast I/O 込みの 412 通り。
- `test/*.test.cpp` の全 181 提出例。
- 計 593 通りがすべて上限未満で、圧縮が必要だったのは 66 通りです。
- 最大のヘッダー＋Fast I/O は 248,788 bytes、最大の完成した提出例は 261,222 bytes です。

任意のライブラリの組み合わせや、任意の長さの解法まで固定サイズに収まるとはしません。最終ソース全体を生成コマンドへ渡すことで、上限以上のコードを誤って出力しないようにしています。

## サイズ

`oj-bundle` による依存先展開後、通常の結合結果と生成結果の UTF-8 バイト数を測りました。ヘッダーの行は Fast I/O と空の main を含みます。命令や関数を削除する最適化は行っていません。

| 対象 | 通常の結合 bytes | 生成後 bytes | 262,144 bytes までの余裕 |
| --- | ---: | ---: | ---: |
| `fps/fps.hpp` | 372,838 | 210,855 | 51,289 |
| `fps/compose.hpp` | 453,372 | 248,107 | 14,037 |
| `fps/compositional_inverse.hpp` | 453,498 | 248,267 | 13,877 |
| `fps/power_projection.hpp` | 454,715 | 248,788 | 13,356 |
| `fps/multipoint_eval.hpp` | 449,421 | 244,489 | 17,655 |
| `fps/interpolate.hpp` | 449,705 | 244,524 | 17,620 |
| `fps/polynomial_gcd.hpp` | 410,023 | 229,182 | 32,962 |
| `convolution/convolution.hpp` | 386,630 | 217,769 | 44,375 |
| `math/combinatorics/binomial_prefix_sum.hpp` | 387,256 | 219,134 | 43,010 |
| `test/fps_compose_large.test.cpp` | 475,751 | 260,356 | 1,788 |
| `test/fps_interpolate.test.cpp` | 472,072 | 256,836 | 5,308 |
| `test/power_projection.test.cpp` | 477,467 | 261,222 | 922 |

[全 593 通りの原票](results/source_size_256k/result.json)には、ソース・生成物の SHA-256、コマンド、コンパイラ条件、比較結果を記録しています。`#line` に入る作業ディレクトリなどによって、別環境ではバイト数が少し変わります。

## 変更した生成処理

既存の `compact_fps.py` を基に、文字列化マクロを保護しながら通常のコード領域をまとめ、共通するトークン列を抽出します。辞書の導入でかえって大きくなった参照は展開し直します。asm の同じ clobber 一覧を共有し、継続行や長い空行列は論理行番号を保って短くします。raw 文字列やコメント内部は変更せず、条件付きの `#line` や間接的な `__LINE__` 参照では必要に応じて行の圧縮を避けます。

asm のオペランド・clobber・命令、プリプロセッサ条件、全機能、CPU 判定、他環境の分岐を残します。生成に使う内部マクロは最後に解除します。`tools/bundle.py` は結合と上限検査を一度に行う入口です。

## 動作・速度の維持

66 通りすべての圧縮結果について、GCC で元コードとプリプロセス後の全トークンを照合しました。さらに合成・補間・power projection・線形漸化式・二項係数 prefix sum の 5 提出例を Clang および GCC の `-U__linux__` で照合し、計 76 組が一致しました。

圧縮された提出例 26 件の GCC、上記 5 件ずつの Clang・非 Linux マクロ指定について、計 36 組で実行ファイル全体の SHA-256 が一致しました。条件は GCC 14.2 / Clang 19、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` です。パスだけを `-ffile-prefix-map` で同じ値に正規化し、同じファイル名で比較しました。

GCC の通常の x86-64 ターゲットも [別途比較](results/source_size_256k/generic.json) しています。現行ツールで圧縮結果を再生成し、検証済み生成物とのバイト単位の一致も [再生成記録](results/source_size_256k/replay.json) で確認しています。原票が使用したツールの原本は [measured_compact.py](results/source_size_256k/measured_compact.py) に保存しています。

入力に対する処理・命令・データ配置が同じ実行ファイルなので、速度を保つためのカーネル削除や低速経路への差し替えはありません。同一バイナリの既存の正答テストや、ノイズを含む実行時間の再測定は繰り返していません。Clang・非 Linux マクロでの確認は Linux 上の比較で、Mac 実機の測定ではありません。

## サイト・回帰テスト

サイトの Bundle も同じ上限判定を使い、必要な場合だけ圧縮します。生成した 206 ヘッダーのコードは通常の GCC 設定でコンパイルを通過しました。URL コメントを付けて Copy する場合も上限未満であることを検査します。今後ツールが変わった際も、結合コードの検証キャッシュが更新されるようにしています。

文字列化マクロ、raw 文字列内の継続行・空行・コードに見える文字列、条件分岐、条件付きの `#line`、間接的な `__LINE__`、`source_location`、上限エラー時の出力先維持を回帰テストで確認しています。サイズ・コンパイラ・最終ソースの整合性は [最終監査](results/source_size_256k/audit.json) に記録しています。

## 再現

```sh
python3 test/benchmark/source_size_budget.py \
  --work /tmp/source-size-256k \
  --output /tmp/source-size-256k.json \
  --bundle /path/to/oj-bundle --cxx g++ --clang clang++ --jobs 3
python3 -m unittest discover -s .verify-helper -p bundle_test.py
```

`--work` には存在しないディレクトリを指定します。検査用の通常コード・圧縮コード・プリプロセス結果・実行ファイルを保存するため、十分な空き領域を用意します。
