# FPS 入出力の追加候補（round 4）

2026-10-19 UTC。比較元は `7521ea8703a362aaf3ccd370188fccab86050848`。今回は全候補を不採用とし、production の入出力は変更していない。9 回の探索で見えた短縮を、21 回の確認では複数入力にわたって再現できなかった。

## 条件

AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。flags は全版共通で `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。公開 API の配列生成・確保・入力・計算・出力・終了を含む計測用のコードを埋め込まない driver 全体を通常ファイル入出力で測った。ビルド・単体検証は共有 flock、計時は `/tmp/cp-fps-tuning.lock` の排他 flock。予備実行 1 回の後、版の順序を混ぜた 9 回、候補を絞って 21 回を比較した。

原本は畳み込み [409465](https://judge.yosupo.jp/submission/409465) と対数 [409452](https://judge.yosupo.jp/submission/409452) を一切編集せず使用した。原本の preinit / `_exit` も維持している。公式問題は commit `1814c4e5205517e368bb57a8d1127eb961cfeaae`。全 2,604 回の出力を `checker input actual answer` の順で検証した。原票には warmup を含む全試行、実行順、入力・正解・checker・driver・依存ヘッダー・バイナリ・各出力の SHA-256 がある。

## 不採用候補

| 候補 | 内容 | 判断 |
| --- | --- | --- |
| parser Montgomery fusion | 4 stream の 8 lane 値を作った直後に Montgomery 変換し、後段の変換を除く | 最大入力・対数で退行 |
| split Montgomery fusion | decimal の high/low 部分から Montgomery 値を直接作り、32 bit の `*100000000` 依存を避ける | 9 回で退行 |
| tile4 | 4 step を小さく転置し、16 本の 64 bit scatter を 4 本の 256 bit store にまとめる | 9 回の短縮が 21 回で再現せず、1 桁入力でも退行 |
| output shift | `998244351 = 2^30 - 2^26 - 2^23 - 1` を使い、Montgomery 解除の乗算 2 本を shift/sub に替える | 入力ごとに勝敗が変わり、21 回では退行もある |
| tile4 + output shift | 上記の組合せ | 安定せず |
| range iterator | Montgomery 変換を返す iterator で vector へ直接構築 | 9 回で約 0.5〜1.3 ms 退行 |
| input resize | inline 化済みの Mint を resize で生成し、SIMD で直接書き、append64 の中間コピーを省く | 9 回では全 6 入力で短縮、21 回で再現せず |
| input extra capacity | 入力だけに余分な 2 MiB の capacity を持ち、完全所有する capacity 内の huge page を助言する | minor fault は減るが、21 回の全体時間は安定改善せず |
| resize + extra capacity | 上記の組合せ | 9 回で混在 |

## resize・capacity の確認結果

21 回の中央値、単位 ms。Δ は同じ trial の `候補 − 比較元` を取った対応差の中央値であり、版ごとの中央値の差とは一致しない。負は短縮。

| 問題・入力 | 比較元 | resize | extra | 原本 | resize Δ | extra Δ |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| conv / max_random_00 | 12.352 | 12.406 | 12.509 | 10.016 | +0.061 | +0.208 |
| conv / max_random_01 | 12.438 | 12.386 | 12.452 | 9.880 | +0.222 | +0.025 |
| conv / small_and_large_01 | 9.799 | 9.855 | 10.008 | 8.200 | -0.090 | +0.036 |
| conv / all_same_00 | 13.371 | 13.477 | 13.218 | 8.601 | +0.044 | -0.244 |
| conv / all_same_01 | 12.351 | 12.436 | 12.362 | 9.119 | -0.017 | -0.009 |
| log / max_random_00 | 13.492 | 13.597 | 13.598 | 12.371 | +0.089 | +0.049 |
| log / max_random_01 | 13.530 | 13.717 | 13.622 | 12.360 | +0.062 | -0.018 |
| log / max_all_zero_00 | 13.982 | 14.081 | 14.352 | 12.819 | +0.305 | +0.299 |

extra の探索 9 回では、畳み込み max00 / max01 の対応差が −0.317 / −0.281 ms だった。しかし確認 21 回では +0.208 / +0.025 ms となった。resize も最初の探索では全 6 入力の中央値が短くなったが、この確認では対数や 1 桁入力で退行した。短い探索だけで採用しない。

tile4 / output shift の確認 21 回も [summary.json](results/fps_io_round4/summary.json) と [全試行](results/fps_io_round4/tile_shift_confirm21.json) に残した。たとえば tile4 の conv max00 / max01 / all_same00 の対応差は +0.054 / −0.057 / +0.582 ms、output shift は +0.056 / +0.124 / +0.315 ms。

## 物理メモリ

extra は 131072 word 以上の新規入力で `reserve(n + 524288)` を試す。整数・max_size の範囲外では通常の reserve、追加分の `bad_alloc` でも通常の `reserve(n)` へ戻る。HUGEPAGE は capacity 内に完全に収まる 2 MiB 整列区間だけ、POPULATE_WRITE は従来どおり実際に使う範囲内の完全な 4 KiB ページだけに限定した。助言が失敗しても無視する。元の結果側 prepare は変更していない。

THP は `always [madvise] never`。時間比較の `wait4` による maxrss は、exec 前の Python の RSS が下限になる試行がある。そのため別途、小さい native launcher から fork して未編集 driver を実行し、その子だけの `wait4` を 9 回測った。これは物理メモリ確認用で、上の全体時間の順位表へ混ぜていない。単位は RSS が KiB、fault が回数、各列は 9 回の中央値。

| 入力 | 比較元 peak RSS | extra peak RSS | 比較元 minor fault | extra minor fault |
| --- | ---: | ---: | ---: | ---: |
| conv / max_random_00 | 26200 | 28192 | 2314 | 1789 |
| conv / small_and_large_01 | 19032 | 20076 | 1708 | 1447 |
| log / max_random_00 | 18896 | 19916 | 1658 | 1437 |

major fault は全試行 0。ページフォールトの減少と約 1〜2 MiB の peak RSS 増加を確認したが、全体時間の安定した短縮には結び付かなかった。差の全額を API や一つの処理に帰属させない。

## 安全性と再現

tile4・output shift の組合せ・resize・extra を対象に、通常と ASan/UBSan で、既存 FPS I/O・一般 bulk I/O、guard page 直前の入力、FILE / pipe / string_view、不規則な空白、uint32 上限、records、入力位置、既存出力 prefix の保持と異常 chunk の callback 非実行を確認した。extra には threshold 前後の大きな入力、追加確保の例外を注入した通常確保への fallback、max_size 超過も確認した。output shift は 200 万 lane を既存の `Mint::val()` と比較した。tile4 の順序と範囲は別担当も read-only で確認した。これらは候補の正しさの検証で、採用したことを意味しない。

[results/fps_io_round4/](results/fps_io_round4/) に原票 8 本、検証記録 4 本、相対 patch 9 本、実際の計測ハーネス 2 世代、memory launcher、専用テストと再現スクリプトを保存した。全 30 件の再構築ヘッダー SHA-256 照合も成功した。大きな生成出力とバイナリは commit せず、実ファイルは `/workspace/cp-fps-round4/io` に保持している。各出力の SHA は原票にあり、再実行で出力と checker 検証を再生成できる。

```sh
python3 test/benchmark/results/fps_io_round4/reproduce.py --prepare-only
python3 test/benchmark/results/fps_io_round4/reproduce.py --verify
python3 test/benchmark/results/fps_io_round4/reproduce.py --measure --memory
```

計時には記録と同じ公式入力・正解・checker と未編集原本の cache が必要。既定位置は `/tmp/library-checker-problems` と `/tmp/fps-reference` で、CLI から変更できる。原本は `python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference` で取得する。公式 repository は上記 commit を用い、対象問題の generate.py と checker を用意し、再現スクリプトの SHA 検査に通ることを確認する。移動した作業パスは `__FILE__` 等によりバイナリの byte identity に影響するため、再現スクリプトはソース・fixture の一致と出力 checker を確認し、別パスのバイナリ一致は要求しない。

`--experiment` で探索・確認のいずれかを選べる。`--memory` は baseline / input_extra_capacity / reference を含む実験と併用する。再実行した時間は新しい原票として作業 directory に記録し、保存済みの測定結果を上書きしない。
