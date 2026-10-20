# 評価・補間・Taylor shift の追加調査

基準は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。評価 [409516](https://judge.yosupo.jp/submission/409516)、補間 [409545](https://judge.yosupo.jp/submission/409545)、Taylor shift [409534](https://judge.yosupo.jp/submission/409534) と比較した。今回の候補は実装へ採用していない。

AMD EPYC 9V74、GCC 14.2.0、CPU 2、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。ビルド・検証は `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロックを使用した。whole はプロセスの起動・入出力・終了を含む。順序を反復内で shuffle し、warmup 1 回の後で計測した。すべての計測出力を公式 checker の `input actual official` 順で照合した。

## Taylor shift の区間計測

現行と原本の両方へ同じ clock 計測を追加し、11 試行した。次の「計算」は table 構築・factorial 初期値・weights・NTT・output の合計時間を試行ごとに求めた中央値。「区間外」は同じ試行の whole からこの合計を引いた値の中央値である。

| 入力 | 現行の計算 | 原本の計算 | 現行の区間外 | 原本の区間外 |
|---|---:|---:|---:|---:|
| max_random_00 | 5.214 ms | 5.318 ms | 6.154 ms | 4.586 ms |
| max_random_01 | 5.169 ms | 5.124 ms | 6.115 ms | 4.422 ms |

計算区間の時間はほぼ同じで、whole の差の大部分はその外にある。区間外には入力の読み取り・公開配列の変換・作業領域の確保と解放・結果コピー・出力・プロセスの起動と終了を含む。この計測だけでは、各要因の寄与をさらに分離できない。clock 追加で生成コードも変わり得るため、下記の候補採否には clock を追加していない whole を使った。

factorial 初期値計算は約 6.6 µs だった。ここを事前計算へ置き換えても、約 1.5 ms ある whole の差を埋める根拠にはならない。

## 試した候補

表の値は同じ反復内の「基準 − 候補」の中央値（ms）。正なら候補が速い。括弧内は候補が速かった反復数。

| 候補 | 試行数 | max_random_00 | max_random_01 |
|---|---:|---:|---:|
| Taylor の EVEX256 codegen | 15 | +0.033（8/15） | +0.179（8/15） |
| Taylor の 32 chain を AVX512 の 2 本へ集約 | 15 | -0.088（6/15） | +0.194（12/15） |
| 同上の別測定 | 21 | +0.185（12/21） | +0.308（15/21） |
| Taylor の 64 chain / AVX512 4 本 | 21 | -0.251（7/21） | -0.088（9/21） |
| Taylor の input/kernel weights を別 pass に分離 | 21 | -0.110（7/21） | +0.164（15/21） |
| 分離に加え base の odd lane を使用時に算出 | 21 | -0.022（8/21） | +0.168（15/21） |
| 多点評価の EVEX256 codegen | 15 | +0.121（8/15） | -0.085（6/15） |
| 補間の EVEX256 codegen | 15 | -0.149（6/15） | +0.010（8/15） |

AVX2 の pass 分離は、同時に保持する chain 状態を減らす候補である。演算数とメモリ境界を保ったが、入力間で傾向が異なり、採用できる安定改善を確認できなかった。

AVX512 の 32 chain は一部の測定で短縮したが、今回のローカル CPU に限る結果である。Library Checker の公開環境に表示される `c2d-highcpu-8` で AVX512 が使える根拠は得ていないため、Yosupo での短縮とは扱わない。候補は隔離したコピーだけに適用し、公開 API の AVX2 分岐へ追加していない。

全候補は対応問題の公式全ケースを通過した（Taylor 38、評価 11、補間 9）。64 chain では padding を 2064 words へ広げ、factorial table に `525312! mod p` の 1 項を追加した。内部 AVX512 候補を AVX2 のみの CPU へ無条件で使うことはできない。

## 保存物

[results/fps_evaluation_round3/](results/fps_evaluation_round3/) に原票 5 件、候補 patch、最初の測定・候補生成スクリプト、全 baseline header hash と候補 hash を保存した。原票内の harness hash は `initial_*.py` に対応する。区間計測は現行ヘッダーと原本へ別々の patch を適用するため、`taylor_profile.patch` と `profile_reference.patch` の両方を保存した。

再現用の候補を隔離して生成する例：

```sh
python3 test/benchmark/results/fps_evaluation_round3/reproduce.py \
  --work /tmp/fps-evaluation-round3-repeat --variant taylor_split
```

`--measure` を加えると、生成した候補と同じ baseline、原本を `fps_evaluation.py` で再計測する。公式入力・出力・checker は `/tmp/library-checker-problems`、原本は `/tmp/fps-reference` に置く。作業先は存在しないディレクトリを指定する。再現 runner の構文・patch 適用・hash 一致は確認し、整理後の同一計測は繰り返していない。
