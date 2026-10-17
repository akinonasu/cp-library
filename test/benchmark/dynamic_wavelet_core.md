# Dynamic Wavelet Matrix の core 高速化

2026-10-17 の固定比較に対応する候補です。公開 API と既定の Block=1024・Branch=8 は変えず、次の 4 点だけを組み合わせます。

- P: BMI2 が有効な x86_64 で word 内の `select` に PDEP を使う。
- S: `select` の候補が 1 要素になったら残りの bit を照合する。
- A: AVX2 が有効な x86_64 で 32・64 bit 整数の構築時の bit 詰めをまとめる。
- R: 2 つの境界を渡す `range_freq` で共通する上位 bit の経路を共有する。

P/A の命令条件を満たさない場合はスカラー処理を使います。実行時の CPU 判定はありません。独自の `BitVector` に `access_rank` がない場合も `select` の従来経路を残します。neighbor・leaf・direct-build・木の高さの変更や外部実装の取り込みは含めません。

## 比較元と再現用の原本

- 基準 B: `abfcfcd99e4f4d0d9740a5add1b718193f690cc6`。
- 実験の公開 head: `18b2ea5c4c88b80d32ddbea83b0591bf6bdc89b0`。
- [固定計画・runner・入力生成器・独立 oracle・差分の原本](https://github.com/qandori/cp-library/tree/18b2ea5c4c88b80d32ddbea83b0591bf6bdc89b0/test/benchmark/dwm_core)。
- [実験 run、attempt 1](https://github.com/qandori/cp-library/actions/runs/39070184924)。

PSAR は B に P/S/A/R の順で適用した 2 ヘッダーです。SHA-256 は以下です。

| ヘッダー | B | PSAR |
| --- | --- | --- |
| `components/dynamic_bit_vector.hpp` | `6ccac3add8d0c35649ec0a2b50e03e1142502473c8e2b1fd453ffe77c32689e7` | `1d6a47b55ba375810b30fa297bfae23755099b13e3b4707f2ff575b63e5278ae` |
| `data_structure/dynamic_wavelet_matrix.hpp` | `a2af5a1addcd64377a290b1f5dd6ec7587f100d0132c8f6551649af4468ae33a` | `8683aa75c8b5011b9b3e9c06adde7f97b0f2c94cd4345baadc5b80eb3f577caf` |

## 固定比較の条件と結果

GitHub Actions の通常の公開 Ubuntu 26.04 runner、GCC 15.2.0、`-O2 -std=gnu++23 -fmodules -ftrivial-auto-var-init=zero -DONLINE_JUDGE -DATCODER`、assert 有効、LTO なしです。native は `-march=native`、scalar は `-march=x86-64 -mtune=generic -mno-avx -mno-avx2 -mno-avx512f -mno-bmi -mno-bmi2 -fno-tree-loop-vectorize -fno-tree-slp-vectorize` です。AtCoder の実機で測った結果ではありません。

各ケース n=100000、seed=20261017、構築以外は原則 Q=20000、access は Q=40000、構築は Q=0 です。型・分布・操作・全入力は原本の `plan.json` とソースに固定されています。各ケース・flag の全候補に共通する replay 数を pilot から 1 回だけ決め、AA/BB の校正と ABBA/BAAB を 4 cycle 実行しています。以下は 8 個の完全な cross block の CPU 時間比の中央値から求めた変化率で、負が高速化です。異なる host・flag・ケースをまとめた平均ではありません。

| B → PSAR | native | scalar |
| --- | ---: | ---: |
| 20 bit 全域の構築 | -25.02% | -0.17% |
| 20 bit 重複値の構築 | -37.91% | -0.03% |
| 20 bit 同一値の構築 | -60.40% | -0.05% |
| uint64/64 全域の構築 | -11.74% | +0.04% |
| 20 bit 成功する select | -24.53% | -6.69% |
| uint64/64 成功する select | -61.79% | -56.70% |
| 20 bit 重複値の有効な k の select | -16.71% | +1.62% |
| 20 bit 一般の 2 境界 range | -12.33% | -6.82% |
| 20 bit 狭い値域の range | -43.99% | -45.70% |
| int64/64・20 bit 分だけ変化する値の狭い range | -35.52% | -37.79% |
| 短区間を含む混合問い合わせ | -11.22% | -3.73% |
| int64/64 全域の混合操作 | -7.83% | -7.00% |

重要なトレードオフは scalar の重複値 select です。B → PSAR は CPU +1.62234%、wall +1.62341% で、8 block すべて遅く、CPU の block 範囲は +1.1157% ～ +8.8798% でした。native の同じケースは -16.7139% です。

構成要素の追加も常に有利ではありません。native の S 単独の重複値 select は B 比 +1.26% で 8 block すべて遅く、scalar の PS → PSAR の通常の 20 bit select は +0.9585% で 8 block すべて遅くなっています。後者の最終 PSAR は B 比では -6.6885% です。大きな改善とこれらの退行を両方残し、速いケースだけで採否を決めません。

その他の access・独立 caller・更新・混合操作も固定計画で確認しています。変化が小さい対照ケースもあり、「すべての操作が速い」という結果ではありません。全 13 shard・90 比較行・4320 計測を保持しており、replay 上限到達や 1 秒未満の計測はありません。一方、AA/BB の経験的な揺れが 0.5% を超える行が多く、計画上の記述的な分類は多くが inconclusive です。これは汎用的な信頼区間や全 CPU に対する性能保証ではありません。既存の [count の比較](dynamic_wavelet_count.md) にある Library Checker 提出も別の API・処理条件であり、この結果から fastest を主張しません。

## 小さなローカル確認

通常の回帰テストには `test/unit/dynamic_wavelet_core.cpp` を追加しています。word の select、SIMD 幅と末尾、符号付き・部分 bit 幅、singleton の未一致・重複・更新、境界外と共通 prefix の range、`access_rank` を持たない BitVector の select/range を vector の答えと照合します。既存の bit vector・wavelet・count・standalone・judge テストは置き換えません。

以下は手元での簡易確認用であり、上の固定実験と同じ統計的な検証ではありません。assert は無効にしません。実行する CPU が対応する場合だけ `-march=native` を使ってください。

```sh
g++ -std=c++23 -O2 -march=native -I . test/unit/dynamic_wavelet_core.cpp -o /tmp/dwm-core-check
/tmp/dwm-core-check
```

既存の `test/benchmark/dynamic_wavelet_matrix.cpp` は変更していません。基準 B の checkout を `$BASE`、この checkout を `$CANDIDATE` とし、同じソースをそれぞれのヘッダーに対してコンパイルできます。

```sh
g++ -std=c++23 -O2 -march=native -I "$BASE" "$CANDIDATE/test/benchmark/dynamic_wavelet_matrix.cpp" -o /tmp/dwm-base
g++ -std=c++23 -O2 -march=native -I "$CANDIDATE" "$CANDIDATE/test/benchmark/dynamic_wavelet_matrix.cpp" -o /tmp/dwm-candidate
for kind in select access updates mixed duplicates short; do
  for executable in /tmp/dwm-base /tmp/dwm-candidate /tmp/dwm-candidate /tmp/dwm-base; do
    "$executable" library "$kind" 100000 20000
  done
done
```

この既存 benchmark は seed 713・unsigned/20 bit です。構築 ms、操作 ms、ノード領域 byte、checksum を出力します。同じ入力の checksum の一致を確認し、CPU・OS・compiler と全出力を記録してください。これは軽量な動作・性能確認であり、2 境界 range の性能、64 bit の性能、各ケースの完全な出力や最終状態の一致を証明しません。2 境界 range の動作は上の回帰テストで確認し、固定実験を再現する場合は原本をそのまま別 checkout で使います。新しい n や操作構成は元の計測結果と区別します。

## 残る確認

実験 head での候補・sanitizer・ARM Mac・通常 CI の成功は、この本番差分の最新版 CI の成功とは別です。この差分はまだコンパイル・実行していません。追加した回帰テストと影響する依存先について GCC・Clang・ASan/UBSan・ARM Mac・judge・docs を最新版で確認し、必要な CI が通るまではマージ可能としません。

大きい n と構造変更の最終確認は別の固定計画が必要です。最小限の候補は n=1000000 の 20 bit/64 bit 構築と select/range、default と 128x4 の block・branch 境界前後での増加・縮小・split/merge、reserve/clear/rebuild 後の同じ問い合わせです。小さな状態は vector、大きな読み取り状態は固定配列からの独立な答えで確認し、公開前にケース数・操作数・実行時間上限を固定します。新しい workflow や追加計測をこの差分には含めていません。
