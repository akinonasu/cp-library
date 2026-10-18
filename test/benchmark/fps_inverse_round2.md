# 逆数・平方根の出力処理改善（round 2）

`native_inverse` の Newton 近似を最初から `C = R² mod P` 倍で保持し、出力時の倍率処理を除去した。公式最大入力2件の99回比較で、プロセス全体の中央値が0.99%、1.58%改善した。平方根の出力コピー候補は入力によって差が安定せず、採用しなかった。

## 変更と正しさ

`P = 998244353`、`R = 2^32 mod P`、`C = 932051910`、`C^-1 = 944319265` とする。入力の Mint word を係数とした級数を `F`、その逆数を `h` とすると、出力に必要な Mint word は `C h` になる。従来は `h` を求めてから全係数へ `C` を乗じていた。

新実装は最初の最大32係数だけを `C` 倍する。各段で `current = C h` を保つと、1回目の積の上半分は `C` 倍、続く積は `C²` 倍になる。2回目の既存 `cyclic_product` の係数を `-C^-1 mod P = 53925088` に変更すれば、次の上半分も `C` 倍のまま求まる。追加のNTTはなく、出力全項を `append_fps_words<true>` で追加できる。

中間段の scratch 分割と最終段で入力領域を作業領域へ再利用する順序は従来と同じ。入力の最終読取りより後にだけ再利用し、各最上段の reduce が出力を `P` 未満に保つ。public API、入力 const、Mint の object lifetime を維持した。共通 `fps_blocked.hpp`、I/O、output は変更していない。

## 条件

固定基準は `144ae32387675fa2a889ea3e8cd19b03ad5016bf`。GCC 14.2.0、CPU 2固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert有効。全版をbuild後、compile・検証はshared lock、計時はexclusive `/tmp/cp-fps-tuning.lock` で分離した。各ケース1回warmup後、seed固定のshuffled順でkernel・参照比較を21回、安定性比較を99回測った。

whole は同一の公式入力ファイル・通常出力ファイルを使い、すべての計時結果を実際の公式 `checker input actual_output official_answer` で検証した。平方根の `max_random_00` と `near_262144_00` は公式解が `-1` のため、採用判断には正ケース `_01`・`_02` を使った。

## whole：99回中央値（ms）

| 問題・公式入力 | 基準 | live batch | scaled inverse | scaled変化 |
|---|---:|---:|---:|---:|
| inv max_random_00 | 12.803688 | 12.731701 | 12.677419 | -0.99% |
| inv max_random_01 | 13.138057 | 13.031918 | 12.930367 | -1.58% |

対応する各trialの「基準−scaled」の中央値は、max_random_00 が +0.100760 ms、max_random_01 が +0.198156 ms。二項順位による中央値の95%区間はそれぞれ [+0.042433, +0.220909]、[+0.017656, +0.326497] ms で、両入力とも正だった。詳細は [stability.json](results/fps_inverse_round2/stability.json)。

| 平方根の公式正ケース | 基準 | live batch | 変化 | 対応差中央値の95%区間（ms） |
|---|---:|---:|---:|---:|
| max_random_01 | 13.015614 | 12.959130 | -0.43% | [-0.107520, +0.094331] |
| max_random_02 | 12.958208 | 12.755126 | -1.57% | [+0.124126, +0.310863] |

live batch は `std::array<Mint,64>` へ直接SIMD結果をコピーして中間バッファを除去する候補。append単独では約0.175→0.093 msまで短縮したが、平方根max_random_01は99回で53勝、対応差の区間が0を跨いだため不採用。逆数の最終上半分だけのscale融合と、そのlive batch合成も21回で追加効果が安定せず見送った。

## inverse kernel：21回中央値（ms）

| n | 基準 | live batch | scaled inverse | 参照409370 |
|---:|---:|---:|---:|---:|
| 262144 | 3.497985 | 3.453678 | 3.473629 | 3.317285 |
| 262145 | 3.913654 | 3.874386 | 3.843910 | 6.889421 |
| 393217 | 7.161116 | 7.028138 | 7.109218 | 6.884614 |
| 500000 | 7.299983 | 7.172873 | 7.208206 | 6.874048 |

FPS kernelはpublic const APIを呼び、Mint入力・出力変換を含む。参照inverse kernelは提出と同じ最終段の領域再利用を使い、canonical入力のコピーを計時する。参照sqrt kernelはcanonical入力に `poly::sqrt` を呼ぶ。全版で入力生成と結果hashは除外し、arena・transform・出力確保は含む。結果hashは各kernelケースで一致した。

## inverse whole：参照との同一21回列（ms）

| 公式入力 | 基準 | scaled inverse | 参照409370 |
|---|---:|---:|---:|
| max_random_00 | 12.848375 | 12.956286 | 11.045318 |
| max_random_01 | 13.217885 | 13.018388 | 11.150444 |

追加21回のmax_random_00は単独中央値では0.108 ms悪化した。この21回の対応差中央値は +0.028964 msで、95%区間は0を跨ぐ。同一binary hashを確認した99+21回の対応差120件を補足としてまとめると、max_random_00は中央値 +0.0985865 ms・95%区間 [+0.031377, +0.210664]、max_random_01は +0.216007 ms・[+0.047141, +0.359715] msだった。測定群ごとの揺れも含め、元原票を分けたまま [pooled-stability.json](results/fps_inverse_round2/pooled-stability.json) に保存した。

参照は [inverse 409370](https://judge.yosupo.jp/submission/409370) と [sqrt 409728](https://judge.yosupo.jp/submission/409728)。両方の未改変source、source/binary hash、全raw sampleと実行順を保存した。平方根の同一21回参照比較は [fused_results.json](results/fps_inverse_round2/fused_results.json) にある。

## 検証・再現

既存inv/sqrt public unit、短入力・出力prefix・非1定数項・SIMD/tail境界・read-only guard page上のpaddingなし入力の比較を実施した。scaled候補はlow-level canonical逆数とのraw word一致、通常・ASan/UBSanを通過し、公式inv全25ケースも通過した。live batchと最終段候補も個別unitとsanitizerを通過している。採用後の追加public unit検証は [integrated-checks.json](results/fps_inverse_round2/integrated-checks.json)、表現のproofと検証原票は [scaled-checks.json](results/fps_inverse_round2/scaled-checks.json) に保存。

[原票・再現手順](results/fps_inverse_round2/README.txt) に従い、`git archive 144ae323` と各独立patchで5版を再構成できる。全174headerのSHA-256、両参照kernel source、公式入力hashを再構成時に照合する。全snapshotやbinaryは保存していない。
