# 定数経路を含む畳み込みの最終比較

定数多項式向けの経路、長さが偏った入力への overlap-add、汎用 NTT の結果領域借用を含む最終ソースで、13 ケースを再測定した。変更前は `bd29f70c70e24e27770b9e9a3777b6cf85664b63`、参照は未編集の [409465](https://judge.yosupo.jp/submission/409465)。[定数経路追加前の比較](fps_convolution_round4.md) と原票は保持し、今回の記録を別に保存した。

定数入力 2 件は変更前より約 3.8 ms 短縮し、どちらも全 21 回で速かった。偏った長さの 4 件も変更前と原本を上回った。一方、ランダムな最大入力 2 件は今回の比較で変更前より約 0.2 ms 遅く、原本との差も残る。全ケースで fastest と同等になったとは言えない。

## 条件と結果

2026-10-19 UTC、AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。3 版とも `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。通常ファイルから入力し、通常ファイルへ出力する新しいプロセスの起動から終了待機までを計時した。ビルドは共通 lock の SH、計時は EX。checker と SHA-256 計算は計時区間外。

各ケースで 1 回予備実行した後、21 回測定した。seed `20261044` で各回の 3 版の順序をシャッフルした。単位 ms、時間は中央値。「paired 短縮」は同じ回の変更前−変更後の中央値であり、中央値同士の差とは一致しない。

| 入力 | 長さ | 変更前 | 変更後 | 原本 | paired 短縮 | 変更前への勝数 | 原本への勝数 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| max_random_00 | 524288 × 524288 | 12.521 | 12.718 | 9.838 | -0.164 | 7/21 | 0/21 |
| max_random_01 | 524288 × 524288 | 12.413 | 12.643 | 9.801 | -0.209 | 7/21 | 0/21 |
| all_same_00 | 524288 × 524288 | 13.171 | 9.352 | 8.603 | 3.815 | 21/21 | 0/21 |
| all_same_01 | 524288 × 524288 | 12.425 | 8.598 | 9.301 | 3.780 | 21/21 | 19/21 |
| small_and_large_00 | 100 × 524288 | 9.701 | 6.022 | 8.331 | 3.557 | 21/21 | 21/21 |
| small_and_large_01 | 1000 × 524288 | 9.733 | 6.297 | 8.164 | 3.559 | 21/21 | 21/21 |
| small_and_large_02 | 524288 × 100 | 9.661 | 5.918 | 8.131 | 3.777 | 21/21 | 21/21 |
| small_and_large_03 | 524288 × 1000 | 9.697 | 6.261 | 8.261 | 3.463 | 20/21 | 20/21 |
| medium_00 | 1323 × 9953 | 1.470 | 1.458 | 1.208 | -0.004 | 9/21 | 1/21 |
| medium_01 | 4294 × 3307 | 1.408 | 1.380 | 1.167 | 0.022 | 13/21 | 0/21 |
| custom | 65536 × 65536 | 3.119 | 2.907 | 2.368 | 0.252 | 17/21 | 2/21 |
| custom | 262144 × 262144 | 7.629 | 7.408 | 5.909 | 0.326 | 15/21 | 0/21 |
| custom | 300000 × 220000 | 7.664 | 7.544 | 5.972 | 0.069 | 11/21 | 0/21 |

これは同じマシンで原本を実行した全プロセス比較であり、judge の速度や順位ではない。表の差は採用した変更全体の結果で、個々の変更の効果やランダム最大入力の退行原因を分離していない。定数経路追加前の測定とは別のセッションなので、両表の変更後の時間を引いて定数経路単独の効果とはしない。

## 検証と再現

予備実行を含む `13 × 22 × 3 = 858` 出力すべてが公式 checker を `checker input actual answer` の順で通過した。公式全ケース検証は別に実施するため、この harness では対象 13 ケースだけを確認した。

custom 入力は各長さに対し Python の `Random(617+n)` から `[0,998244353)` の値を a、b の順に生成した。既存入力を毎回独立に再生成したものと SHA-256 で照合し、正解に対して 3 版の全出力を checker で確認した。

コンパイル時の `-MD` で実際の user/system 依存を取得した。変更前 467、変更後 469、原本 284 ファイルで、全 source snapshot・ライブ source・依存・入力・正解・checker・compiler・binary の前後 SHA-256 が一致した。保存した baseline archive と patch から変更前 152 ファイル、変更後 154 ファイル、未編集原本を復元し、全 SHA-256 一致を確認した。追加のビルドや計時は復元確認では行っていない。

新しい `fps_convolution_constant.hpp` の測定時 SHA-256 は `bf6976669ca416df9c5cb8999814a164fc04502eaa8be900061684674df01ca6`、原本は `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。実行した helper と保存版も同一の SHA-256 である。

[原票・条件・前後 manifest](results/fps_convolution_constant_final_round4/primary.json)、[実行 helper](results/fps_convolution_constant_final_round4/harness.py)、[baseline archive](results/fps_convolution_constant_final_round4/baseline.tar.xz)、[変更 patch](results/fps_convolution_constant_final_round4/current.patch)、[未編集原本](results/fps_convolution_constant_final_round4/reference.cpp.xz)、[復元 script](results/fps_convolution_constant_final_round4/replay.py)、[独立監査](results/fps_convolution_constant_final_round4/scope_review.json) を保存した。大きな公式入力は重複保存せず、原票に公式 commit と入力・正解の SHA-256 を記録した。

```sh
python3 test/benchmark/results/fps_convolution_constant_final_round4/replay.py \
  --work /workspace/convolution-constant-round4-replay
```

このコマンドは source の復元と照合を行う。`--build-and-measure` を加えると、原票の compiler・公式 commit・checker・全 13 入力/正解・試行条件・compiler が報告した全依存の内容 hash を照合して再測定する。公式データが別の場所にあれば `--official` を指定する。custom 入力と正解は復元先で生成し、計時前に元の hash と照合する。復元先のパスが異なると binary の配置や SHA-256 が変わる可能性がある。
