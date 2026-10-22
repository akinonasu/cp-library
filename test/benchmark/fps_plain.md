# FPS を通常展開だけで 256 KiB 未満にする

この資料は 2026-10-21 の実装についての測定記録です。軽量版はその後、特殊文字マクロを使わない通常の C++ へ変更し、派生演算も追加しました。現在の軽量版の編集方法・サイズ・併用条件は [最新の資料](fps_light_natural.md) を参照してください。

2026-10-22 JST。基準は `c6bee6c`。短縮済みの配布ヘッダーをリポジトリへ保存し、通常の `oj-bundle` で結合するだけで提出できるようにしました。`tools/bundle.py` は削除しました。サイトの Bundle も通常展開だけを行います。

```sh
oj-bundle -I . a.cpp > submit.cpp
wc -c submit.cpp
```

## 通常展開のサイズ

すべて先頭の参照 URL を含む UTF-8 バイト数です。前後とも通常の `oj-bundle` の出力で、展開後の追加圧縮はありません。通常版の積・逆数・対数・指数・平方根・冪・除算・合成・合成逆元・power projection・多点評価・補間・Taylor shift・Bostan–Mori・線形漸化式・GCD・拡張 GCD・多項式の逆元を含みます。

| 使用例 | 変更前 bytes | 変更後 bytes | 上限までの余裕 |
| --- | ---: | ---: | ---: |
| 通常版の全 API 使用例＋高速 I/O | 495,915 | **243,451** | 18,693 |
| 通常版・軽量版の全 12 ヘッダー＋全 API 使用例＋高速 I/O | 527,626 | **261,795** | 349 |
| 軽量版の全ヘッダー＋単体テスト使用例＋高速 I/O | 54,062 | **39,353** | 26,183 |

通常版の上限は 256 KiB = 262,144 bytes 未満、軽量版の表では 64 KiB = 65,536 bytes 未満です。全 12 ヘッダーの使用例には軽量版の逆元呼び出しも加えています。関数の定義はすべて保持しています。異なる解答部分や他のライブラリを足した最終ファイルは `wc -c` で確認します。[使用例と原票](results/fps_plain/audit.json)

全 208 ヘッダーをそれぞれ単独・Fast I/O 付きで展開し、既存の 181 提出例も調べました。**計 597 通りが追加圧縮なしで 256 KiB 未満**でした。参照 URL 付きの最大は 253,415 bytes です。[サイズ原票](results/fps_plain/sizes.json)

保存済みの [Kth term 提出コード](../../submissions/kth_term_of_linearly_recurrent_sequence.cpp)も通常展開で更新し、参照 URL 込みで **239,753 bytes** です。GCC・Clang の両方でコンパイルし、公式 checker で小さいケースと最大ファイルなどを確認しました。[提出コードの検査](results/fps_plain/submission.json)

## 配布実装と編集用原本

通常版の実装と公開関数を `components/fps_backend.hpp` にまとめ、既存の公開ヘッダー・内部部品から読み込みます。共通する記述の辞書は一度だけ含まれ、全機能を併用しても重複しません。軽量版は独立した `components/fps_light_backend.hpp` にまとめ、通常版を読み込みません。Fast I/O の 3 ヘッダーも短縮済みの内容にしました。

編集用原本は、対応する場所の `.inc` に保存しています。**40 原本は基準の `.hpp` とバイト単位で一致**します。演算・const 性・精度上限・CPU 判定・非 Linux / 非 x86 の分岐は変えていません。配布ヘッダーでは共通するトークン列を一時的な内部マクロへまとめ、通常版の内部変数名も短くしています。内部マクロは末尾で解除します。

メンテナーが原本を編集した場合だけ、次のコマンドで配布ヘッダーを更新します。利用者が提出時に実行する必要はありません。CI は原本と配布内容の一致、および全ヘッダー・全 API 使用例のサイズを検査します。

```sh
python3 tools/update_fps_headers.py
python3 tools/update_fps_headers.py --check
```

[原本と配布ヘッダーの SHA-256](results/fps_plain/distribution.json)、[原本と基準版の一致・最終検証](results/fps_plain/validation.json)を記録しています。

## 生成コードの同値性

通常版・通常版と軽量版の併用・軽量版の 3 使用例を GCC 14.2 と Clang 19 で比較し、GCC の非 Linux 分岐も加えました。**7 組すべてで、内部変数名を戻したプリプロセス後の全トークン列が一致し、NDEBUG 設定の実行ファイル全体の SHA-256 も一致**しました。使用例の実行も成功しました。

assert 有効時も、診断の文字列・ファイル名・行番号と診断用 pragma だけを正規化し、**7 組すべてで条件式を含む全トークン列の一致**を確認しました。[リリース設定の原票](results/fps_plain/audit.json)、[assert 有効時の原票](results/fps_plain/assert_audit.json)

配布ヘッダーでは、短縮されたレイアウトに起因する `misleading-indentation` 診断だけを局所的に抑制し、末尾で診断設定を戻します。Clang が句読点に似た文字として警告する U+01C3 は、短縮用識別子に使いません。

## 速度

