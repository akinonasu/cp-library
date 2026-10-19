# 統合版 exp の262145項

正式16問の測定後、完成済みの `before` / `current` / 編集していない最速原本の3バイナリを追加ビルドせず使った。入力は公式 `near_262144_02.in`、262145項。製品ソースと正式測定の原票は変更していない。

| 版 | 全体時間の中央値 ms |
| --- | ---: |
| before (`bd951dc`) | 13.566695 |
| current（最終統合版） | 10.235058 |
| 原本 [409662](https://judge.yosupo.jp/submission/409662) | 12.352805 |

同じ反復の before−current の中央値は **+3.304306ms**、current の勝数は **21/21**。ローカルの同条件で、current は最速原本も上回った。この結果は judge 上の fastest 達成を示すものではない。

AMD EPYC 9V74 / Linux x86_64 / GCC14.2.0、CPU2固定、排他ロック `/tmp/cp-fps-tuning.lock`。flags は `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。assert を有効にした。各版1回予備実行後に seed 20261107 で反復ごとの順序をシャッフルし21回測定した。起動・ファイル入出力・計算・終了を含む wall time。各 checker は計時後に `checker input actual answer` の順で実行し、全66回成功した。

3バイナリ・ドライバー・ヘッダー snapshot・入力・公式出力・checker の SHA を前後で照合した。Bostan の無関係な pragma 表記だけは [別の同一性記録](../fps_round3_validation/bostan_pragma_normalization.md) により、snapshot の履歴値と統一後を関連付けた。元の正式測定 JSON の SHA は `360af00d2182ce18c0c963a537220534d877c823b43a3fbf3d32923c9a5a1237` のまま。

`exp_262145_recheck.py` は実行したハーネス、JSON は親測定との関連・全試行・順序・checker・ハッシュ、ログは結果を記録する。元と同じバイナリと snapshot の絶対パスを用意して再実行できる。再実行による時刻やバイナリの変化を元の原票へ上書きしないよう、別の `--output` を指定する。
