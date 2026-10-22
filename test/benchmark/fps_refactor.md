# FPS 全機能の 256 KiB 対応

この記録は `c6bee6c` 時点の専用生成スクリプトを使った方式です。現在は[通常展開だけで収める方式](fps_plain.md)へ移行し、`tools/bundle.py` は削除しています。以下は当時の測定と再現記録です。

2026-10-22 JST。基準は `32f6930`。通常版の公開 API と精度上限を保って実装を共通化し、全ヘッダーを同時に include した提出コードを 256 KiB 未満にしました。個々の演算だけを残す生成ではなく、生成後も全関数の定義を含みます。

```sh
python3 tools/bundle.py a.cpp -I . -o submit.cpp
```

サイトの Bundle も同じ処理です。通常の `oj-bundle` の出力は引き続き 256 KiB を超えるため、提出する解答全体を上記コマンドへ渡します。生成時に最終 UTF-8 バイト数を検査し、上限以上なら既存の出力先を変更せずエラーにします。

## サイズ

各例は `components/fast_io.hpp` と公式提出例用の `test/fps_io.hpp` も含みます。通常版は積・逆数・対数・指数・平方根・冪・除算に加え、Bostan–Mori、線形漸化式、合成・合成逆元、power projection、多点評価・補間、Taylor shift、GCD・拡張 GCD・多項式の逆元をすべて含む 10 ヘッダーです。

| 結合するもの | 変更前の生成後 bytes | 変更後の生成後 bytes | 262,144 bytes までの余裕 |
| --- | ---: | ---: | ---: |
| 通常版全 10 ヘッダー＋高速 I/O | 331,386 | 241,884 | 20,260 |
| 通常版・軽量版全 12 ヘッダー＋高速 I/O | 345,836 | 256,193 | 5,951 |
| 通常版全ヘッダー＋全公開 API を呼ぶ実使用例＋高速 I/O | 333,601 | 244,200 | 17,944 |

通常版の生の結合結果も 626,325 → 492,453 bytes に減りました。同じ現行の圧縮処理を前後に使っても 329,254 → 241,884 bytes で、本体の共通化による削減は 87,370 bytes です。残りの差は、2 回だけ出現する共通トークン列も辞書に採用する生成処理の改善です。`#line` に入る入力名・パスと解答部分によりサイズは変わります。

[サイズ・全ソース SHA・試行原票](results/fps_refactor/performance.json)と[実使用例のコンパイル・実行](results/fps_refactor/usage_fixture.json)を保存しています。

## 共通化

- 積専用の NTT と、他の演算用の blocked Transform / Recursion を共通化しました。`fps_convolution.hpp` は 64,387 → 7,084 bytes。両入力を Montgomery 表現のまま扱い、最後の既存のスケール演算へ補正を融合します。
- blocked NTT の端の処理と小さい変換を既存の SIMD 処理へ集約しました。`fps_blocked.hpp` は 89,982 → 61,151 bytes。
- 合成の列変換 4 本を共有テンプレートに、多点評価・補間の小さい積木を共通の処理にしました。計 10,040 bytes 削減しました。
- NTT の ARM 専用処理は既存のスカラー処理へ統合しました。旧合成専用の特殊化と未使用の逆元処理も整理し、`ntt.hpp` は 78,118 → 55,113 bytes になりました。Linux の通常の合成には引き続き専用の高速経路を使います。
- 対数の新しい Linux 経路と同じ条件で、その後に置かれていた旧経路を除きました。旧ヘルパー自体は単独利用・検証用に残しています。
- Taylor shift の 513 要素の定数表は、同じ値をコンパイル時に生成する形にしました。旧表の全要素との一致を確認しています。実行時の表生成は増えません。

逆数の旧特殊化を単純な Newton 更新へ統一する案は、小サイズで最大 1.62 倍遅くなったため採用しませんでした。大きい変換の過剰な剰余削減を使う案も除外しています。[採否と再現資料](results/fps_refactor/kernels/metadata.json)、[追加演算](results/fps_refactor/extras/metadata.json)、[旧経路の整理](results/fps_refactor/fallback/reference.json)に記録しています。

