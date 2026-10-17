# FPS の調整

2026-10-18 JST。比較元は `fd41ae5df33960b93288ae98f9edac4e9fdb166f`。公開 API を維持し、Linux x86_64 / GCC を優先して調整した。Mac は動作互換を確認する。

## 採用した変更

- 基本演算は隣接する窓の NTT を共有し、最大 7 項の積和をまとめる。2 冪の直後では小さい区間幅を使う。`pow` は対数の積分と指数の微分を相殺する。多項式除算は既知の高次係数を利用し、余りの巡回畳み込みの長さを短縮する。短い次数を指定した級数除算では不要な入力を除く。
- 多項式 GCD・拡張 GCD・逆元は、必要な次数まで進む Half-GCD の `jump` を使う。小区間の fraction-free 処理、変換のキャッシュ、最後の行列の上段だけを計算する処理を組み合わせる。
- Taylor shift の階乗・逆階乗を 32 本の積の列で生成する。AVX2 がない場合はスカラー処理を使う。
- 合成は内側の valuation に合わせて外側を切り詰める。合成と Power Projection は指数の最大公約数で圧縮する。線形漸化式は末尾の零係数を除き、その次数に対応する末尾の初期値へ縮小する。
- FPS の検証用プログラムを既存の FastInput / FastOutput に統一した。冪・平方根・多項式除算・大規模合成・大規模合成逆元の公式テストを追加した。

