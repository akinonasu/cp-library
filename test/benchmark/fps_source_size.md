# FPS のソースサイズ削減

基準は `3d7dfe2caa96a00c8e78246b26a29bcafa41926c`。速度改善済みの演算・dispatch・メモリ管理を維持し、インライン asm の文字列の記述だけを短縮した。

各命令を文字列化する `components/fps_asm.hpp` と、各 asm ブロックの短い引数名を組み合わせる。ブロック用マクロは使用直後に `undef` する。最終的な asm の文字列、`volatile`、入出力・clobber・ラベル・alignment は変えず、元ファイルの改行数も保った。

## サイズ

`oj-bundle -I . test/対象.test.cpp` を各 snapshot のルートから実行し、UTF-8 のバイト数を測った。上限は 512 KiB = 524,288 bytes。影響する提出コード24件と、各FPS公開ヘッダー＋FPS用I/Oの10件がすべて上限未満。最大は power projection の 641,120 → 477,407 bytes（約25.5%減）、残り46,881 bytes。任意の追加ライブラリや利用者のコードを含むサイズを保証するものではないため、提出時は必要なヘッダーだけを結合する。

| 提出コード | 変更前 bytes | 変更後 bytes |
| --- | ---: | ---: |
| `test/convolution.test.cpp` | 560509 | 408939 |
| `test/cyclic_convolution.test.cpp` | 532066 | 380496 |
| `test/fps_bostan_mori.test.cpp` | 552540 | 400970 |
| `test/fps_compose.test.cpp` | 639386 | 475673 |
| `test/fps_compose_large.test.cpp` | 639404 | 475691 |
| `test/fps_compositional_inverse.test.cpp` | 639524 | 475811 |
| `test/fps_compositional_inverse_large.test.cpp` | 639542 | 475829 |
| `test/fps_divmod.test.cpp` | 546706 | 395136 |
| `test/fps_exp.test.cpp` | 546651 | 395081 |
| `test/fps_exp_sparse.test.cpp` | 546719 | 395149 |
| `test/fps_interpolate.test.cpp` | 623582 | 472012 |
| `test/fps_inv.test.cpp` | 546651 | 395081 |
| `test/fps_inv_sparse.test.cpp` | 546719 | 395149 |
| `test/fps_linear_recurrence.test.cpp` | 554004 | 402434 |
| `test/fps_log.test.cpp` | 546651 | 395081 |
| `test/fps_pow.test.cpp` | 546584 | 395014 |
| `test/fps_sqrt.test.cpp` | 546598 | 395028 |
| `test/fps_taylor_shift.test.cpp` | 564114 | 412544 |
| `test/multiplicative_convolution.test.cpp` | 537442 | 385872 |
| `test/multipoint_eval.test.cpp` | 623319 | 471749 |
| `test/multivariate_convolution.test.cpp` | 533892 | 382322 |
| `test/ordered_convolution.test.cpp` | 532518 | 380948 |
| `test/polynomial_inv_mod.test.cpp` | 584052 | 432482 |
| `test/power_projection.test.cpp` | 641120 | 477407 |

## 速度・安全性の維持

Linux x86_64、GCC14.2。両版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でコンパイルした。比較用の `-ffile-prefix-map` によるパス正規化だけを両版に適用し、ファイル名も揃えた。

GCC14の全24提出コード、GCC13の4件、Clang19の2件、GCC14で `-U__linux__` を指定した2件、合計32組で実行ファイル全体のSHA-256が一致した。Clangと非Linuxマクロの確認はLinux上でのfallback比較であり、Mac実機の測定ではない。さらに4変更ヘッダーに含まれる25個のasm文字列とvolatile属性も完全一致した。

演算や命令を置換した変更ではない。比較した実行ファイルには実行命令・配置・データの差がないため、速度のばらつきを含む再ベンチマークや、同じバイナリの成功済み全公式ケースの再実行は行わなかった。公開APIと受け付ける入力の範囲も変えていない。直前の速度比較は [FPS追加高速化4](fps_round4.md) を参照。

ドキュメント278ページの生成・153件のメタデータ検証、影響する32ヘッダーのドキュメント結合コンパイルとGCC14・Clang19での単体include検証も成功した。既存のYuki DijkstraのBoostメタデータ警告は残るが、検証は成功。記録は [docs.json](results/fps_source_size/docs.json)、最終整合性確認は [audit.json](results/fps_source_size/audit.json)。

## 再現

[原票](results/fps_source_size/result.json)には全source、compiler、bundle、実行ファイルのSHA-256と全コマンドを保存した。[実行したscript](results/fps_source_size/executed.py)は検証時の版をそのまま保存。[可搬版helper](fps_source_size.py)はパス設定を引数化したもので、比較処理の関数ASTが実行版と同一であること、`--prepare-only`で全34サイズ・bundle SHAを再現することを確認した。

```sh
python3 test/benchmark/fps_source_size.py \
  --work /workspace/fps-size-replay \
  --output /workspace/fps-size-replay-results \
  --bundle /tmp/fps-venv/bin/oj-bundle \
  --bundle-python /tmp/fps-venv/bin/python
```

`--prepare-only` はサイズ確認だけを行う。`--gcc13` と `--clang` に実行ファイルを指定すると追加の互換性比較も行う。検証対象のrevisionへcheckoutし、新しいworkディレクトリを使う。

最終的な提出方法は従来どおり `oj-bundle a.cpp > submit.cpp`。`wc -c submit.cpp` で利用者のコードを含む総サイズを確認する。