基礎となる手法・原本は従来の [FPS 調査](fps_round4.md)と[409465](https://judge.yosupo.jp/submission/409465) などの記録を引き継いでいます。今回、judge のランキングを取り直したり、新しい judge 実行時間を測ったりはしていません。

## 速度

Linux x86_64、AMD EPYC 9V74、GCC 14.2、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell -Wall -Wextra` で比較しました。73 条件を前後それぞれ 9 回、毎回新しいプロセスで実行し、合計 1,314 サンプルを記録しています。入力生成と全係数 checksum は計時外、演算内部の確保は計時内です。前後の実行順を試行ごとにシャッフルし、コンパイルや他の計測との排他ロックを取得しています。

基本演算は `n=524288` の中央値です。数値はローカルでのミリ秒で、judge の表示時間とは別です。

| 演算 | 変更前 ms | 変更後 ms | 実行時間の増減 |
| --- | ---: | ---: | ---: |
| 積 | 4.810 | 5.362 | +11.5% |
| 二乗 | 4.716 | 5.036 | +6.8% |
| 逆数 | 7.262 | 7.393 | +1.8% |
| 対数 | 9.022 | 9.117 | +1.1% |
| 指数 | 11.750 | 12.187 | +3.7% |
| 平方根 | 7.412 | 7.756 | +4.6% |
| 冪 | 19.314 | 20.043 | +3.8% |
| 級数除算 | 9.076 | 9.403 | +3.6% |
| 多項式除算 | 7.820 | 7.122 | -8.9% |

追加演算は `n=32768` です。

| 演算 | 変更前 ms | 変更後 ms | 実行時間の増減 |
| --- | ---: | ---: | ---: |
| Taylor shift | 0.437 | 0.440 | +0.8% |
| 多点評価 | 2.430 | 2.469 | +1.6% |
| 補間 | 3.278 | 3.308 | +0.9% |
| 線形漸化式 | 12.593 | 12.617 | +0.2% |
| 合成 | 5.903 | 6.089 | +3.2% |
| 合成逆元 | 5.995 | 6.285 | +4.8% |
| power projection | 5.000 | 5.214 | +4.3% |
| GCD | 7.890 | 7.959 | +0.9% |
| 拡張 GCD | 8.756 | 8.929 | +2.0% |
| 多項式の逆元 | 7.700 | 7.818 | +1.5% |

全条件で最大の増加は積 `n=32768` の **13.83%**（0.325 → 0.370 ms）です。平方根 `n=32768` も 11.65% 増加しました。表の大きい積は 11.47% 増、それ以外の大きい基本演算は最大 6.8% 増でした。大幅に遅くなる軽量版への置換はせず、通常版の高速経路を共通化しています。小さい差には測定のばらつきも含みます。

通常の高速経路の対象を超える合成 `n=2097153` は、NTT 整理単体の比較で 2973.020 → 3093.769 ms（4.06% 増）、全係数 checksum が一致しました。この比較は最終 73 条件とは別の 3 試行で、[測定版と最終版の差](results/fps_refactor/ntt/manifest.json)も記録しています。

[生 CSV](results/fps_refactor/performance.csv)から中央値と回答一致を再計算し、[測定後監査](results/fps_refactor/performance_audit.json)で全依存・ツール・バイナリの一致を確認しています。

## 生成と安全性

`ProductBottom` は実際のローカルポインタと整数の開始位置を保持し、添字の差を計算してからアクセスします。部分木の先頭から親の基底を逆算して、配列の前のポインタを形成することはありません。既存の経路の開始位置は 0 で、借用する出力領域の alignment・padding と const 入力の不変性も維持しています。[独立レビュー](results/fps_refactor/fallback/design_review.json)に範囲とスケールの根拠を残しました。

生成処理は 192 KiB 以上から圧縮するようにし、単独ヘッダーの Bundle でも解答部分の余裕を確保します。通常版と軽量版の全ヘッダーを同時に include して演算を呼ぶ例を、サイズの回帰テストへ加えました。

通常版全体・軽量版併用・全 API 使用例について、GCC 14.2 と Clang 19 の 6 組、および GCC の非 Linux 分岐 1 組で圧縮前後を比較しました。**7 組ともプリプロセス後の全トークン列と ELF ファイル全体の SHA-256 が一致**し、実行も成功しました。生成処理自体はこの比較で実行時の計算を変えていません。[原票](results/fps_refactor/bundle/audit.json)と[再現方法](results/fps_refactor/bundle/manifest.json)を保存しています。

## 正答検証

- 変更した本体と推移的な依存先を選び、GCC・Clang の単独 include 90 件と単体テスト 82 件が成功しました。
- Clang の ASan/UBSan 21 件と、Linux GCC の native AVX2 経路を通す追加 ASan/UBSan 6 件が成功しました。借用出力、guard page、読み取り専用入力、alignment、末尾の padding も含みます。
- 全 API・include 順・高速 I/O の先頭配置・実 ACL modint・利用者マクロ・portable 分岐を組み合わせた 14 通りが成功しました。
- Yosupo の 27 提出例を GCC・Clang で実行し、公式入力はそれぞれ 773 件、計 **1,546 件**成功しました。AtCoder / yukicoder の外部 6 提出例は再提出せず、実依存の単体テストで確認しています。
- balanced 積で実 NTT 長 `2^23` を通し、全係数を閉形式と照合しました。他の公開出力端点も疎・単項式入力で確認しました。全演算の最大サイズの dense カーネルを実行した、という検査ではありません。
- Clang の対数は `f=1+x+…+x^64`、出力長 `2^24` の **16,777,216 係数すべて**を閉形式と照合しました。旧対数経路の削除後も上限を保っています。Mac 実機ではなく Linux / Clang の確認です。
- サイトは影響する 78 ソースを再生成し、206 ヘッダーのサイズ検査と影響する 45 Bundle の構文検査が成功しました。生成器の回帰 5 件、および軽量版との全 12 ヘッダー併用のサイズ回帰も成功しました。

[主検証原票](results/fps_refactor/validation.json)、[GCC native sanitizer の統合原票](results/fps_refactor/native_sanitize_manifest.json)、[Clang の対数最大精度](results/fps_refactor/fallback/log_precision.json)にコマンドとソース・入力・checker の SHA を保存しています。

追加の GCC sanitizer の評価テストに限り、基準版でも同じ診断が出る、到達不能な `Part[1]` の `array-bounds` 警告をエラー扱いから外しました。警告表示と ASan/UBSan の実行時検査は残しています。通常の GCC/Clang テストと Clang sanitizer は `-Werror` のままです。ライブラリと既存 CI の警告設定は変更していません。

## 再現

```sh
python3 test/benchmark/fps_256k_refactor.py --suite all --trials 9 \
  --bundler /path/to/oj-bundle --bundle-directory /tmp/fps-bundles \
  --output /tmp/fps-performance.json
python3 test/benchmark/fps_refactor_check.py \
  --problems /path/to/library-checker-problems --acl /path/to/ac-library \
  --cxx g++ clang++ --sanitize-cxx clang++ --jobs 2 \
  --stages standalone unit sanitize integration precision official \
  --work /tmp/fps-validation --output /tmp/fps-validation.json
python3 test/benchmark/fps_refactor_native_sanitize.py \
  --output /tmp/fps-native-sanitize.json
python3 -m unittest discover -s .verify-helper -p bundle_test.py
```

基準ソースは Git の `32f6930` から復元し、現行ソース・実際の依存先・コンパイラ・バイナリを測定前後に SHA-256 で確認します。生成コードの追加検証は [再現スクリプト](results/fps_refactor/bundle/)を参照してください。