Linux x86_64、AMD EPYC 9V74、GCC 14.2、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell -Wall -Wextra`。NDEBUG は指定していません。73 条件を前後それぞれ 9 回、実行順を乱択して比較し、**計 1,314 サンプル**の回答サイズと全係数 checksum が一致しました。毎回新しいプロセスを使い、入力生成と checksum は計時外、演算中の確保は計時内です。

基本演算は `n=524288`、追加演算は `n=32768` の中央値です。単位は ms。小さい差には配置・CPU 状態・測定のばらつきも含まれます。

| 演算 | 変更前 ms | 変更後 ms | 実行時間の増減 |
| --- | ---: | ---: | ---: |
| 積 | 5.244 | 5.304 | +1.14% |
| 二乗 | 5.708 | 5.739 | +0.54% |
| 逆数 | 8.033 | 7.777 | -3.19% |
| 対数 | 10.393 | 10.747 | +3.41% |
| 指数 | 13.389 | 12.054 | -9.97% |
| 平方根 | 7.570 | 7.425 | -1.91% |
| 冪 | 19.709 | 19.864 | +0.78% |
| 級数除算 | 9.859 | 9.765 | -0.95% |
| 多項式除算 | 7.608 | 7.429 | -2.35% |
| Taylor shift | 0.378 | 0.389 | +2.80% |
| 多点評価 | 2.402 | 2.559 | +6.50% |
| 補間 | 3.400 | 3.375 | -0.73% |
| 線形漸化式 | 12.442 | 12.542 | +0.81% |
| 合成 | 5.985 | 6.030 | +0.75% |
| 合成逆元 | 6.113 | 6.137 | +0.38% |
| power projection | 5.150 | 5.129 | -0.40% |
| GCD | 8.273 | 8.194 | -0.95% |
| 拡張 GCD | 8.992 | 9.096 | +1.15% |
| 多項式の逆元 | 7.897 | 7.979 | +1.04% |

`n>=32768` の最大の増加は多点評価の **6.50%**（2.402 → 2.559 ms）です。全条件では多項式除算 `n=1024` の **12.63%**（0.033380 → 0.037596 ms、約 4.2 µs 増）が最大でした。通常版の高速な演算実装を維持しています。[測定原票](results/fps_plain/performance.json)、[全サンプル CSV](results/fps_plain/performance.csv)

基礎となる手法・参照原本は [FPS の既存調査](fps_round4.md)を引き継いでいます。今回の変更は配布表現で、judge の順位を新しく測った結果ではありません。

## 正答検証

- GCC・Clang の単独 include **108 件**と単体テスト **88 件**が成功しました。
- Clang の ASan/UBSan **21 件**、Linux GCC の native AVX2 経路の ASan/UBSan **6 件**が成功しました。guard page、読み取り専用入力、alignment、padding、出力領域の借用を含みます。
- 全 API・include 順・高速 I/O の先頭配置・利用者マクロ・実 ACL modint・非 Linux 分岐の **14 通り**が成功しました。
- 実 NTT 長 `2^23` の dense な積と、他の公開出力端点を確認しました。他の端点では疎・単項式入力を使っています。
- Yosupo の **54 提出例**を GCC・Clang で実行し、公式 checker で **延べ 3,054 ケース**が成功しました。外部 AtCoder / yukicoder の 7 提出例は再提出せず、対応する単体検証で確認しています。
- サイトの全 **389 ソース**（208 ヘッダー・181 提出例）を生成し、全ヘッダーの参照 URL 付き Copy サイズと、変更の影響する **56 Bundle** の構文検査が成功しました。全体・軽量版の一覧、リンク、コピー対象の対応も検査し、全 208 件が 256 KiB 未満、158 件が軽量版の 64 KiB 以内でした。
- 結合後の Bundle・生成器の回帰 6 件、CI の回帰 46 件、サイズ別サイトの Python 検査 10 件、コピー操作の Node 検査 8 件が成功しました。Fast I/O の AtCoder 相当 constexpr 制限と、全 API 使用例を含むサイズ予算の検査も成功しています。

サイトのサイズ別表示を追加した `785e1a3` を取り込みました。C++ 実装 270 ファイルのハッシュは正答・速度検証時と一致しています。サイト再生成後も 206 ヘッダーの展開コードが以前の検査時と一致し、初回の部分生成で古いコードを残していた Fast I/O の 2 ページは更新して構文検査を追加しました。[サイト検証](results/fps_plain/documentation.json)、[回帰検査](results/fps_plain/python_checks.json)

公式入力 2 問の追加生成を挟んだため、単体検証と公式検証は別原票です。成功済みの検査はソース・コンパイラ・ログの SHA を確認して再利用し、[統合原票](results/fps_plain/validation.json)で全対象の完了を検査しています。[単体検証の原票](results/fps_plain/core_checks.json)、[公式検証の再開原票](results/fps_plain/official.json)、[GCC native sanitizer](results/fps_plain/native_sanitize.json)

追加 GCC sanitizer の評価テスト 1 件は、従来と同じ到達不能な `Part[1]` に対する GCC の `array-bounds` 診断だけをエラー扱いから外しました。警告表示と ASan/UBSan の実行時検査は有効です。通常の単体検証・Clang sanitizer は `-Werror` のままです。非 Linux 分岐は Linux 上での `-U__linux__` による確認で、Mac 実機での速度測定ではありません。

## 再現

```sh
python3 tools/update_fps_headers.py --check
python3 test/benchmark/fps_plain_audit.py --work /tmp/fps-plain-audit --output /tmp/fps-plain-audit.json --bundle oj-bundle --cxx g++ clang++
python3 test/benchmark/fps_plain_assert_audit.py --audit /tmp/fps-plain-audit.json --output /tmp/fps-plain-assert.json
python3 test/benchmark/fps_256k_refactor.py --baseline c6bee6c --suite all --trials 9 --output /tmp/fps-plain-performance.json
python3 test/benchmark/source_size_budget.py --work /tmp/fps-plain-sizes --output /tmp/fps-plain-sizes.json --bundle oj-bundle --jobs 3
```

作業ディレクトリは未使用のパスを指定します。正答検証には、Library Checker の公式入力・出力・checker と ACL のチェックアウトを用意し、`test/benchmark/fps_refactor_check.py --baseline c6bee6c` の `--problems`・`--acl` に渡します。各原票には実際のコンパイラ・コマンド・ソースと回答の SHA を保存しています。
