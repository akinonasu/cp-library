# FastIO の constexpr 上限

AtCoder の [GCC 15.2 の設定](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)は `-fconstexpr-depth=1024 -fconstexpr-loop-limit=524288 -fconstexpr-ops-limit=2097152`。Clang は `-fconstexpr-depth=1024 -fconstexpr-steps=524288` を指定している。

4 桁の出力表を二重ループで作る実装は、GCC の 2097152 演算の上限でコンパイルに失敗した。従来のローカル検証はこの上限を指定していなかった。各行の 4 文字を直接初期化することで、表の内容を保ったまま定数評価を減らした。実行時の初期化には変更していない。

`.verify-helper/fast_io_test.py` を追加し、GCC / Clang の各上限でヘッダーのコンパイルと `UINT32_MAX` の出力を検証する。既存の CI の unittest discovery から実行される。

- GCC 15.2 と Clang 21 で、各上限を付けた ABC467 G の提出コードをコンパイル。
- 既存の `fast_io.cpp`・`fps_speed_io.cpp`・`fps_io.cpp` を GCC / Clang で検証。出力のテストは 0〜65535、桁の境界、`UINT32_MAX`、乱数 10 万個を含み、4 桁表の全項目を通る。
- 添付された ABC467 G の全 14 入力を、上限付きでビルドした GCC / Clang の両方で実行。前回、独立した公式別解と照合した出力の SHA-256 と全て一致。
- 同じ GCC / 最適化設定で、修正前後の ABC467 G のバイナリ全体が一致。実行時の速度の再測定は行っていない。

[修正前のエラー](results/fast_io_constexpr/reproduction.log)、[検証ログ](results/fast_io_constexpr/check.log)、[設定と照合結果](results/fast_io_constexpr/check.json)。

```sh
python3 .verify-helper/fast_io_test.py
g++ -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -DATCODER -DONLINE_JUDGE \
  -fconstexpr-depth=1024 -fconstexpr-loop-limit=524288 -fconstexpr-ops-limit=2097152 \
  -I. test/benchmark/abc467_g.cpp -o /tmp/abc467_g
clang++ -std=gnu++2b -O2 -fconstexpr-depth=1024 -fconstexpr-steps=524288 \
  -I. test/benchmark/abc467_g.cpp -o /tmp/abc467_g_clang
```
