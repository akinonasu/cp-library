# 統合版 sqrt の単独再確認

正式16問の sqrt `max_random_01.in` は、個別の中央値で12.237523→12.391363ms、paired 差で−0.013520ms・10/21勝という小幅な差だった。持続的な退行とばらつきを区別するため、同じ最終 before/current/reference の3バイナリ・同じ入力で21回だけ追加測定した。追加ビルド、製品ソースの変更、他の入力への拡大は行っていない。

| 測定 | before ms | current ms | 参照原本 ms | paired 差 ms | current 勝数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 正式16問の初回 | 12.237523 | 12.391363 | 10.108820 | −0.013520 | 10/21 |
| 追加21回 | 13.467505 | 13.239099 | 11.198106 | +0.193720 | 18/21 |

追加試行で初回の退行は再現しなかった。時点によって全版の絶対時間が変わるため、異なる測定間の絶対値ではなく、それぞれの測定内の before/current を比較する。初回の値も正式原票にそのまま残した。

AMD EPYC 9V74 / Linux x86_64 / GCC14.2.0。公式検証の共有ロックが解放された後、CPU2固定・`/tmp/cp-fps-tuning.lock` の排他ロックで実行した。flags は3版とも正式測定の `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。assert は有効。各版1回予備実行後、seed 20261108 で反復ごとの順序をシャッフルし21回測定した。

時間は起動・ファイル入出力・計算・終了を含む wall time の中央値。paired 差は同じ反復の before−current の中央値で、個別の中央値の差とは異なる。計時後の `checker input actual answer` は全66回成功した。バイナリ、ドライバー、header snapshot、入力、公式出力、checker の前後ハッシュが一致した。

原票の `parent_raw.sqrt_default_row` に初回の全値を関連付け、元の正式 JSON は編集していない。SHA は `360af00d2182ce18c0c963a537220534d877c823b43a3fbf3d32923c9a5a1237` のまま。snapshot の無関係な Bostan pragma の変更は、[別のバイト同一性記録](../fps_round3_validation/bostan_pragma_normalization.md) で関連付けた。

`*.py` は実行したハーネス、`*.json` は全試行・順序・checker・ハッシュ・親測定との関係、`*.log` は結果。元と同じ3バイナリと snapshot の絶対パスを用意し、別の `--output` を指定して再実行できる。
