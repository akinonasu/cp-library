# 畳み込みの追加高速化と最終比較

この資料は定数係数の経路を追加する前の統合比較です。追加後の最終版は [定数経路を含む最終比較](fps_convolution_constant_final_round4.md)を参照してください。

長さが大きく異なる入力では、短い側の変換を再利用する overlap-add を採用した。通常の NTT 経路にも、生存中の `vector<Mint>` の結果領域を借りる方式を適用した。API・入力の const 性・非対応環境の fallback は維持している。

最終比較の変更前は `bd29f70c70e24e27770b9e9a3777b6cf85664b63`、変更後はその revision に今回の convolution 変更を適用した snapshot、参照は未編集の [409465](https://judge.yosupo.jp/submission/409465) である。途中に取り込んだ `BulkOutput` の pragma 範囲変更は双方に含めた。[段階別の分析](fps_convolution_phases_round4.md) と初期候補の原票は `7521ea8` を基準にした別の実験であり、ここでの短縮量と混ぜない。

## 測定条件

2026-10-19 UTC、AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。3 版とも `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。ビルドは共通 lock の SH、計時は EX。通常ファイルから入力し、通常ファイルへ出力する新しいプロセスの起動から終了待機までを測った。checker と SHA-256 計算は計時区間外である。

各ケースで予備実行を 1 回行い、その後 21 回測った。seed `20261044` で各回の 3 版の順序をシャッフルし、同じ回の変更前と変更後を paired 比較した。下表はプロセス全体の中央値、単位 ms。「paired 短縮」は各回の変更前−変更後を計算してから中央値を取るため、表の中央値同士の差とは一致しない。

| 入力 | 長さ | 変更前 | 変更後 | 原本 | paired 短縮 | 変更前への勝数 |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| max_random_00 | 524288 × 524288 | 12.441 | 12.421 | 9.793 | -0.133 | 9/21 |
| max_random_01 | 524288 × 524288 | 12.360 | 12.353 | 9.819 | 0.037 | 11/21 |
| all_same_00 | 524288 × 524288 | 13.441 | 13.279 | 8.540 | 0.075 | 13/21 |
| all_same_01 | 524288 × 524288 | 12.322 | 12.388 | 9.130 | -0.125 | 8/21 |
| small_and_large_00 | 100 × 524288 | 9.732 | 5.657 | 8.115 | 3.868 | 21/21 |
| small_and_large_01 | 1000 × 524288 | 9.749 | 6.131 | 8.067 | 3.664 | 21/21 |
| small_and_large_02 | 524288 × 100 | 9.561 | 5.701 | 8.048 | 3.820 | 21/21 |
| small_and_large_03 | 524288 × 1000 | 9.544 | 6.109 | 8.097 | 3.417 | 21/21 |
| medium_00 | 1323 × 9953 | 1.436 | 1.439 | 1.228 | -0.000 | 10/21 |
| medium_01 | 4294 × 3307 | 1.397 | 1.377 | 1.167 | 0.013 | 12/21 |
| custom | 65536 × 65536 | 2.970 | 2.830 | 2.287 | 0.137 | 16/21 |
| custom | 262144 × 262144 | 7.204 | 7.008 | 5.715 | 0.196 | 17/21 |
| custom | 300000 × 220000 | 7.185 | 7.135 | 5.829 | 0.110 | 15/21 |

偏った長さの 4 ケースは全 21 回で変更前と原本を上回った。その他のケースには原本との差が残る。最大サイズ 2 件で今回の変更による安定した改善は確認しておらず、すべての畳み込みが fastest と同等になったとは言えない。これは同一マシンでの原本との比較であり、judge 上の実行時間や順位ではない。段階別分析は採用前の測定なので、この最終版の残差をそのまま説明する内訳としては使わない。

## 正しさと再現

予備実行を含む `13 × 22 × 3 = 858` 出力すべてが公式 checker を `checker input actual answer` の順で通過した。これは最終比較の 13 ケースの検証であり、別途担当した公式全ケース検証と重複して数えない。custom 入力は各長さに対し Python の `Random(617+n)` から `[0,998244353)` の値を a、b の順に生成し、既存入力と独立した再生成結果の SHA-256 一致を確認した。正解ファイルに対しても 3 版の全出力を checker で確認した。

コンパイル時に `-MD` で実際の user/system 依存を取得した。変更前 467、変更後 468、原本 284 ファイル、全 source snapshot・ライブ source・入力・正解・checker・compiler・binary の前後 SHA-256 が一致した。変更前 152 ファイル、変更後 153 ファイルと原本は、保存した archive・patch からの復元でも全 SHA-256 が一致した。独立した読み取り監査でも試行順・paired 差・中央値・勝数を再計算した。

[原票と前後 manifest](results/fps_convolution_round4/primary.json)、[実行した helper](results/fps_convolution_round4/harness.py)、[baseline archive](results/fps_convolution_round4/baseline.tar.xz)、[変更 patch](results/fps_convolution_round4/current.patch)、[未編集原本](results/fps_convolution_round4/reference.cpp.xz)、[復元 script](results/fps_convolution_round4/replay.py) を保存した。大きな公式入力は重複保存せず、原票に公式 commit と入力・正解の SHA-256 を記録した。原本 source SHA-256 は `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。

```sh
python3 test/benchmark/results/fps_convolution_round4/replay.py \
  --work /workspace/convolution-round4-replay
```

これだけでは source 復元と照合を行う。`--build-and-measure` を加えると、保存済み原票の compiler・公式 commit・checker・全 13 入力/正解・試行条件を照合して再測定する。公式データが別の場所にある場合は `--official` を指定する。custom 入力と正解は復元先で生成し、元の SHA-256 と照合してから計時する。復元先のパスが異なると binary の配置や SHA-256 が変わる可能性がある。

保存した `harness.py` は計測時と完全に同じ版である。計測後に replay の条件照合を補強し、同じ変更を [継続用 helper](fps_convolution_round4.py) の埋め込み replay に反映した。[版の記録](results/fps_convolution_round4/harness_versions.json) に両 SHA-256 を残した。計測コードは変更していない。
