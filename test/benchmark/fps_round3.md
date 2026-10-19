# FPS の追加高速化と SIMD 候補の比較

2026-10-19 UTC。比較元は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。公開 API、入力を変更しない性質、一般の係数型の経路を保ち、残っていたコピー・初期化・変換の費用を削減した。

## 採用した変更

| 対象 | 変更 | 詳細 |
| --- | --- | --- |
| 係数入力 | parser の作業領域を一回の入力で共有し、4 ストリームの再結合を省いて Mint へ直接追加する。残部は 64 係数ずつ変換する | [入力](fps_io_round3.md) |
| 畳み込み | 生存中の結果 vector の整列した領域を変換用に借用し、別の作業領域からの結果コピーを省く | [畳み込み](fps_convolution_borrow_round3.md) |
| 指数 | 2 冪を 1〜4 項だけ超える長さは、既存の指数本体で prefix を計算し、微分の恒等式による SIMD 内積で末尾を補う | [指数](fps_exp_round3.md) |
| Bostan–Mori | doubling の逆変換に最初のコピーと最後の twist 乗算を融合する | [Bostan–Mori・除算](fps_division_round3.md) |
| Mint | 既定コンストラクターを常に inline 化し、GCC が vector の零初期化に残していた要素ごとの呼び出しを除く | [Mint](fps_mint_inline_round3.md) |

借用する出力は、先に Mint オブジェクトを生成する。入力の const 性と同一 vector の二乗を維持し、確保領域外への読み込み・書き込みを要求しない。入力 parser は全ストリームを検証してから callback を呼び、失敗した分割を部分的に確定しない。指数の追加経路も、入力末尾の padding を要求しない。各部品の整数範囲・生存期間・境界は専用テストと資料に記録した。

## 同条件の全体比較

AMD EPYC 9V74、Linux x86_64、GCC 14.2、CPU 2 固定。全版の flags は `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。公開 API の配列生成、確保・コピー・計算・出力、プロセスの起動と終了を含む。ビルド・検証は共有 flock、計時は排他 flock で分離した。

`fps_all_speed.py` で、比較元・統合版・未編集の最速原本を各 1 回予備実行し、順序を混ぜた 21 回を測る。公式問題の commit は `1814c4e5205517e368bb57a8d1127eb961cfeaae`。各出力を `checker input actual answer` の順で検証し、ソース・ヘッダー・バイナリ・入出力と全試行を [原票](results/fps_round3_all_speed.json) に保存する。今回取得し直した各問題の先頭 C++ 提出も参照中の 16 提出と一致した。[順位取得の原票](results/fps_round3_reference_heads.json)を参照。

単位は ms、列は各版の全体時間の中央値。平方根は答えが存在する `max_random_01`、他は `max_random_00` を使う。

| 処理 | 比較元 ms | 統合版 ms | 最速原本 ms |
| --- | ---: | ---: | ---: |
| 積 | 13.180 | 12.387 | 10.023 |
| 逆数 | 11.915 | 11.730 | 10.286 |
| 対数 | 14.149 | 14.012 | 12.708 |
| 指数 | 17.143 | 16.560 | 15.114 |
| 冪 | 25.008 | 24.674 | 22.986 |
| 平方根 | 12.238 | 12.391 | 10.109 |
| 多項式除算 | 5.923 | 5.835 | 4.652 |
| 線形漸化式 | 54.925 | 53.042 | 56.929 |
| 合成（通常） | 2.759 | 2.778 | 2.702 |
| 合成（large） | 32.851 | 32.872 | 31.867 |
| 合成逆元（通常） | 2.570 | 2.585 | 2.463 |
| 合成逆元（large） | 29.930 | 29.904 | 29.315 |
| 多点評価 | 15.164 | 15.005 | 14.475 |
| 補間 | 19.714 | 19.549 | 18.838 |
| Taylor shift | 10.030 | 9.810 | 8.665 |
| 多項式逆元 | 15.401 | 15.302 | 14.749 |

積と線形漸化式は中央値同士で約 6.0% / 3.4% 短縮した。同じ反復での「比較元−統合版」の中央値は 0.481 / 1.843 ms、勝数は 17 / 19 回（各 21 回）。線形漸化式は最速原本より約 6.8% 短い中央値となった。指数も全体で改善したが、今回の複数変更の効果を一つの変更へ帰属しない。

平方根は中央値同士で 0.154 ms 悪化した一方、対応差の中央値は -0.014 ms、勝数は 10/21 とほぼ中立だった。この懸念に限り、同じ入力と 3 バイナリを再ビルドせず追加 21 回比較した。比較元 13.468 / 統合版 13.239 / 最速原本 11.198 ms、対応差の中央値は +0.194 ms、18/21 回で短縮し、66 出力の checker が成功した。初回の悪化は再現しなかった。初回の表も保持し、この小さい差を全入力での改善率へ広げない。[平方根の追加原票と再現コード](results/fps_round3_sqrt_recheck/sqrt_max_random01_recheck.json)を参照。他の小さい時間差も確定的な改善や退行とは扱わない。

同じ最終バイナリを使い、指数の `near_262144_02`（262145 項）を追加で 21 回比較した。比較元 13.567 / 統合版 10.235 / 最速原本 12.353 ms。対応差の中央値は 3.304 ms、21/21 回で短縮し、全 66 出力の公式 checker が成功した。最大入力と境界入力の短縮率を混ぜない。[追加測定と再現コード](results/fps_round3_exp_boundary/exp_262145_recheck.json)に親原票とのハッシュ照合を含めて保存した。

全関数で最速原本と同等には達していない。特に積・逆数・対数・冪・平方根・除算には差が残る。比較原本と同じ計算カーネルだけでなく、入力を保持する公開 API の配列生成・初期化、境界でのコピー、安全な入出力も時間へ含まれる。ただし、残差のすべてが API 上不可避であるとは確認していない。この表は指定したローカル環境と入力の比較で、実際の judge の順位ではない。

```sh
python3 test/benchmark/fps_all_speed.py \
  --baseline bd951dce6fdcb3ea9db4fcef342147012a8ab0c0 \
  --work /workspace/cp-fps-scratch-round3/fps-round3-all-speed \
  --trials 21 --output /tmp/fps-round3-all-speed.json
