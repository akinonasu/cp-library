# FPS の constexpr 上限

Taylor shift 用の階乗表を一度に作る初期化が、AtCoder の GCC 設定 `-fconstexpr-ops-limit=2097152` でコンパイルエラーになることを再現しました。FPS の各ヘッダーはこの表を含むため、Taylor shift を呼ばないコードでも発生します。

階乗表の定数評価を 16 項目ずつ、32 回に分割しました。前のブロックの末尾を使って次を計算し、最後に従来と同じ 513 項目の表へ結合します。実行時の初期化は追加していません。

- GCC 14.2 で `-fconstexpr-depth=1024 -fconstexpr-loop-limit=524288 -fconstexpr-ops-limit=2097152`、Clang 19.1.7 で `-fconstexpr-depth=1024 -fconstexpr-steps=524288` を指定してコンパイル・実行しました。
- 表の全項目と、最適化経路の境界をまたぐ Taylor shift を愚直解と照合しました。最大 524288 項の既存テスト、GCC / Clang の関連テスト、ASan / UBSan、および保存済み Library Checker の全 38 ケースも通過しました。
- GCC 14.2 の `-std=c++20 -O2 -DNDEBUG` で修正前後の提出例をビルドし、`.text` と `.rodata` のバイト列がそれぞれ一致しました。速度の再測定は行っていません。
- 展開した Taylor shift 提出例は 239791 B です。通常版全 API と軽量版基本 API を組み合わせる既存の 256 KiB 検査も通過しました。

再発防止テストは [.verify-helper/fps_constexpr_test.py](../../.verify-helper/fps_constexpr_test.py) です。CI の Python テストから同じ上限で実行します。[検証記録](results/fps_constexpr/check.json) にコンパイラ、ソースと生成コードのハッシュ、判定結果を保存しました。

```sh
python3 .verify-helper/fps_constexpr_test.py
```
