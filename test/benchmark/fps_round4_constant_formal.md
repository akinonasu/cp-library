# 最終定数入力経路を含む FPS 16 問の比較

2026-10-19 UTC。定数入力向けの最終変更を含め、16 問の同じ最大入力を比較し直した。比較元は `bd29f70c70e24e27770b9e9a3777b6cf85664b63`。この測定は round 4 全変更を含む統合版の波及確認であり、定数入力専用経路だけの効果を切り出したものではない。[専用経路の比較](fps_convolution_constant_final_round4.md)は別資料にある。

## 条件と再利用

AMD EPYC 9V74、Linux x86_64、CPU 2 固定、GCC 14.2.0。flags は `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。プロセス起動から入力・公開配列の構築・確保・計算・出力・終了までを含めた。各問で warmup 1 回と 21 試行、3 版の順序を乱択した。計時は `/tmp/cp-fps-tuning.lock` の排他 lock、ビルドと前後の照合は共有 lock。各出力を `checker input actual answer` の順で検証し、16 × 3 × 22 = 1,056 回すべて成功した。

[初期 formal](results/fps_round4_formal.json)に残る before・reference の 32 バイナリを、そのまま再利用した。記録されたバイナリ SHA、driver / 原本の SHA、旧 snapshot の宣言済みヘッダー map、コンパイル command、flags、コンパイラの版表示を照合した。reference は local include を持たない未編集原本で、初期測定と同じ提出を使った。初期ビルド時の compiler executable SHA と実際の全 system include dependency は未記録である。今回取得した現在の hash を、過去にも取得していたとは扱わない。

current の 16 本だけを新規ビルドした。最終 convolution source pack の 154 ファイルと driver archive の 16 ファイルを使い、重複する convolution driver 1 本を除く 169 ファイルを固定した。`-MD` が記録した実際の project / system include 依存の和集合は 500 ファイル。これらの内容、169 ファイルの snapshot と計測当時の live source、compiler driver / cc1plus / collect2 / as / ld、48 バイナリ、input・answer・checker、旧 snapshot と原本を計時前後に照合し、すべて一致した。リンク・実行時の全依存まで取得したという意味ではない。

公式問題 commit は `1814c4e5205517e368bb57a8d1127eb961cfeaae`。入力・正解は初期 formal と同じ SHA、全 48 個の最終出力 SHA も初期の同じ版と一致した。各試行の時間・順序、warmup、checker 成功記録、実 command、すべての前後 hash は [原票](results/fps_round4_constant_formal.json)、進行記録は [ログ](results/fps_round4_constant_formal.log)にある。

## 結果

21 試行の中央値、単位 ms。基準も今回のセッションで再実行した値で、以前の formal の数値を転記していない。

| 問題 | 基準 | 最終統合版 | 未編集原本 |
| --- | ---: | ---: | ---: |
| `convolution_mod` | 12.770 | 12.426 | 9.958 |
| `inv_of_formal_power_series` | 11.790 | 11.720 | 10.255 |
| `log_of_formal_power_series` | 13.369 | 13.458 | 12.286 |
| `exp_of_formal_power_series` | 16.499 | 16.434 | 14.736 |
| `pow_of_formal_power_series` | 24.336 | 24.530 | 22.768 |
| `sqrt_of_formal_power_series` | 12.556 | 12.447 | 10.226 |
| `division_of_polynomials` | 5.966 | 6.010 | 4.796 |
| `kth_term_of_linearly_recurrent_sequence` | 53.625 | 53.469 | 57.351 |
| `composition_of_formal_power_series` | 2.804 | 2.823 | 2.748 |
| `composition_of_formal_power_series_large` | 33.260 | 33.281 | 32.495 |
| `compositional_inverse_of_formal_power_series` | 2.587 | 2.621 | 2.553 |
| `compositional_inverse_of_formal_power_series_large` | 30.215 | 30.230 | 29.663 |
| `multipoint_evaluation` | 15.506 | 15.454 | 14.709 |
| `polynomial_interpolation` | 19.698 | 19.861 | 19.374 |
| `polynomial_taylor_shift` | 10.130 | 10.194 | 8.906 |
| `inv_of_polynomials` | 15.466 | 15.542 | 14.920 |

最大の無作為入力では、convolution・log・exp・pow などに原本との差が残る。小さい増減の原因は分離しておらず、初期 formal と別セッションの数値差を定数検出だけの効果へ帰属させない。API 境界が差の全額だとも断定しない。

## source pack と再現

[再現 pack](results/fps_round4_constant_formal/)は、測定済み snapshot から保存した driver archive、source hash、再利用の出自、固定した helper、再現 script を含む。共通 source は既存の [最終 convolution pack](results/fps_convolution_constant_final_round4/) の `baseline.tar.xz` と `current.patch` を SHA 付きで参照する。既存の pack・初期 formal 原票は変更していない。current 169 ファイルと baseline 167 ファイルを再構築して全 SHA を照合した。測定後の upstream 統合とは分け、pack 作成のために live source を取り直していない。

```sh
python3 test/benchmark/results/fps_round4_constant_formal/replay.py --prepare-only
python3 test/benchmark/fps_all_speed.py --baseline bd29f70c70e24e27770b9e9a3777b6cf85664b63 \
  --work /tmp/fps-round4-formal-new --output /tmp/fps-round4-formal-new.json --trials 21
```

`replay.py` は source の復元・照合だけを行い、コンパイルや再計時を行わない。再計測は、current の FPS source が同じことを確認した checkout で既存の `fps_all_speed.py` を使い、保存した flags・入力・原本 source SHA に揃える。48 本を新たにビルドする場合は新しい比較であり、歴史的な before / reference バイナリとの byte identity は主張しない。測定に実際に使った固定 helper も同梱している。

公式 input / answer / checker と原本 cache は記録と同じ SHA が必要。既定は `/tmp/library-checker-problems` と `/tmp/fps-reference` で、計測 helper の CLI で変更できる。原本は `python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference` で準備できる。別の作業パスは `__FILE__` などを通じてバイナリの内容にも影響するため、source と fixture の一致、出力 checker、同じセッションでの前後 guard を確認し、別パスで歴史的バイナリを完全再生成したとは扱わない。大きな実出力とバイナリは `/workspace/cp-fps-round4/formal-constant` に保持し、pack には生成手順と SHA を保存した。
