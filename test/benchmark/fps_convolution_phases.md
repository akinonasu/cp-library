# 畳み込みの段階別監査

`f2906ffa0b76dc08f34890e2e2390d27b2a0befc` と [最速原本 409465](https://judge.yosupo.jp/submission/409465) を、確保から解放まで同じ範囲で比較した。**Plan の確保・初期化と `multiply` を各試行で合算した中央値は、現行 4.312119 ms、原本 4.298859 ms だった。** 今回の測定では、大きな専用演算の移植漏れを示す差は確認できなかった。

原本を変更したものは分析専用コピーであり、正式な全体比較の原本ではない。現行の分析 driver は公開 driver が最大ケースで選ぶ `run<Product>` と入出力の処理を展開し、既存の `Product`・`copy_input`・結果生成 helper を呼ぶ。計測のための独立コピー・関数展開・コンパイラ最適化・コード配置による摂動を含む。既存のヘッダーと正式 driver は変更していない。

## 条件と全体時間

2026-10-18 UTC、Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 固定。オプションは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。分析側へ複写した現行 helper は、元の O3/AVX2 属性も維持した。ビルドは `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロックで分離した。

公式 `convolution_mod/max_random_00`（両入力 524,288 項）を通常ファイルから読み、通常ファイルへ出力した。各版 1 回の予備実行後、seed `20261021` で順序をシャッフルし、21 回ずつ測った。無計測版 2 本・分析版 2 本の全 88 出力が公式 checker を通過し、出力ファイルの SHA-256 も一致した。

| 中央値 ms | 現行 | 原本 |
| --- | ---: | ---: |
| 無計測のプロセス全体 | 15.317114 | 11.656700 |
| 分析版のプロセス全体 | 15.051052 | 11.677331 |
| 分析版の内部区間 | 13.200801 | 10.122863 |
| 各試行の「全体−内部区間」 | 1.864419 | 1.554688 |

分析版を作ると、全体の中央値は現行で **−0.266062 ms**、原本で **＋0.020631 ms** 変わった。したがって分析版の内訳を、無計測バイナリの厳密な内訳としては扱わない。内部区間の外には起動・ロード、main/preinit 前の処理、終了と待機、診断出力などが含まれる。原本の preinit 実行と `_exit`、現行の通常の main 実行は保持した。

## 対応する区間

次の各行は、**同じ試行の該当 phase を先に合算してから、その 21 値の中央値を取った値**である。行ごとの中央値を足して処理全体の中央値を求めることはしない。

| 各試行で合算した区間の中央値 ms | 現行 | 原本 |
| --- | ---: | ---: |
| I/O・ヘッダー・出力領域の準備 | 0.120549 | 0.018177 |
| 入力確保・parsing・Mint 生成・scratch 解放 | 3.545765 | 2.354461 |
| Plan 確保・初期化＋multiply（根表生成込み） | 4.312119 | 4.298859 |
| 内部 copy_input | 0.652984 | 0 |
| 結果 vector の生成 | 0.688506 | 0 |
| 出力変換・format・write | 3.229616 | 3.055525 |
| 各領域の解放・最終 flush | 0.653604 | 0.381238 |

原本は入力より先に Plan を確保し、その内部配列へ直接 parsing する。現行は公開 Mint vector を生成してから API 内で別の作業配列を確保する。現行の公開入力確保は `MADV_POPULATE_WRITE` を含み、原本の仮想領域確保は後の初回アクセスへページ実体化の費用を送るため、確保や parsing の行だけで差を断定しない。

参考として、Plan 確保・初期化だけの中央値は現行 0.200779 / 原本 0.007151 ms、`multiply` だけは 4.110679 / 4.288423 ms だった。現行の根表の零初期化が THP の初回ページアクセスを前倒しし、計算区間を約 0.18 ms 軽く見せている可能性があるが、これは分離して実測した説明ではない。主比較には確保・初期化も含む 4.312119 / 4.298859 ms を使う。`copy_input` もコピー・正規化だけでなく、書き込み先の作業配列へ初めてアクセスする費用を含む。

現在の実装で追加の時間が現れる主な区間は、公開入力・可変の内部配列・結果 vector を別々に維持する処理と、その変換・解放だった。この結果は API の不可避な下限や、さらなる高速化の不可能性を示すものではない。以前の `6.49 / 4.08 ms` という計算部分の比較には確保などの範囲の不一致があり、約 2.4 ms の専用演算差の根拠には使えない。

## 計時と scope の確認

`CLOCK_MONOTONIC` の連続した timestamp で区切った。各試行では `sum(phases_ns) == internal_ns` が整数で完全一致し、区間の重複はない。内部の区間和と外側の残差で、その試行の全体時間まで対応する。原票の `sum_of_phase_medians_ms` は非加法性を確認する診断値で、全体時間や主比較の算出には使わない。

時計の呼び出しは現行 203 回、原本 91 回。100 万 step を 9 回測った較正値は 1 step あたり中央値 24.570005 ns。内部区間に載る時計と集計の推定費用は 0.004963 / 0.002211 ms だった。最後の timestamp 後の診断整形・書き込みはこの推定値に含まれず、全体時間には含まれる。この較正ではコード配置や inlining の影響を補正できない。

別 agent の読み取りレビューでも、入力長・32768/25600 のブロック幅、処理の順序、一時 vector と各領域の解放順、連続区間に抜けや二重計上は見つからなかった。注意点は次の通り。

- 現行 FastOutput の構築は入力前、原本 Writer の構築は計算後である。準備区間は合算して比較した。
- 現行の出力準備は自動初期化される stack 配列を含む。最終 flush 区間は現行の `fflush` も含む。
- 分析 driver は両入力長が 32 より大きく、`Product::fits` を満たす今回の最大ケース専用で、一般の dispatch を再現するものではない。

所有する `Leaves` scratch の初期化も、無計測バイナリを逆アセンブルして照合した。現行の明示的な `Leaves[2][2]{}` と、原本に対する `-ftrivial-auto-var-init=zero` は、どちらも `memset(ptr,0,0x600)` で 1536 byte を零にしていた。この箇所に現行だけの余分な零初期化は見つからなかった。初期化は変更していない。

測定後に main の `ddcb3a7` を取り込んだ。追加された階乗の overload は既存 FPS の raw 呼び出し先を変えず、正式比較の 16 問を同じパス・flags で再ビルドしたバイナリは、すべて測定時の SHA-256 と一致した。`upstream_integration.json` にコンパイルコマンドと対応を記録している。今回の分析用 snapshot 自体は変更していない。

## 保存物と再現

[results/fps_convolution_phases/](results/fps_convolution_phases/) に原票 `results.json`、計時計 `profile.hpp`、分析用現行 driver、原本の分析用 `reference.patch`、時計較正 driver、scope レビュー、初期化の assembly 抜粋を保存した。原票には全試行、各 phase、ソース・バイナリ・入出力の SHA-256、完全なコンパイルコマンド、グループの定義を含む。`archive_check.json` は保存物と復元の照合記録である。元の 106 ソースファイルと分析 driver は、復元後も実測時のハッシュと一致した。

参照ソースと、記録した Library Checker コミットの入力を用意する。以下の `/tmp` は復元先の例であり、測定時の一時ファイルは不要である。

```sh
python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference
git clone https://github.com/yosupo06/library-checker-problems /tmp/library-checker-problems
git -C /tmp/library-checker-problems checkout 1814c4e5205517e368bb57a8d1127eb961cfeaae
flock -s /tmp/cp-fps-tuning.lock \
  python3 /tmp/library-checker-problems/generate.py -p convolution_mod
python3 test/benchmark/results/fps_convolution_phases/prepare.py \
  --work /tmp/fps-conv-phases-reproduction \
  --reference /tmp/fps-reference/convolution_mod/409465.cpp \
  --official-root /tmp/library-checker-problems --measure
```

`--measure` を `--prepare-only` にするとソース復元とハッシュ検査まで、`--build` にするとビルドまでを行う。基準コミットを含む Git 履歴が必要である。再計時は作業先へ新しい `results.json` を作り、保存した原票を上書きしない。異なる作業パスでのバイナリのハッシュは変わる場合がある。今回の資料保存ではソース復元だけを検証し、計時は追加していない。
