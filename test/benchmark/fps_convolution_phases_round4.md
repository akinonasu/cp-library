# 畳み込みの段階別再計測

`7521ea8703a362aaf3ccd370188fccab86050848` と未編集の [409465](https://judge.yosupo.jp/submission/409465) を比較した。借用 A、新しい入力 parser、`Mint` の constructor 調整を含む現在の実装が対象で、[以前の計測](fps_convolution_phases.md)を置き換えて読むための分析である。**今回も kernel 単独の移植漏れを示す差はなく、結果配列の準備・入力の保持・コピー・解放に主な追加時間が現れた。** これは API の下限や、高速化が不可能という意味ではない。

production は変更していない。分析版は現行 `run_borrowed` と I/O を展開して時計を挿入し、原本の分析コピーは preinit・`_exit`・各オブジェクトの解放順を維持した。計測用コードの inlining・配置・時計呼出しによる摂動を含む。

## 条件

2026-10-19 UTC、Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 固定。flags は `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。複写した現行 helper も元の O3/AVX2 属性内で定義した。ビルドは共通 lock の SH、計時は EX。

公式 `convolution_mod/max_random_00`・`01`（両入力 524,288 項）を通常ファイルから読み、通常ファイルへ出力した。無計測 2 本・分析 2 本を各 1 回予備実行した後、seed `20261040` で順序をシャッフルして各 9 回測った。予備実行を含む全 80 出力が `checker input actual answer` を通過し、各ケースでは出力 SHA-256 も 4 版で一致した。追加の計時は行わず、この 1 組を分析記録として固定した。実 judge の速度や順位ではない。

## 全体と計測による変化

単位 ms、各 9 回の中央値。「全体−内部」は各試行で差を取ってから中央値にした。

| 区間 | 00 現行 | 00 原本 | 01 現行 | 01 原本 |
| --- | ---: | ---: | ---: | ---: |
| 無計測のプロセス全体 | 12.159 | 9.664 | 12.103 | 9.783 |
| 分析版のプロセス全体 | 12.352 | 9.733 | 12.275 | 9.846 |
| 分析版の内部区間 | 10.493 | 8.192 | 10.444 | 8.322 |
| 分析版の全体−内部 | 1.828 | 1.569 | 1.762 | 1.544 |

無計測版に対して分析版は、現行で +0.193 / +0.172 ms、原本で +0.069 / +0.063 ms だった。このため以下を無計測版の厳密な内訳として扱わない。外側の残差には起動・ロード・main/preinit 前の処理・終了待機・最後の診断出力などが含まれる。両版の main と preinit/`_exit` の違いは保持しており、単に揃えて消してはいない。

## 対応する機能別の費用

各試行の該当 phase を先に合算してから中央値を取った。列ごとの中央値を足して全体を再構成することはしない。下表の group はすべての phase を重複なく覆うが、機能別の合算であり、参照版での連続した API 区間を意味しない。

| 合算した区間 | 00 現行 | 00 原本 | 01 現行 | 01 原本 |
| --- | ---: | ---: | ---: | ---: |
| I/O・header・出力領域の準備 | 0.120 | 0.015 | 0.118 | 0.015 |
| 入力確保・parse・Mint 生成・scratch 解放 | 2.511 | 1.841 | 2.452 | 1.856 |
| 結果確保・Plan 準備・copy・kernel・移動/resize | 5.323 | 4.333 | 5.348 | 4.319 |
| 出力変換・format・write | 2.093 | 1.905 | 2.068 | 1.988 |
| Plan/結果/入力の解放・最終 flush | 0.316 | 0.067 | 0.295 | 0.070 |

現在は結果 `vector<Mint>` の整列した内部を A として借りる。確保と `assign(words,0)` は計算前に移っており、Plan の確保・初期化と kernel だけを比較すると、その費用を落としてしまう。上表の中央の group は結果の reserve/assign、Plan の確保・初期化、A/B のコピー、根表生成を含む multiply、最後の memmove/resize を含める。解放は別 group に置いた。参照版の Plan 解放は出力後なので、これを加えた値を「連続 API 区間」とは呼ばない。

次の個別値は参考値である。

| 個別 phase | 00 現行 | 00 原本 | 01 現行 | 01 原本 |
| --- | ---: | ---: | ---: | ---: |
| 公開入力 reserve/準備 | 0.572 | 0 | 0.512 | 0 |
| 入力 parse・遅延 scratch 確保等 | 1.720 | 1.841 | 1.718 | 1.856 |
| 入力 Mint 生成 | 0.207 | 0 | 0.196 | 0 |
| 結果 reserve/assign | 0.639 | 0 | 0.611 | 0 |
| Plan 確保/初期化 | 0.182 | 0.007 | 0.184 | 0.007 |
| A/B copy | 0.478 | 0 | 0.464 | 0 |
| multiply（根表生成込み） | 3.921 | 4.326 | 3.962 | 4.311 |
| 最後の memmove/resize | 0.060 | 0 | 0.059 | 0 |
| 出力 Montgomery 変換 | 0.157 | 0 | 0.149 | 0 |

入力と結果の準備は `MADV_POPULATE_WRITE` や初回ページアクセスを含み、reserve/assign の数字全体が零埋めループの費用ではない。原本は Plan 内へ直接 parse し、その過程でも配列を初めて実体化する。現行は公開 Mint 入力を維持し、結果と内部 B へコピーする。first touch が別区間に移るため、kernel 単独が軽く見える可能性もある。root table・ページ状態・配置の寄与をこの計測だけで個別に確定していない。候補の採否には、その候補自身の無計測プロセス全体との比較が必要である。

現行の `read_blocks` は元の実装を呼び、emit 直前・直後だけを区切った。4 stream の順序や scratch の寿命は変えていない。最初の parse phase は遅延 scratch 確保を含み、最後の scratch phase はループ終端・ポインタ更新・解放も含む。このため入力 group 全体を主に比較する。出力の 25,600 係数刻みと処理順も元のままである。

## 監査と保存物

`CLOCK_MONOTONIC` の連続した timestamp を使い、全分析試行で整数の `sum(phases_ns) == internal_ns` が完全一致した。時計回数は現行でケース 00 が 1,066 回、01 が 1,064 回、原本は各 91 回。100 万 step × 3 回の較正中央値は 24.962 ns/step。内部の時計・集計の推定費用は現行で約 0.027 ms、原本で約 0.002 ms だが、最後の診断整形/write と、コード配置や inlining の摂動はこの推定に含まない。

独立した読み取りレビューで、borrowed-A の生存期間、入力長、配列境界、解放順、preinit/`_exit`、時計の抜け・二重計上を確認した。ライブラリ snapshot の 111 ソースは測定後も不変であり、保存時の production とも一致した。

[原票](results/fps_convolution_phases_round4/results_9.json)、[phase 監査](results/fps_convolution_phases_round4/phase_audit.json)、[独立 scope review](results/fps_convolution_phases_round4/scope_review.json)、[コンパイルコマンドと source/binary hash](results/fps_convolution_phases_round4/build.json) を保存した。結果・試行順・各 phase・各入力/正解/出力の hash は原票にある。分析用現行 driver、計時計、原本への分析用 patch、測定・復元 script も同じディレクトリに置いた。原本と library header は複製せず、Git の固定 revision と原本 SHA から復元する。

```sh
python3 test/benchmark/results/fps_convolution_phases_round4/prepare.py \
  --work /tmp/convolution-phases-round4-replay \
  --reference /tmp/fps-reference/convolution_mod/409465.cpp --prepare-only
```

復元を実行し、全 snapshot と両分析 driver の source SHA が測定時と一致することを確認した。`--build` は両 control・両 profile と時計較正を SH 下でビルドする。`--measure` は続けて EX 下で同じ 9 回の比較を行い、復元先の `replay.json` に保存する。公式データが別の場所にある場合は `--official-root` を指定する。測定時と異なるパスでは binary hash やコード配置が変わり得る。復元確認のための追加計時はしていない。
