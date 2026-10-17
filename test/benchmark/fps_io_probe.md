# FPS の入出力の分解

[畳み込み #409465](https://judge.yosupo.jp/submission/409465) のソースには、通常の整数入力のほか、9 桁の係数と 1 文字の区切りを 8 個ずつ読む AVX2 処理、1 桁なら 16 個ずつ読む処理があります。ブロックの区切り位置を検査し、合わなくなった場所から通常の bulk 入力に戻ります。出力は各剰余を 9 桁の欄と区切りの 10 bytes に揃え、AVX2 でまとめて変換します。

このプローブのための `FastInput`・`FastOutput`・`test/fps_io.hpp` の変更はありません。以下は計算を一切行わず、入出力の処理だけを比較します。参考ソースから入出力、固定幅の読み込み、`fields` の出力だけを抜き出します。畳み込みのコード、起動前の `solve`、`_exit` は含めません。

## 条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native -ftrivial-auto-var-init=zero`。コンパイル単位・公式ケース単位で共通の排他ロックを取り、他の測定と同時に実行しません。実行ファイル・入力・出力・ログは Linux の `/tmp` に置きます。1 回のウォームアップ後、実行順を seed 409465 でシャッフルして各 7 回を測ります。

参考の `io::read_bulk` は `__znver3__` が定義された場合だけ転置パーサを選びます。比較では `IO_BULK32_TRANSPOSE=0` と `=1` を明示して別々にコンパイルします。転置版の測定もこの Intel CPU 上での結果で、Zen 3 の実測ではありません。

| 名前 | 入力の処理 |
| --- | --- |
| `own_raw` | `FastInput::read(unsigned)` |
| `own_mint` | 同じ入力後に `mint::raw` で内部表現へ変換 |
| `ref_scalar` | 参考の `Reader::read<unsigned>()` を 1 個ずつ呼ぶ |
| `ref_bulk` | 参考の AVX2 bulk、転置パーサは無効 |
| `ref_transposed` | 転置パーサを明示的に有効化した bulk |
| `ref_fixed` | 参考の固定幅の検査・入力と、その失敗後の非転置 bulk |

各 Reader の作成・破棄、出力先ベクトルの確保・零初期化は入力の計測に含みます。ベクトルの破棄とチェックサムは計測外です。`own_mint` の内部表現は計測後に通常の剰余へ戻し、全入力のチェックサムを全方式で照合します。元提出と同じく、bulk 入力は 2 本の配列ごとに呼びます。

| 名前 | 出力の処理 |
| --- | --- |
| `own_raw` | `FastOutput::write(unsigned)` と空白 |
| `own_mint` | `mint::val()`、同じ `FastOutput` と空白 |
| `ref_scalar` | 参考の `Writer::write(unsigned, ' ')` |
| `ref_fields` | 参考の AVX2 固定幅出力 |

公式の正答から作った binary の配列を計測前に読み込みます。`own_mint` の配列の作成・破棄は計測外、`val()` は計測内です。Writer の作成・flush・破棄を含みます。出力のすべての整数を公式の正答と照合します。固定幅出力には先頭の空白が付くため、byte 列の一致は要求しません。

## 読み方

生データ、入力の桁数の分布、参考ソース・抜粋・プローブの SHA-256 は [results.json](results/fps_io_probe/results.json) に保存します。

入力は 2 配列を合わせて 1,048,576 係数、出力は 1,048,575 係数です。時間は ms、各 7 回の中央値です。

| 入力ケース | own raw | own Mint | ref scalar | ref bulk | ref fixed | ref transposed |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `all_same_00` | 2.701 | 3.104 | 3.130 | 1.755 | 1.070 | 1.879 |
| `all_same_01` | 7.015 | 7.827 | 3.156 | 1.873 | 1.655 | 1.966 |
| `max_random_00` | 7.547 | 8.246 | 3.348 | 1.918 | 1.867 | 1.911 |

| 出力ケース | own raw | own Mint | ref scalar | ref fields |
| --- | ---: | ---: | ---: | ---: |
| `all_same_00` | 6.223 | 6.551 | 3.687 | 2.144 |
| `all_same_01` | 8.616 | 9.106 | 4.239 | 2.097 |
| `max_random_00` | 8.860 | 9.613 | 4.201 | 2.071 |

`all_same_01` では単独の入力・出力の合計が own Mint 16.933 ms、ref fixed / fields 3.752 ms でした。通常の汎用 FastIO を使っても、上位提出の入出力との差は残っています。この観測では Mint の表現変換だけで説明できず、入力の AVX2 bulk 化、整数の整形、固定幅の bulk 出力それぞれに差があります。転置入力を強制しても、この Intel CPU では非転置 bulk より速くなるとは限りません。

これは Reader の初期化・ページフォールト・配列の確保や、Writer の flush も含めたステージの比較です。純粋な 10 進変換だけの速度ではありません。また元の提出は畳み込みの領域を出力に再利用し、起動前に実行して `_exit` するため、全体時間からこの時間を引いて厳密な NTT の時間を求めることはできません。

固定幅出力の空白の付け方や、非負・指定範囲の整数だけを読む前提は、一般の `cin`・`cout` の意味論と同じではありません。プローブの差を、そのまま通常の入出力ライブラリへ適用できる改善量とは解釈しません。

```sh
python3 test/benchmark/fps_io_probe.py --output test/benchmark/results/fps_io_probe --trials 7
```
