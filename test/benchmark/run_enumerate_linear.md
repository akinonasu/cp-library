# Run 列挙と線形時間法

## 結論

2026-10-18、線形時間法への全面置き換え、左一致長の再利用、不要候補の除去を比較した。全面置き換えと左一致長の再利用は採用しない。通常経路で、先頭要素が異なる候補の延長計算を省く変更だけを採用した。API と出力順は変えず、最悪時間は引き続き `O(n log n)`、空間は `O(n)`。

線形時間法の存在と、今の実装を速くできるかは区別する。今回測定した線形時間の著者実装は、20 万・100 万文字の非一様な入力でも既存版より遅かった。線形時間法の別実装まで否定する結果ではない。

## 参考資料

- [Ellert・Fischer, Linear Time Runs over General Ordered Alphabets](https://arxiv.org/abs/2102.08670)：一般の順序付きアルファベットでも最悪 `O(n)`。Lyndon 配列に対応する左右の LCE を再利用する。
- [著者実装](https://github.com/jonas-ellert/linear-time-runs/tree/065dfae01cccc8fa7fa8b116cfa91f272c0b22ba)：前後の番兵を要求する。比較用に各バイトを `uint16_t` の 1..256 へ写し、0 の番兵を加えた。変換・出力の変換・既存 API と同じ順序へのソートも測定に含む。MIT の著作権表示を含む元ファイルは取得先キャッシュに保持する。
- [Library Checker 406234 / 12 ms](https://judge.yosupo.jp/submission/406234)：Lyndon stack、比較予算、Z algorithm の分割統治、AVX2、整数ソート。現在の主な比較対象。
- [403379 / 12 ms](https://judge.yosupo.jp/submission/403379)：同率の最速 AC として取得・確認した。

最新一覧・取得日時・参照提出のハッシュは [取得記録](results/run_enumerate_linear_reference.json)。著者実装のハッシュは各測定の `environment.json` に保存する。

## 条件

WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2.0、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native`、assert 有効。変更前は `829efa17b452f904eef5558930d91dd3792feffa`。変更前のヘッダーは Git から取得して比較用に名前だけ変更する。共用する比較・ソート部品は変更していない。

入力 seed 713、実行順 seed 20261018。各条件 1 回ウォームアップし、実行順をシャッフルして 7 回測定した。以下は中央値、単位は ms。測定中は他のコンパイル・テストを実行していない。Library Checker 実機の時間とは直接比較しない。

カーネルは各試行 3 反復。入力生成を除き、列挙、出力整理、checksum を含む。公式ケースは Linux `/tmp` の通常ファイルを stdin / stdout に使い、プロセス起動・入力・構築・列挙・ソート・出力・終了を含む。コンパイル、入力のコピー、公式 checker とハッシュ計算は時間外。最終比較の変更前・採用版には、実際の検証ドライバーを使う。参照提出は元のプログラムをそのままコンパイルする。

著者版は前後の一致長を再利用するグラフと複数の配列を構築する。既存版の直接比較より前処理とメモリアクセスが増えることが、今回のサイズで不利になった要因と考えられる。一様列の専用判定を持たないため、非一様な入力を中心に比較する。

## 最終版

| 長さ | 入力 | 変更前 | 採用版 | 線形時間の著者版 |
| ---: | --- | ---: | ---: | ---: |
| 200000 | 英小文字ランダム | 3.9955 | 3.7698 | 11.4017 |
| 200000 | 2 文字ランダム | 7.2838 | 7.1578 | 15.4988 |
| 200000 | 周期 7 | 1.5522 | 1.3550 | 5.8238 |
| 200000 | Fibonacci | 7.0868 | 6.9910 | 9.8045 |
| 200000 | Thue–Morse | 7.1412 | 6.9517 | 9.6609 |
| 200000 | 周期 4096 | 5.9281 | 6.3189 | 8.9096 |
| 1000000 | 英小文字ランダム | 20.2761 | 18.9857 | 61.6805 |
| 1000000 | 2 文字ランダム | 36.9223 | 36.5725 | 81.9881 |
| 1000000 | 周期 7 | 7.7751 | 6.8219 | 33.1474 |
| 1000000 | Fibonacci | 38.6804 | 38.2746 | 55.4924 |
| 1000000 | Thue–Morse | 44.0496 | 43.2041 | 62.4577 |
| 1000000 | 周期 4096 | 31.3110 | 32.8392 | 50.3325 |

英小文字ランダムで約 5.6–6.4%、周期 7 で約 12.3–12.7% 短縮した。一方、周期 4096 の入力は約 4.9–6.6% 遅く、100 万文字の同一文字ブロックも 13.9056 → 13.9510 ms だった。全入力での改善は主張しない。

公式 24 ケースの入出力込みの結果は以下。合計値は比較の補助であり、ジャッジ順位の指標ではない。

| 指標 | 変更前 | 採用版 | 著者版 | 最速 AC 406234 |
| --- | ---: | ---: | ---: | ---: |
| 各ケースの中央値の合計 | 75.5759 | 72.6655 | 188.5014 | 73.6226 |
| 最大の中央値 | 9.0558 | 8.9826 | 12.9492 | 7.7152 |
| fib_str_03 | 9.0558 | 8.9826 | 12.4136 | 7.7152 |
| max_random_00 | 4.9638 | 4.7931 | 12.5861 | 5.1331 |
| max_random_01 | 5.0794 | 4.8268 | 12.5679 | 5.1945 |

公式ケースの合計は約 3.9% 短縮した。最大ケースでは最速 AC に届いていない。公開ジャッジへの提出は行っていない。

## 比較した変更

| 候補 | 内容 | 判断 |
| --- | --- | --- |
| linear | 著者の線形時間版 | 非一様な入力でも遅く、不採用 |
| cached | 論文の左一致長再利用を既存の Lyndon stack に導入 | Fibonacci は短縮したがランダム列が悪化し、不採用 |
| filtered | 全経路で先頭要素が異なる候補を除外 | 同一文字ブロックで悪化するため、そのままでは不採用 |
| guarded | 通常経路で、比較済み長が 0 の場合だけ除外 | 分岐が増え、改善が安定せず不採用 |
| unindexed | 同一要素区間の索引を使わない通常経路だけで除外 | 採用。最終版の current と同じ変更 |

左一致長再利用の試作は、20 万文字 Fibonacci で 7.2173 → 6.5372 ms。一方、2 文字ランダムは 7.1688 → 7.7298 ms、英小文字ランダムは 4.1165 → 4.3397 ms。比較済み長の配列を追加する費用もあり、全面採用しなかった。試作は既存の全探索・差分テストと ASan/UBSan でも確認した。

採用した除外の根拠は、候補の先頭要素が異なれば右一致長が 0 になること。左一致長が周期未満なら run の長さ条件を満たせず、周期以上なら元々重複として捨てるので、左右の延長計算自体を省ける。Lyndon stack の更新後に除外するため、次の候補の生成は変わらない。同一要素区間の索引を使う経路には分岐を追加しない。

## 生データと検証

- [全面置き換え・左一致長再利用などの比較](results/run_enumerate_linear_exploration/results.json)・[条件](results/run_enumerate_linear_exploration/environment.json)
- [比較済み長が 0 の場合だけ除外する案](results/run_enumerate_linear_guarded/results.json)・[条件](results/run_enumerate_linear_guarded/environment.json)
- [通常経路だけ除外する案](results/run_enumerate_linear_unindexed/results.json)・[条件](results/run_enumerate_linear_unindexed/environment.json)
- [最終版の全測定値](results/run_enumerate_linear_final/results.json)・[条件](results/run_enumerate_linear_final/environment.json)

入力・出力・ソースの SHA-256 と全試行を保存する。各段階の結果は同じ段階内で比較し、別段階の時間差を改善と扱わない。

既存の二値列・三値列の全探索、ランダム列、全バイト、整数列・独自型、大きい周期列、Fibonacci・Thue–Morse、強制的な分割統治との比較を実行した。最終版は GCC 15.2・GCC 13.4・Clang 21.1.8 の単体テスト、Clang の ASan/UBSan を通過した。GCC 15 の全測定で公式 checker を通し、Clang でも結合した単一ファイルで公式全 24 ケースを通過した。[Clang の記録](results/run_enumerate_linear_clang.json)。公開用ヘッダーとドキュメントも検証した。

## 再現

`run_enumerate_reference.py` で最速提出を取得し、`library-checker-problems/generate.py` で runenumerate の公式データを生成してから実行する。名前を変更した候補ヘッダーはキャッシュ内に生成し、公開 API には追加しない。

```sh
python3 test/benchmark/run_enumerate_reference.py \
  --cache .verify-helper/cache/run_enumerate/reference \
  --output .verify-helper/cache/run_enumerate/reference.json
python3 test/benchmark/run_enumerate_linear.py \
  --cache .verify-helper/cache/linear-runs \
  --official /path/to/library-checker-problems/string/runenumerate \
  --references .verify-helper/cache/run_enumerate/reference \
  --output .verify-helper/cache/linear-runs/final \
  --variants baseline current linear fastest --sizes 200000 1000000
```

`--variants baseline filtered cached guarded unindexed linear fastest` で候補を比較できる。`--core-only` はカーネルのみ、`--judge-only` は公式ケースのみを測定する。
