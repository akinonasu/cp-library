# 汎用 NTT の結果配列借用

固定基準は `7521ea8703a362aaf3ccd370188fccab86050848`、比較原本は [Library Checker 提出 409465](https://judge.yosupo.jp/submission/409465) の未編集コード。専用 `Product` で既に使っていた結果配列の借用を、奇数 log や一方が密な入力を処理する `ntt::Convolution` にも広げた。従来の所有配列用コンストラクタを保ち、追加した内部コンストラクタと `run_borrowed_as<Plan>` で全長の結果コピーを省く。公開 API は変更しない。

GCC 14.2、Linux x86_64、CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。全版一回の warmup 後、各入力 21 回の順番を shuffle し、process 全体を計時した。コンパイル・正しさ検証は共有 lock、計時は排他 `/tmp/cp-fps-tuning.lock`。すべての計時出力を `checker input actual answer` で検証した。

| 入力 | 基準 ms | 借用 ms | 原本 ms | paired 改善中央値 ms | 改善回数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 65536 × 65536 | 2.920714 | 2.765282 | 2.215943 | 0.157023 | 19/21 |
| 262144 × 262144 | 7.135097 | 6.858434 | 5.667680 | 0.281179 | 21/21 |
| 300000 × 220000 | 7.250349 | 6.993246 | 5.744534 | 0.257344 | 15/21 |
| max_random_00（control） | 12.067280 | 12.051837 | 9.727632 | 0.073480 | 11/21 |
| max_random_01（control） | 11.887592 | 11.790928 | 9.622385 | 0.058467 | 13/21 |

最初の三入力はこの変更の対象となる汎用経路。最大二入力は既存の `Product` 経路を通る control であり、そこに見える変動を汎用借用の効果とは扱わない。中央値の差と paired 中央値も異なる統計量である。確認測定では対象三入力がいずれも改善したため借用版を採用した。比較した split 版の値も原票に残し、採用版へ置き換えていない。

追加コンストラクタは A を所有せず、B と二つの根表だけを確保する。32 byte 境界から `len+16` 個以上の生存中の `Mint` を借り、既存と同じ GCC may-alias SIMD、assembly、`memcpy` で処理する。公開入力は const のままコピーし、予約容量への生存期間外の書き込みや、`Mint` 配列への scalar `uint32_t` 参照を追加しない。返す係数は正規化した Montgomery 表現で、時間 $O(N\log N)$、追加空間 $O(N)$ は変わらない。

独立レビューと拡張 unit は、最小変換長 64、奇数・偶数 log、両辺 sparse、片辺 dense、入力交換・同一入力・零・遅延 Montgomery 値、0 / 32 mod 64 の借用先、末尾 guard / canary と入力 read-only を確認した。GCC 14 Werror、ASan / UBSan、GCC 13 Werror が成功した。既存の全 53 公式入力成功記録についても、private 候補の全 109 source、driver、binary の SHA-256 が一致することを確認して再利用した。

計時の[原票と復元資料](results/fps_convolution_borrow_round4/)に固定基準 archive、patch、原本、source / binary SHA、全試行を保存した。独立した[安全性レビュー・検証記録](results/fps_convolution_generic_borrow_round4/)には拡張 unit、全コマンド・flags・hash、GCC 14 normal / sanitizer / GCC 13 のログ、全 53 入力の再利用根拠がある。

この資料と helper 検証は `7521ea8` を基準にした汎用借用単体の証拠である。非対称畳み込みや、その後の upstream の FastInput 変更を含む production の最終統合検証・全体時間とは分けて記録する。
