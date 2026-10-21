# FPS 提出コードの追加圧縮

この資料は初期版の圧縮ツールの記録です。現在の全ライブラリ向けの生成方法・サイズ・検証は [256 KiB 制限への対応](source_size_256k.md) を参照してください。

前回提示した Kth term of Linearly Recurrent Sequence の単一ファイルは 402,426 → **248,530 bytes**。256 KiB = 262,144 bytes より 13,614 bytes 小さい。C++23・UTF-8のファイルとして提出する。

`fps` の通常ヘッダーを変更せず、`oj-bundle` の後で [tools/compact_fps.py](../../tools/compact_fps.py) を実行する。公開関数・型・演算・実行時CPU判定・他環境のfallbackを残す。未使用関数を削る方法ではなく、通常どおり全FPS機能の記述を含む。

## 方法

```sh
oj-bundle -I . a.cpp > bundled.cpp
python3 tools/compact_fps.py bundled.cpp --limit 262144 -o submit.cpp
wc -c submit.cpp
```

`--limit` はUTF-8バイト数の上限を指定し、その値以上なら出力前に失敗する。既存の出力ファイルも上書きしない。上限指定なしでも利用できる。依存するPythonモジュールは標準ライブラリのみ。`oj-bundle` は従来どおり別途インストールする。

通常のC++行に繰り返し現れるトークン列を、短いUTF-8識別子のマクロに置き換える。候補名は元ソースに含まれない文字から選び、生成したマクロは末尾で `undef` する。FPS内部の `CP_FPS_ASM_` マクロ名も提出ファイル内だけで短縮する。文字列化を伴うマクロの引数、asm、プリプロセッサ指令、コメント、raw文字列、行番号依存箇所は保護する。トークンが結合しないよう必要な空白を残し、元コードの改行数と `#line` を維持する。

短縮はソースの記述だけに作用し、実行時の展開・解凍・追加メモリはない。UTF-8のまま保存・提出する。

## サイズ

下表は同じdriverを `oj-bundle -I .` した結果。元のKth term bundleの402,434 bytesと前回提示ファイル402,426 bytesとの差は空行のタブ8 bytes。24件中16件が256 KiB未満。合成・多点評価・補間などの追加機能は超える場合があるため、任意の組合せや利用者のコードまで256 KiB未満になるとはしない。

| 提出コード | 通常 bytes | 圧縮 bytes | 256 KiB未満 |
| --- | ---: | ---: | :---: |
| `test/convolution.test.cpp` | 408,939 | 251,324 | ○ |
| `test/cyclic_convolution.test.cpp` | 380,496 | 235,187 | ○ |
| `test/fps_bostan_mori.test.cpp` | 400,970 | 247,624 | ○ |
| `test/fps_compose.test.cpp` | 475,673 | 285,443 | — |
| `test/fps_compose_large.test.cpp` | 475,691 | 285,461 | — |
| `test/fps_compositional_inverse.test.cpp` | 475,811 | 285,609 | — |
| `test/fps_compositional_inverse_large.test.cpp` | 475,829 | 285,627 | — |
| `test/fps_divmod.test.cpp` | 395,136 | 244,258 | ○ |
| `test/fps_exp.test.cpp` | 395,081 | 244,264 | ○ |
| `test/fps_exp_sparse.test.cpp` | 395,149 | 244,246 | ○ |
| `test/fps_interpolate.test.cpp` | 472,012 | 279,429 | — |
| `test/fps_inv.test.cpp` | 395,081 | 244,263 | ○ |
| `test/fps_inv_sparse.test.cpp` | 395,149 | 244,244 | ○ |
| `test/fps_linear_recurrence.test.cpp` | 402,434 | 248,530 | ○ |
| `test/fps_log.test.cpp` | 395,081 | 244,264 | ○ |
| `test/fps_pow.test.cpp` | 395,014 | 244,184 | ○ |
| `test/fps_sqrt.test.cpp` | 395,028 | 244,192 | ○ |
| `test/fps_taylor_shift.test.cpp` | 412,544 | 255,542 | ○ |
| `test/multiplicative_convolution.test.cpp` | 385,970 | 238,668 | ○ |
| `test/multipoint_eval.test.cpp` | 471,749 | 279,406 | — |
| `test/multivariate_convolution.test.cpp` | 382,322 | 236,204 | ○ |
| `test/ordered_convolution.test.cpp` | 380,948 | 235,448 | ○ |
| `test/polynomial_inv_mod.test.cpp` | 432,482 | 263,056 | — |
| `test/power_projection.test.cpp` | 477,407 | 286,336 | — |

