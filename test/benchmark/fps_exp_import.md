# FPS 指数の入力係数変換

比較元は `144ae32387675fa2a889ea3e8cd19b03ad5016bf`。`native_exp` が入力から微分係数を作る処理を 1 回の Montgomery 乗算にまとめ、専用の内部関数へ切り出した。公開 API、入力保持、短い入力の零拡張、微分済み入力の意味を維持する。

## 変更と参照との差

[最速原本 409662](https://judge.yosupo.jp/submission/409662) は標準表現の入力を読み、次数を掛ける。公開ライブラリの Mint 入力は Montgomery 表現で、従来は `aR → a → i*a` の 2 段階で変換していた。最初の `R` の逆数による Shoup 積と、Montgomery 表現の次数 `iR` による積を、`Montgomery(aR, i)` にまとめた。次数のベクトルも剰余更新から通常の `+8` 更新へ変えた。

公開上限は `n <= 2^24` で、入力の内部語は `2p` 未満、次数は `p` 未満である。積を `T` とすると `T < 2p*n`、REDC の出力は `p + T/2^32 < 2p` なので、`p` による縮約を 1 回行えばよい。64 bit の中間和も上限内に収まる。微分済み入力は従来と同じ逆数による変換を使う。

Mint オブジェクトは 8 個分だけ `memcpy` で読み、端数は個別に処理する。入力に追加の整列・padding 条件はない。変更は GCC/Linux x86_64/AVX2 向けの専用部品内に閉じており、他環境・係数型の経路はそのままである。

## 測定

2026-10-18 UTC、AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。オプションは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。共有ロックでビルド・検証し、`/tmp/cp-fps-tuning.lock` の排他ロックで計時した。

公式 `exp_of_formal_power_series/max_random_00`、50 万項の通常ファイル入出力を使った。各版 1 回の予備実行後、順序をシャッフルして各 21 回測ったプロセス全体の中央値である。各行は独立した測定で、原本 409662 のソースは変更していない。

| 候補 | 比較元 | 候補 | 原本 409662 | 採否 |
| --- | ---: | ---: | ---: | --- |
| 変換を inline で置換 | 17.128455 ms | 17.288904 ms | 15.261101 ms | 不採用 |
| 変換を専用関数へ切り出す | 17.079102 ms | 16.947686 ms | 15.390043 ms | 採用 |
| 同じ切り出し版の別順序での確認 | 17.803812 ms | 17.282925 ms | 15.517533 ms | 確認 |

乱数 seed は各行 `20261022`・`20261024`・`20261025`。予備実行を含む 198 出力すべてが公式 checker を通過し、出力ファイルのハッシュも一致した。採用版は同じバイナリを 2 組で測った。各試行の比較元との差の中央値は −0.273 / −0.122 ms、平均差は −0.231 / −0.315 ms であった。

入力変換だけを、確保と入出力を除く warm buffer で測ると、50 万語で 0.183505 → 0.092471 ms だった。1 プロセス内 64 反復、各 9 試行の中央値である。入力変換の短縮は約 0.091 ms であり、全体の中央値の差 0.13〜0.52 ms すべてをこの乗算削減に帰することはできない。関数の切り出しによる inlining・コード配置の変化や測定変動は分離していない。全体時間の改善方向は 2 組で確認できたが、効果量にはばらつきがあり、最速原本との差も残る。

## 検証

既存の指数・冪専用 unit、二重 include の standalone Werror、追加した `fps_exp_import` unit の通常実行と ASan/UBSan が成功した。追加 unit は微分前・微分済み入力の結果一致、`2p` 未満の非正規化語、短い入力、8 語の端数、入力保持を検査する。読み取り専用ページの末尾に入力を置き、その直後を `PROT_NONE` にしたケースも含む。

追加 unit の Clang 19.1.7 による fallback 構文検査と、GCC の Linux 専用経路を無効にした構文検査も成功した。Sanitizer の条件は `ASAN_OPTIONS=detect_leaks=0`、`UBSAN_OPTIONS=halt_on_error=1`。候補と最終チェックのソースハッシュは検証前後で同一だった。独立レビューでも、数学的な範囲と入力境界に追加の懸念は見つからなかった。

## 原票と再現

[results/fps_exp_import/](results/fps_exp_import/) に全試行の原票、2 候補のパッチ、単体ベンチ、unit の写し、通常・sanitizer・Clang の検証記録を保存した。`inline.json` は不採用形、`outlined.json`・`confirm.json` は同じ採用バイナリの 2 測定、`import.json` は変換単体の原票である。`archive_check.json` に復元と保存物のハッシュ照合を記録した。

原本のキャッシュを用意し、基準コミットから復元する。測定時の一時フォルダやバイナリは不要である。

```sh
python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference
python3 test/benchmark/results/fps_exp_import/reproduce.py \
  --work /tmp/fps-exp-import-reproduction --prepare-only
python3 test/benchmark/results/fps_exp_import/reproduce.py \
  --work /tmp/fps-exp-import-reproduction \
  --reference /tmp/fps-reference/exp_of_formal_power_series/409662.cpp \
  --official-root /tmp/library-checker-problems --verify --measure
```

公式問題の使用コミットは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。新しい環境では同コミットの `library-checker-problems` を用意し、共有ロック下で `generate.py -p exp_of_formal_power_series` を実行する。`--prepare-only` は復元とハッシュ検査だけ、`--verify` は通常・sanitizer 検証、`--measure` は 3 組の全体比較と変換単体の再測定を行う。再実行の結果は作業先の `reproduction.json` へ保存し、原票を上書きしない。今回の保存時には復元だけを検証し、計時は追加していない。
