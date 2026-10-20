# FPS 入力共通化後の統合検証

2026-10-19 UTC。[FPS の追加高速化](fps_round3.md)の実装 `f3ac5ae` を push した後、main に `e59415c`・`f4aa743` の Dijkstra 改善が追加された。前者は FPS の公式ドライバーの実依存を変更しなかった。後者は FPS の一括入力を `components/fast_io_bulk.hpp` へ移し、入力を共通化したため、FPS への影響を別に確認した。この追補は production を変更しない。

## 変更の範囲

`fps_io.hpp` の残りは include の置換だけである。移動した BulkInput/BulkOutput の本体には、large chunk が一度拒否された後、同じ呼び出しの残りを従来の 2 整数・scalar 経路で読むフラグが追加された。拒否された chunk は入力位置や callback を確定しない。境界・桁・uint32 overflow の検査と出力処理は維持されている。

後続コミットの [回帰結果](results/shortest_path_io_validation.json) は GCC 15.2・Clang 21 の `fps_io` と `fast_io_bulk` の通常実行・ASan/UBSan 計 8 件。全ソース hash が統合後の実ファイルと一致し、成功済みの検証は重複実行しなかった。

## バイナリと速度

元の snapshot・48 バイナリ・出力・原票を保全し、同じ source/include パスへ変更した 2 ヘッダーだけを一時的に反映した。同じ compiler・flags で新しい 16 バイナリを別名へ生成し、元の snapshot を復元した。16 本とも SHA が変わったため、コード移動を byte 同一として扱わず、全 16 問を再計測した。

AMD EPYC 9V74、GCC 14.2、CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。ビルドは共有 flock、計時は排他 flock。各版 1 回の予備実行後、21 回の順序を混ぜた。元の高速化版・共通化後・未編集の最速原本を比較し、起動・ファイル入力・計算・出力・終了を含めた。入力は元の総合測定と同じである。

単位は ms、各列は中央値。前回と異なるセッションの値をつないで短縮率を作らない。

| 処理 | f3ac5ae | f4aa743 | 最速原本 |
| --- | ---: | ---: | ---: |
| 積 | 12.071 | 12.194 | 9.714 |
| 逆数 | 11.671 | 11.587 | 10.147 |
| 対数 | 13.503 | 13.627 | 12.342 |
| 指数 | 16.478 | 16.355 | 14.761 |
| 冪 | 24.192 | 24.321 | 22.283 |
| 平方根 | 12.181 | 12.318 | 10.116 |
| 多項式除算 | 6.061 | 5.969 | 4.841 |
| 線形漸化式 | 53.311 | 53.366 | 57.071 |
| 合成（通常） | 2.864 | 2.829 | 2.859 |
| 合成（large） | 34.733 | 34.087 | 32.925 |
| 合成逆元（通常） | 2.777 | 2.772 | 2.816 |
| 合成逆元（large） | 31.275 | 31.653 | 30.561 |
| 多点評価 | 15.238 | 15.141 | 14.671 |
| 補間 | 19.777 | 19.706 | 19.043 |
| Taylor shift | 9.818 | 9.842 | 8.623 |
| 多項式逆元 | 15.470 | 15.428 | 15.004 |

同じ反復の「新版−旧版」の中央値は各問 -0.163〜+0.155 ms と混在した。このセッションでは系統的な退行を確認していないが、すべての入力で速度が等しいことの証明ではない。小さい差を追加の高速化として計上せず、全関数で最速原本へ到達したとも扱わない。

全 1056 実行が `checker input actual answer` の順で成功し、旧新の最終出力 hash は 16 問すべて一致した。元の 97 artifact、新しい 16 バイナリ、両 snapshot、公式データの前後 hash も一致した。元の総合測定は変更していない。

## 原票

[results/fps_round3_upstream/](results/fps_round3_upstream/) に実行済みの `integrate.py`・`time_integration.py`、`integration.json`・`timing.json` とログをそのまま保存した。`manifest.json` は保存物の hash 一覧である。実行時は repo 外へ出力し、バイナリと大きな `.out` は commit していない。

再現には、元の `fps_all_speed.py` で同じ work パスに `f3ac5ae` の測定 snapshot・バイナリを用意し、Bostan pragma の [正規化記録](results/fps_round3_validation/bostan_pragma_normalization.json)と一致させる。作業 tree のコードは `f4aa743` と一致させる。helper の `ROOT`・`WORK`・`OUT` は実測時の絶対パスなので、別環境では必要な変更を明示し、元の hash と変更後の hash を区別する。`integrate.py` は snapshot を一時的に移動するため、同じ work パスを並行して使用しない。