## 速度・正答・互換性

初期比較の基準revisionは `8fff4f8`。Linux x86_64、GCC14.2、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。両版のパスだけを同じ値へ `-ffile-prefix-map` で正規化し、ソースのファイル名も揃えた。

GCC14で全24提出コード、GCC13でKth termと合成の2件、Clang19で同じ2件、GCC14で `-U__linux__` を加えた同じ2件、字句処理の回帰例1件、合計31組で**実行ファイル全体のSHA-256が一致**した。さらにプリプロセス後のトークン列も全組で一致した。Mac実機の測定は行っておらず、Clangと非Linuxマクロの確認もLinux上の比較である。[原票](results/fps_compact/result.json)、[検証スクリプト](fps_compact_check.py)。

回帰例は、改行を挟む文字列化マクロ・コメント・raw文字列・複数行のassert・`__LINE__`を含む。実行結果も一致した。`#line`のない入力を拒否すること、CLIのサイズ超過時に既存出力を保持することも確認した。

最終的な提出ファイル自体も元と同じ条件でコンパイルし、公式20ケースを通過済みの実行ファイルとバイト単位で一致した。コンパイラ、checker、入力・正解ファイルのSHA-256も過去の証拠と一致することを確認し、その20ケースの実行結果を再利用した。[提出ファイルの記録](results/kth_term_submission.json)、[元の記録](results/fps_compact/previous_submission.json)。速度・正答の確認済みバイナリ自体が変わらないため、同じケースの再実行やノイズを含む速度の再測定はしない。judgeへの提出・fastest順位の確認は行っていない。

並行して上流へ追加されたライブラリ整理を `cdbcfda` まで取り込んだ。23件のbundle SHA-256はそのまま一致。`multiplicative_convolution` は依存ヘッダーの移動で `#line` が変わったため、新しいbundleでも実行ファイルとプリプロセストークン列の一致を追加確認した。上表はこの更新を含む。[最終統合確認](results/fps_compact/integration.json)。

ドキュメント284ソースの生成と157ライブラリのメタデータ検証も成功した。変更したヘッダーはないため、成功済みの通常ヘッダー単体検証は繰り返していない。既存のYuki DijkstraのBoost依存メタデータ警告は残る。[ドキュメントの記録](results/fps_compact/docs.json)。

## 再現

```sh
python3 test/benchmark/fps_compact_check.py \
  --work /workspace/fps-compact-replay \
  --output /workspace/fps-compact-replay-results \
  --bundle /tmp/fps-venv/bin/oj-bundle \
  --gcc /usr/bin/g++ \
  --gcc13 /tmp/fps-gcc13/toolchain/usr/bin/g++-13 \
  --clang /tmp/fps-clang/toolchain/usr/bin/clang++-19
```

`--work`には未作成のディレクトリを指定する。`--prepare-only`はサイズ確認までで終了する。追加コンパイラがない場合は `--gcc13`・`--clang` を省略できる。各回のsource・compiler・bundle・実行ファイル・ログのSHA-256とコマンドを原票に保存する。上流の移動を含むrevisionでは、旧snapshotの全source hashではなく新しい測定として記録される。