```

## SIMD と不採用候補

AVX512F/VL の 256 bit・512 bit カーネル、IFMA の葉の積、根表と倍率の融合、ブロック幅の変更も比較した。この CPU では全体時間が退行するものや入力ごとに勝敗が変わるものがあり、共通カーネルへは採用していない。[NTT・AVX512](fps_ntt_round3.md)、[IFMA](fps_avx512_leaf_ifma.md) に候補と原票を残した。IFMA の葉の積は既存 AVX2 の約 2 倍の時間を要した。

合成・合成逆元・射影のコピー融合と作業領域の重複、評価・補間・Taylor shift の境界処理、除算の入力融合・出力バッチも調べた。計算単体だけの短縮や、全体時間で再現しないものは採用していない。[合成・合成逆元](fps_composition_round3.md)、[評価・補間](fps_evaluation_round3.md)、[除算](fps_division_round3.md) を参照。今回の不採用結果は、追加改善が不可能であることを示さない。

Library Checker の [公式環境表示](https://github.com/yosupo06/library-checker-judge/blob/master/frontend/src/pages/Help.tsx) は `c2d-highcpu-8 (GCP)` である。[GCP の C2D](https://docs.cloud.google.com/compute/docs/compute-optimized-machines#c2d_machine_types) は AMD EPYC Milan で、[対応命令の資料](https://docs.cloud.google.com/compute/docs/cpu-platforms)では AVX512 に対応しない。今回の測定機は AVX512 対応の別機種であり、ローカルの AVX512 の結果を Yosupo の実機へ直接当てはめない。採用した高速経路は AVX2 以下で動作する。

AtCoder の [現行 C++23 環境](https://atcoder.jp/posts/language_update20260616_jp?lang=ja) は GCC 15.2。今回の GCC 14.2 の測定は、AtCoder 実機や GCC 15.2 での最速獲得の確認ではない。

## 最終検証

比較元との推移的な依存差分から、43 ヘッダー・43 単体テスト・53 judge source・43 bundle を選んだ。GCC 14・Clang 19 の各 43 ヘッダー・43 単体と、GCC の重点 5 単体の ASan/UBSan が成功した。[最終 scope audit](results/fps_round3_validation/scope-audit.json)で全対象の実行 coverage と実依存 188 ファイルを照合した。

Yosupo の 48 ドライバー・全 1296 ケースが成功した。Yukicoder の 5 ドライバーは token がないためローカルでは skip し、対応する単体は確認した。実依存 203 ファイルと公式データ 2310 ファイルの前後 SHA が一致し、公式の `info.toml` の全ケース数、登録済み `hash.json` の入力・正解の SHA、固定 source 10 件も照合した。[公式検証の audit](results/fps_round3_validation/judge_audit.json)に選択 53 件と実行・skip の完全な対応を保存した。

文書 268 source を生成し、全 145 library metadata と影響する 43 bundle のコンパイルが成功した。575 ファイルの前後 SHA と固定した 10 source が一致し、生成 Markdown 本文に未 escape の Liquid 開き記号はない。ローカルに Boost がなく、既存の FPS 外の judge source 1 件で属性取得警告がある点は原票に区別した。全属性取得の成功とは扱わない。[文書の結果](results/fps_round3_validation/docs_result.json)、[独立レビュー](results/fps_round3_validation/independent_review.md)、[選択と全ログ](results/fps_round3_validation/)に確認範囲を保存した。

文書生成時、新しい Bostan 部品の `#pragma GCC target` を verify-helper が処理できず停止した。既存部品と同じ `_Pragma` の記法へ揃えた。初回失敗ログを保持し、通常の前処理結果が一致することと、同一パス・同一 flags で生成した object の byte 一致をそれぞれ [前処理](results/fps_round3_validation/docs_pragma_preprocessing.json)・[生成コード](results/fps_round3_validation/docs_pragma_codegen.json) に記録した。初回の固定ハッシュと正規化後のハッシュは別々に保存する。

厳密な前後ハッシュ比較を行う初回単体 runner は、この 1 ファイルの記法変更を検出して `status: failed` を残した。GCC・Clang・Sanitizer の各コマンド自体はすべて成功している。最終 scope audit は、変更がこの正確な正規化だけであること、旧内容へ読み取り専用で置換した依存 plan が元と一致すること、前処理・object 一致と全テスト coverage をすべて検証して `passed` とした。履歴の失敗を成功へ書き換えていない。

正式測定の原票も正規化前の実測 hash を保持した。正規化後の snapshot から、同じパスと flags で依存する線形漸化式のバイナリだけを再ビルドし、byte 一致を確認した。[バイナリ同一性](results/fps_round3_validation/bostan_pragma_normalization.json)に測定原票を変更していないことも記録した。
