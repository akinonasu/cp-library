# 定数係数畳み込みの原票

`screen9.json` は最初の三版・9 回、`confirm21.json` は同じ三本と noinline 版を追加した四版・21 回。warmup、shuffle 順序、全出力の公式 checker、入力・answer・出力・binary SHA、ns 値を保持する。元の三本は変更せず、二段階の絶対値を混ぜて改善率を出していない。

`measure_screen9.py` / `measure_confirm21.py` は各 raw JSON と hash が一致する当時の harness。`unit_screen9.cpp` は最初の raw に記録した unit と byte 同一。採用版の unit は `private_validation/unit.cpp`。

`baseline.tar.gz` は bd29 に既存の汎用借用・非対称畳み込みを統合した freeze tree のコピー。`snapshot.json` は tree 全 source hash。`constant.patch` は inline 版、`constant_noinline.patch` は採用版。原本 409465 は未編集のまま gzip 圧縮し、展開後 hash を metadata に保存した。

`metadata.json` と build JSON は各 binary の compiler 実体 SHA / version、actual `-MM` 全依存、source / binary SHA と実コマンドを持つ。`timing_*.d` は timing flags と同じ条件の actual dependency trace。計時後も source と binary の hash が保存値と一致することを確認した。`screen9_metadata.json` の status は最初の 9 回終了時の履歴であり、採用決定前の記録を変更していない。

`private_validation/reuse_checks.json` は最終 unit を `-std=c++2b -O2 -Wall -Wextra -DDEBUG=1 -I tree`、および ASan / UBSan flags で実行した再利用用の記録。コンパイラの実体 hash / version、actual `-MM` の前後 raw と依存 hash、source / binary / log hash、compile / run 戻り値を持つ。`final_checks.json` は GCC 14 Werror・sanitizer、GCC 13 Werror、Clang fallback、GCC 14 `-U__linux__`・GCC 13・Clang standalone の別七条件の記録。これらの実行でも最終 unit source は同一である。

private の採用 header / dispatch と配置された production は byte 同一、unit も同一 hash で root が配置した。production の最終全体計測・公式全件・依存先検証は別資料であり、この private 原票を後から置き換えない。

`reproduce.py` は artifact hash、基準 source、両 patch、原本・unit を復元する。`--check` / `--sanitize` / `--measure` で新しい作業先に再検証・再計時できる。履歴 harness は当時の絶対 path を残すため、別の場所での再現には `reproduce.py` を使う。
