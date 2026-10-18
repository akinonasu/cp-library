# Bostan–Mori / 線形漸化式

基準は `ce81108fe8e4f3a93d816c94e475d02ef0329608`。共通 FPS/NTT/入出力を基準のまま固定し、`fps/bostan_mori.hpp` だけを交換した比較です。他の FPS 高速化を含む main 全体の比較ではありません。

参照は Library Checker [405848](https://judge.yosupo.jp/submission/405848)、掲載実行時間 68 ms。ソース SHA-256 は `f6d088ba82caad15a517bffc2396f4a1d81dd75ea98ed0992226c4fa0b847ff2`。[400681](https://judge.yosupo.jp/submission/400681) のソースも同一でした。judge の時間と以下のローカル時間は比較できません。

## 不足していた点と採用内容

NTT doubling による Bostan–Mori、分子の 1/2 補正の遅延、分母が変換長に一致する場合の最高次係数補正は、基準版にも入っていました。最速版は変換配列を整列 allocator で確保しており、基準版は通常の `std::vector<Mint>` でした。

変換中の分子・分母だけを既存の `allocate_fps_words` で確保するようにしました。64 バイト整列と、2 MiB 以上での Linux huge page 要求を共用します。確保失敗時の通常の整列確保への fallback も既存部品のものです。これは整列だけを単独で分離した測定ではなく、確保方法・初期化・終端コピーを含む変更全体の測定です。

生の作業領域には実際の `unsigned int` 配列の生存期間があり、使用範囲を初期化してから変換します。`Mint` の存在しない領域を `Mint*` として参照しません。通常の係数演算へ戻す終端では、生存中の係数 vector へコピーします。公開シグネチャ、一般の係数型からの変換、任意の非零分母定数項、仮分数、空の分子、非 AVX2 環境の処理を維持します。`linear_recurrence.hpp` 自体は変更していません。

最速版には、さらに 4096 項タイル化、stride 4/1 専用 radix-4、逆 NTT 終段と twist 積の融合があります。既存 NTT に終段融合と 2048 項タイルを追加した候補では改善がなく、専用変換コードは本体へ入れていません。最速版の専用 NTT 全体を移植したという結果ではありません。

## 最終測定

2026-10-19 JST、Linux x86_64 / AMD EPYC 9V74 / GCC 14.2.0、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。既存ヘッダーの GCC O3 pragma と assert は有効です。

公式入力を通常のファイルとして与え、出力は pipe で回収します。各ケース・各バイナリを 1 回 warmup 後、順序を shuffle して新しい process を 11 回実行した実時間の中央値です。入力・計算・出力・確保・解放・process 起動を含み、checker は含みません。計時中は他のコンパイル・検証を同じ lock で排除します。すべての計時出力を公式解答と照合します。

| 公式ケース / 呼び出し | 基準 ms | 採用版 ms | 最速提出原本 ms |
| --- | ---: | ---: | ---: |
| max_random_00 / `bostan_mori` | 58.489 | 57.306 | 57.995 |
| max_random_00 / `linear_recurrence` | 59.129 | 57.700 | 57.995 |
| near_65536_00 / `bostan_mori` | 30.922 | 30.785 | 30.073 |
| near_65536_00 / `linear_recurrence` | 32.196 | 30.794 | 30.073 |

最大ケースでは最速原本と同程度でした。65535 次のケースは原本より約 2.4% 遅く、最速を全ケースで上回ったとは言えません。near ケースには 29〜39 ms のばらつきがあり、僅差の改善を断定しません。生の全試行、入力 hash、候補ソース hash、バイナリ hash、コンパイラ条件は [fps_bostan_speed.json](results/fps_bostan_speed.json) に記録しています。

## 不採用候補

以下はそれぞれ別の探索測定（warmup 1 回・9 試行）で、行をまたぐ時間差は比較しません。入力は `max_random_00` と `near_65536_00`、呼び出しは `linear_recurrence` です。

| 候補 | max: 基準 → 候補 ms | near: 基準 → 候補 ms | 判断 |
| --- | ---: | ---: | --- |
| 終段融合のみ・O3 | 58.321 → 58.304 | 29.797 → 29.720 | 有意な改善なし |
| 終段融合＋2048 項タイル・O3 | 58.611 → 59.150 | 30.163 → 30.498 | 不採用 |
| aligned workspace → 最小 2 MiB 確保 | 59.405 → 59.001 | 30.647 → 30.725 | near で改善せず、余分な確保を避けて不採用 |

最初の新規ヘッダーで既存 NTT と O3 指定が揃っていなかった候補は、max で終段融合 72.157 ms、タイル併用 120.750 ms に退行しました。O3 を揃えると退行が消えたため、この差を手法の性能差とは扱いません。探索時は事前の全ソース hash を保存しておらず、この探索記録は最終版の証拠の代わりには使いません。全探索値と後から採取したバイナリ hash は [fps_bostan_screen.json](results/fps_bostan_screen.json)、不採用変換コードは [fps_bostan_transform_candidate.hpp](fps_bostan_transform_candidate.hpp) に残しています。

## 検証と再現

採用版の両公式ドライバは、公式 20 ケースずつ、合計 40 回の checker 検証に成功しました。新しい `fps_bostan_workspace.cpp` は次数 31〜100000、2 冪の前後、`q[0]=7`、`LLONG_MAX` を含む項番号を、独立に計算できる `p(x)/(7(1-cx^d))` の係数と比較します。通常実行と ASan/UBSan（`ASAN_OPTIONS=detect_leaks=0`、`UBSAN_OPTIONS=halt_on_error=1`）が成功しました。漏洩検査の成功を示すものではありません。既存の `fps_algorithms.cpp` と `fps_tuning_advanced.cpp` も、共通実装を基準に固定した採用版で通常実行に成功しました。

```sh
python3 test/benchmark/fps_bostan_speed.py \
  --reference /tmp/fps-reference/kth_term_of_linearly_recurrent_sequence/405848.cpp \
  --official-root /tmp/library-checker-problems/other/kth_term_of_linearly_recurrent_sequence \
  --output /tmp/fps_bostan_speed.json
```

`--experimental fused_o3` または `--experimental tiled_o3` を追加すると、基準の共通実装に不採用候補を組み込んで比較します。探索初版の O2 境界は `fused_o2` / `tiled_o2` で再現できます。候補の字句や未使用メソッドまで探索初版と同一であることを保証するものではありません。