個別の候補、採否、検証内容は [基本演算](fps_tuning_basic.md)、[合成・漸化式](fps_tuning_advanced.md)、[多項式演算](fps_tuning_polynomial.md) にある。共通 NTT 自体への変更は残していない。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、コンパイル・検証・測定を `/tmp/cp-fps-tuning.lock` で排他した。フラグは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=native`。assert は有効。各版を新しいプロセスで実行し、ウォームアップ 1 回の後、固定 seed で順序をシャッフルして 7 回測定した。以下は中央値、単位 ms。

比較元のヘッダーは `git archive`、変更後は測定開始時のコピーを使う。入力・実行ファイル・出力は Linux の `/tmp` に置く。計算部分は入力生成とチェックサムを除き、API 呼出し・確保・結果代入を含む。プロセス全体の測定は起動・通常ファイルの入出力・終了を含み、公式 checker とハッシュ計算は含めない。

計算部分は全結果のチェックサムを照合する。公式入力は各問題の大きい 2 ファイルを選び、平方根が存在しない入力・冪の先頭係数が零となる入力は速度比較から除いた。全版の出力を公式 checker で確認する。全公式ケースの正しさの検証は、この速度比較と別に行う。

これは AtCoder・Library Checker の実機測定ではない。上位提出の掲載時間とローカル時間を直接比較せず、同じローカル入力・環境で再コンパイルした値を比べる。実機での最速性、全入力での改善は主張しない。

## 計算部分

| 操作 | 係数数 | 変更前 | 変更後 |
| --- | ---: | ---: | ---: |
| 畳み込み | 524,288 | 10.570 | 10.628 |
| 平方 | 524,288 | 9.272 | 9.340 |
| 逆元 | 524,288 | 10.252 | 10.387 |
| 対数 | 524,288 | 16.933 | 16.719 |
| 指数 | 524,288 | 18.948 | 19.165 |
| 冪 | 524,288 | 35.967 | 35.148 |
| 平方根 | 524,288 | 11.033 | 11.149 |
| 級数除算 | 524,288 | 14.473 | 14.361 |
| 多項式除算 | 524,288 | 14.520 | 14.267 |
| 合成 | 131,072 | 46.348 | 46.658 |
| 合成逆元 | 131,072 | 41.077 | 40.488 |
| Power Projection | 131,072 | 33.800 | 33.984 |
| Bostan–Mori | 131,072 | 46.808 | 46.882 |
| 線形漸化式 | 131,072 | 47.723 | 47.889 |
| 多点評価 | 131,072 | 30.893 | 30.662 |
| 補間 | 131,072 | 44.545 | 44.539 |
| Taylor shift | 524,288 | 16.278 | 14.754 |
| 多項式 GCD | 50,000 | 55.926 | 30.475 |
| 多項式逆元 | 50,000 | 75.166 | 30.686 |

大きい密な入力では基本演算の変化は小さい。畳み込み・逆元・Bostan–Mori・多点評価・補間の本体を新しい方式に置き換えたわけではない。共有処理が変わったものも含めて測定し、改善しなかった値も掲載した。

2 冪以外の長さや構造のある入力には、別の効果がある。個別測定では多項式除算 500,000 係数が 13.432 → 10.571 ms、指数が 4 の倍数の合成 131,072 係数が 45.382 → 9.867 ms、同様の重み付き Projection が 33.677 → 7.236 ms、実効次数 8 の漸化式が 5.923 → 0.066 ms となった。個別測定は共有依存先を揃えて各変更の効果を比較しており、上の commit 全体の比較とは条件が異なる。

## 上位提出とプロセス全体の比較

参照提出は各問題の時間順上位 AC から取得した。[参照一覧](results/fps_tuning_reference.json) に取得日時・URL・掲載時間・ソース SHA-256 を保存した。GCD 単体の参照は取得できなかったため、多項式逆元の Half-GCD を調査し、GCD は計算部分と単体テストで比較した。

`before` と `current` は同じ変更後の FastIO ドライバーを使い、ヘッダーの差だけを比べる。`before_stdio` は比較元のドライバーをそのまま使い、入出力変更を含む差を示す。既存ドライバーがなかった問題には `before_stdio` はない。参照提出は変更せずコンパイルする。

この節の `power_projection` は Projection を使って大規模合成逆元問題を解くドライバーであり、単体 API の比較ではない。Projection 単体は前節の計算部分と個別資料で測定している。実行中はハーネスとドライバーも固定し、記録した SHA-256 を維持する。

参照には専用の SIMD 入出力、固定桁の係数出力、早期実行と `_Exit` を使うものがある。入出力単体の比較は [fps_io_probe.md](fps_io_probe.md) に分けた。段階を分けて測った時間を全体の時間から引いて、厳密な NTT 時間を推定する用途には使わない。

| 操作・公式入力 | 旧 stdio | 旧 FastIO | 変更後 | 参照提出 |
| --- | ---: | ---: | ---: | ---: |
| multiply / `all_same_01` | 63.060 | 30.061 | 29.846 | [409465](https://judge.yosupo.jp/submission/409465) 8.937 |
| inv / `max_random_00` | 35.563 | 19.528 | 19.891 | [409370](https://judge.yosupo.jp/submission/409370) 9.488 |
| log / `max_random_00` | 42.300 | 26.479 | 26.716 | [409452](https://judge.yosupo.jp/submission/409452) 11.719 |
| exp / `max_random_00` | 44.109 | 27.012 | 26.366 | [409662](https://judge.yosupo.jp/submission/409662) 13.390 |
| pow / `binary_exp_max_00` | — | 43.840 | 41.713 | [409718](https://judge.yosupo.jp/submission/409718) 21.262 |
| sqrt / `max_random_01` | — | 20.498 | 20.691 | [409728](https://judge.yosupo.jp/submission/409728) 9.115 |
| divmod / `q0_equals_zero_02` | — | 17.969 | 18.722 | [409561](https://judge.yosupo.jp/submission/409561) 3.700 |
| compose / `max_random_01` | 3.201 | 2.784 | 2.682 | [409336](https://judge.yosupo.jp/submission/409336) 2.381 |
| compose_large / `max_random_00` | — | 52.217 | 51.425 | [409535](https://judge.yosupo.jp/submission/409535) 30.147 |
| compositional_inverse / `max_random_01` | 2.863 | 2.562 | 2.650 | [409399](https://judge.yosupo.jp/submission/409399) 2.033 |
| compositional_inverse_large / `max_random_00` | — | 44.381 | 44.866 | [409549](https://judge.yosupo.jp/submission/409549) 28.268 |
| power_projection / `max_random_00` | 50.166 | 44.912 | 45.577 | [409549](https://judge.yosupo.jp/submission/409549) 28.524 |
| bostan_mori / `max_random_00` | 55.860 | 51.296 | 50.183 | [405848](https://judge.yosupo.jp/submission/405848) 46.440 |
| linear_recurrence / `max_random_00` | 54.989 | 50.656 | 50.279 | [405848](https://judge.yosupo.jp/submission/405848) 46.221 |
| multipoint_eval / `max_random_00` | 42.663 | 37.167 | 36.670 | [409516](https://judge.yosupo.jp/submission/409516) 13.503 |
| interpolate / `max_random_00` | 58.217 | 53.029 | 51.180 | [409545](https://judge.yosupo.jp/submission/409545) 17.305 |
| taylor_shift / `fft_killer_00` | 42.117 | 25.322 | 23.484 | [409534](https://judge.yosupo.jp/submission/409534) 7.960 |
| inv_mod / `max_random_00` | 75.722 | 76.104 | 33.173 | [409589](https://judge.yosupo.jp/submission/409589) 12.418 |

同じ問題のもう一つの入力と全試行は生データにある。密な大規模合成・合成逆元・多点評価・補間には、上位提出との差が残る。入出力も含む差なので、表の比率を計算部分だけの比率とは解釈しない。

## 不採用の共通 NTT 案

- 根の表を増分生成する案は、524,288 係数の指数が 19.151 → 18.638 ms となった一方、平方が 8.682 → 9.280 ms、平方根が 10.428 → 10.761 ms と悪化したため不採用。
- 逆元の SIMD 葉を 8 組から 4・2・1 組へ減らす案、4 組で融合する案も比較した。500,000 係数の逆元は 8 組 9.890 ms に対し、4 組 10.506、2 組 10.915、1 組 12.898、4 組融合 10.784 ms で、既存方式を維持した。

再現コードは [根の表](fps_tuning_ntt_extend.py)、[SIMD 葉](fps_tuning_ntt_leaves.py)、生データは [根の表](results/fps_tuning_ntt/extend.json)、[SIMD 葉](results/fps_tuning_ntt/leaves.json) にある。これらは各 5 回、`NDEBUG` の候補測定であり、最終比較の条件とは異なる。

## 検証

変更箇所と推移的な依存先を抽出し、20 ヘッダーと 12 単体テストを対象にした。GCC 15.2・GCC 13.4・Clang 21.1.8、Clang ASan/UBSan を確認した。既に成功した同じソースのチェックは個別の記録を再利用している。新しいテストは `NDEBUG` でも照合を省かず、AVX2 とスカラー処理、非 2 冪、空入力・零・共通因子・最大指数などを確認する。

Library Checker の 20 ドライバーは GCC・Clang ともに全公式ケースが成功した。yukicoder 1307 に依存する 2 ドライバーは、ローカルの `YUKICODER_TOKEN` 未設定で入力を取得できなかった。これらはリポジトリの secret を使う [Linux CI](https://github.com/qandori/cp-library/actions/workflows/verify.yml) で検証する。Mac は [Apple Clang CI](https://github.com/qandori/cp-library/actions/workflows/macos-portability.yml) で確認する。

ドキュメントを再生成し、影響する 20 個の結合ヘッダーをコンパイルした。125 ヘッダーの API 表・数式・log 表記と、生成したページの内容も確認した。[対象・ソースハッシュ](results/fps_tuning_validation.json)、[単体検証](results/fps_tuning_check.log)、[公式ケース](results/fps_tuning_judge.log)、[文書検証](results/fps_tuning_docs.log) に記録を残した。公式ケースのログの末尾の失敗 2 件は、上記の yukicoder の入力取得に対応する。

## 再現

```sh
python3 test/benchmark/fps_tuning_reference.py --cache .verify-helper/cache/fps-tuning-reference --output test/benchmark/results/fps_tuning_reference.json
python3 test/benchmark/fps_tuning.py --output test/benchmark/results/fps_tuning_native
python3 test/benchmark/fps_tuning_selection.py --output .verify-helper/cache/fps-tuning-selection.json
```

公式 problem repository は `--official-root` で指定できる。事前に公式の `generate.py` で入力・正答・checker を生成する。`--kernel-only` と `--judge-only`、`--only` で測定対象を絞れる。ソースと入力の SHA-256、全試行、コンパイラ・CPU・カーネルは [environment.json](results/fps_tuning_native/environment.json) と [results.json](results/fps_tuning_native/results.json) に保存する。
