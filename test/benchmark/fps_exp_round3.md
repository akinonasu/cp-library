# FPS exp / pow の追加比較

基準は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。採用したのは、`native_exp` の次数が $n=2^k+\delta$、$2^k\ge4096$、$1\le\delta\le4$ の場合に、$2^k$ 項を従来の本体で計算して残りだけ漸化式で補う変更である。公開 API、入力保持、短い入力、微分済み入力、AVX2 の実行時判定を維持する。log と pow の計算経路には変更を採用していない。

[参照 409662](https://judge.yosupo.jp/submission/409662) の指数は、この境界でも Newton の更新をもう一段行う。$q=xf'$ とすれば $i g_i=\sum_{j=1}^{i}q_jg_{i-j}$ なので、少数項だけなら線形時間の内積で補える。微分済み入力では入力が既に $q$ である。追加部分は $O(n\delta)$ 時間、$O(1)$ 追加空間で、全体の $O(n\log n)$ 時間・$O(n)$ 追加空間は変わらない。通常サイズの本体は `native_exp_regular` にそのまま残した。

AVX2 内積は入力と構築済みの出力を 8 項ずつ `memcpy` で読み、添字を掛けた Montgomery 語と逆順の出力を積算する。各入力を $p$ 未満へ縮約し、各 64 bit lane に最大 8 積を集める。$8p^2+2^{32}p<2^{64}$ なので Montgomery 縮約の補正和まで溢れず、縮約結果は $3p$ 未満になる。横和は毎回 $2p$ で縮約し、加算前の上限を $4p$ 未満に保つ。完全な 8 項だけを読み、端数は `Mint` の演算で処理する。入力の余分な padding・整列は要求しない。native 入出力の非重複条件と、raw `exp` のインプレース経路は変えていない。別担当による数学・読み取り境界の独立レビューも行った。

2026-10-19 UTC、AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。オプションは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。ビルド・検証は `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロック下で行った。公式の通常ファイルを読み、別の通常ファイルへ書くプロセス全体を測定した。各版 1 回の予備実行後、順序をシャッフルして各 21 回測った。各出力は公式 checker で検証した。原本ソースは変更していない。

| 公式 exp 入力 | 基準 | 採用候補 | 原本 409662 |
| --- | ---: | ---: | ---: |
| max_random_00、500000 項 | 17.386 ms | 17.546 ms | 15.612 ms |
| max_random_04、500000 項 | 17.212 ms | 17.122 ms | 15.311 ms |
| near_262144_01、262144 項 | 9.228 ms | 9.162 ms | 7.929 ms |
| near_262144_02、262145 項 | 12.742 ms | **9.672 ms** | 11.591 ms |

262145 項では全 21 組で候補が速く、組ごとの差の中央値は −2.999 ms、平均は −3.090 ms だった。通常サイズでは中央値の大小が混在し、max_random_00 の組ごとの差は中央値 +0.061 ms・平均 +0.019 ms、max_random_04 は −0.200 / −0.133 ms だった。500000 項の短縮は主張しない。元の本体と分離後の本体は 12376 byte・2527 命令で、call/jump のアドレスと RIP 定数位置を正規化した命令列の SHA-256 も一致した。通常サイズの本体に演算を加えず、境界の改善を採用した。これは当環境の局所比較であり、judge の記録を更新したという意味ではない。

同じ処理を元の本体内へ直接分岐させた初期形では、500000 項の 2 入力に +0.154 / +0.052 ms の中央値差が出たため、計算本体を分離する形へ直した。初期形も原票に残している。

AVX512 の積分処理も試したが採用しなかった。単純な 16 lane 化の 64/128 積列版と、$2^{52}$ を基数とする IFMA Montgomery 版を実装し、公式 4 入力で各 15 回比較した。IFMA 版は max_random_00 が 17.353 → 17.352 ms だった一方、262143 項は 8.865 → 9.047 ms、262145 項は 12.209 → 12.405 ms となり、一貫した改善がなかった。AVX512 の有無に関係する変更は production に含めていない。

pow は、微分の添字を初めから $e iR$ とし、線形の blocked recurrence 全体で $e f'/f$ を作って後段の $e$ 倍を省く AVX2 候補を実装した。既存 `log_derivative_impl` を専用入力 view と ADL の微分処理で再利用し、変換の再利用も係数 1 に変えた。$e=0$、短入力、raw の alias 順序について独立レビューし、既存 unit も通過した。[原本 409718](https://judge.yosupo.jp/submission/409718) と同条件で比較したが、最大 2 入力の再確認で優劣が入れ替わった。変換補助を関数へ切り出して係数 1 をテンプレート化した案も、最終比較で最大ランダム 2 入力は短縮する一方、binary_exp_max は 24.789 → 24.972 ms となった。安定した全体改善を確認できず、どちらも採用しなかった。

検証では既存の指数・冪 unit に加え、独立した `Mint` の和との比較、lazy な入力語、入力保持、8 lane と 64 項の前後、guard page 直前の入力・出力を確認した。通常および ASan/UBSan で成功した。採用形では 262144 / 262145 / 262148 / 262149 項、小さい次数、次数を超える入力と短い入力、微分済み入力、既存の出力要素も追加確認した。GCC の単独ヘッダー、Linux マクロを外した経路、Clang 19 の単独ヘッダーも検査した。各条件と終了コードは原票に残す。

原票・パッチ・unit の写し・再現スクリプトは [results/fps_exp_round3/](results/fps_exp_round3/) にある。`whole_exp_tail_outlined.json` が採用候補の測定、`whole_exp_tail.json` は初期形、`whole*.json` の残りは不採用候補である。`core_assembly.json` と `assembly.py` は通常本体のコード生成照合を記録する。`metadata.json` に全候補・原本のハッシュ、コンパイル条件、独立レビューを記録し、`archive_check.json` に保存物の復元照合を残す。

```sh
python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference
python3 test/benchmark/results/fps_exp_round3/reproduce.py \
  --work /tmp/fps-exp-round3-reproduction --prepare-only
python3 test/benchmark/results/fps_exp_round3/reproduce.py \
  --work /tmp/fps-exp-round3-reproduction --verify --measure \
  --experiment tail-outlined --reference-cache /tmp/fps-reference \
  --official-root /tmp/library-checker-problems
python3 test/benchmark/results/fps_exp_round3/assembly.py \
  --work /tmp/fps-exp-round3-reproduction
```

公式問題のコミットは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。新しい環境では同コミットの問題を用意し、共有ロック下で `generate.py -p exp_of_formal_power_series` と `generate.py -p pow_of_formal_power_series` を実行する。`--experiment` で不採用候補も選択でき、当時と同じ入力ハッシュ・試行順を検査して再測定する。再実行結果は作業先へ保存し、原票を上書きしない。保存時には全 8 版のヘッダーを基準コミットとパッチから復元し、全 SHA-256 の一致を確認した。
