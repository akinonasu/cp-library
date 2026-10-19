# 正式測定後の pragma 表記統一

正式16問の測定が完了し、計時の排他ロックが解放された後に実行した。製品ソースは変更していない。

測定用 `current/components/fps_bostan.hpp` の `#pragma GCC target("avx2")` を、製品側と同じ `_Pragma("GCC target(\"avx2\")")` へ統一した。差はその1行のみ。履歴のヘッダーは `bostan_pragma_before.source`、統一後は `bostan_pragma_canonical.source` として保存した。

- 測定時の header SHA: `bbe4528ee226bcf0ea90d143f0b5832ef6f69b2162b448ec845d249349bc7be3`
- 統一後の header SHA: `bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258`

同じコンパイラ・全 flags・ソースの絶対パス・出力先で、影響する `kth_term_of_linearly_recurrent_sequence_current` だけを共有ロックで再ビルドした。保存した測定時バイナリとの `cmp --silent` は0で、バイト列も直接比較して一致した。

両バイナリの SHA-256 は `ad9485dc5f0dbf22f0a6777eb22cfcd9b0d86fc3d812dda9af518c0ba3c4bb1c`。他のコピー済みヘッダーは全件不変で、追加の計時は行っていない。

元の `fps_round3_all_speed.json` は編集せず、歴史的な header SHA と測定値をそのまま残した。前後の JSON SHA は `360af00d2182ce18c0c963a537220534d877c823b43a3fbf3d32923c9a5a1237` で一致する。統一後の snapshot の header 一覧は、元の記録と分けて `bostan_pragma_normalization.json` の `normalized_sources_current` へ保存した。

`bostan_pragma_normalization.py` は実際の処理、JSON は argv・ハッシュ・比較結果、ログは正式測定完了待ちと実行結果を記録する。再実行する場合は測定時の未統一 snapshot と記録した同じ絶対パスが必要である。
