# Run 列挙の比較

2026-10-18 の線形時間法の検討と追加の改善は [別資料](run_enumerate_linear.md) に記録しました。以下は変更前の測定です。

2026-10-17、WSL2 Linux x86_64 / Core i7-13700F / GCC 15.2.0、CPU 2 固定。Library Checker への提出時間ではなく、同じマシン上での比較です。

## 参考実装

2026-10-16 UTC に [AC の実行時間順一覧](https://judge.yosupo.jp/submissions?problem=runenumerate&status=AC&order=%2Btime) の先頭 8 件を取得しました。順位・実行時間・ソースの SHA-256 は `results/run_enumerate_reference.json` に記録しています。

- [406234 / Rohan_Kapri / 12 ms](https://judge.yosupo.jp/submission/406234): Lyndon stack、比較予算を超えた場合の Z algorithm による分割統治、AVX2、整数ソート。主な比較対象です。元の AVX2・O3 指定を維持しています。
- [403379 / nandhagk / 12 ms](https://judge.yosupo.jp/submission/403379): 同順位の AC としてソースを取得しました。
- [191550 / wery0 / 22 ms](https://judge.yosupo.jp/submission/191550): Lyndon stack と 64 bit ハッシュによる LCE。ソースにも Thue–Morse 列への注意があります。
- [313634 / 29 ms](https://judge.yosupo.jp/submission/313634): 64 bit ハッシュと二分探索による LCE。`hash` の比較対象です。
- [403370 / nandhagk / 24 ms](https://judge.yosupo.jp/submission/403370): Z algorithm による分割統治も確認しました。

## 実装

通常は Lyndon stack の 2 つの文字順で候補を生成します。整数の表現を `memcpy` で読み、64 bit 単位で前後の一致長を求めます。一様な列は先に検出し、この長い比較では CPU 対応を確認して AVX2 を使います。短い比較に毎回 AVX2 の分岐を入れる方式も測定し、列挙中は 64 bit 比較を採用しました。

64 箇所の隣接要素を調べ、56 箇所以上が一致する場合は同一要素区間の両端を前計算します。区間内の一致をまとめて飛ばし、飛ばした部分を比較予算から除きます。これは速度のための選択であり、結果の正しさには影響しません。

比較予算は `64*n` です。超過時は部分結果を捨て、Z algorithm による分割統治へ切り替えます。各節点では元の列における極大性と分割点をまたぐことを確認し、同じ両端を持つ候補の最小周期を残します。候補を節点ごとに整理することで、作業領域と出力は `O(n)` に抑えます。最悪時間は `O(n log n)` です。全経路でハッシュを使いません。

小さい結果には比較ソート、大きい結果には 11 bit 単位の安定な整数ソートを使います。stack と Z 配列は既存の `DefaultInitAllocator` を使い、上書き前のゼロ初期化を省きます。

## カーネル測定

長さ 200,000、seed 713。入力生成を除き、列挙・結果のソート・checksum を含みます。1 回あたり 5 反復、順序を変えた 7 回の中央値、単位は ms です。`generic` は `-std=c++23 -O2 -DNDEBUG`、`native` はさらに `-march=native -ftrivial-auto-var-init=zero` を指定します。実際のジャッジ環境と同一とは限りません。

| 入力 | 本実装 generic | 406234 generic | 本実装 native | 406234 native |
| --- | ---: | ---: | ---: | ---: |
| 英小文字ランダム | 4.1341 | 4.5774 | 4.1490 | 4.3694 |
| 2 文字ランダム | 7.3873 | 7.7525 | 7.3273 | 7.4890 |
| 同一文字 | 0.0044 | 0.0044 | 0.0043 | 0.0045 |
| 周期 7 | 1.7573 | 2.0086 | 1.9589 | 2.0511 |
| Fibonacci | 7.2047 | 7.1657 | 7.4267 | 7.2032 |
| Thue–Morse | 7.5019 | 7.4474 | 7.4869 | 7.3403 |
| 同一文字ブロック | 2.9113 | 15.3498 | 2.9202 | 14.8554 |
| ほぼ周期的 | 5.0853 | 4.9733 | 5.5482 | 5.1585 |

同一文字ブロックは 257 文字ごとに `b`、他は `a` です。ほぼ周期的な列は長さ 1,024 のランダムな 2 文字列を繰り返し、8,191 文字ごとに `c` を入れます。各生成器は `run_enumerate.cpp` にあります。

同一文字ブロックでは比較対象に対して約 5.1〜5.3 倍速く、他の入力では同程度か入力に応じた差があります。Fibonacci・Thue–Morse・ほぼ周期的な列では遅い場合もあり、常に最速とはしていません。

全ての決定的実装で checksum と run 数が一致しました。`hash` は Thue–Morse 列で不一致となり、最終実装には採用していません。不一致は CSV の `correct=False` に残しています。

`word` は一様性判定も含めて AVX2 を無効化、`vectorized` は列挙中も長い比較に AVX2 を利用、`no_index` は同一要素区間の前計算を無効化、`comparison_sort` は最終ソートを比較ソートへ置き換えた版です。全候補を調べる既定の実行では、比較予算 8・32、常時分割統治、GCC O3、配列の初期化、要素単位の比較も測定できます。途中の比較結果は `results/run_enumerate_*exploration/` にあります。

## 公式ケースと検証

Library Checker の公式 24 ケースを GCC・Clang の両方で全件 AC。二値列の長さ 13 まで、三値列の長さ 8 までの全探索、ランダム列、NUL・上位ビットを含むバイト列、整数列、独自型、長さ 200,000 の周期列・Fibonacci・Thue–Morse 列を確認しました。分割統治を強制する差分テスト、ASan/UBSan、ARM64 GCC の静的ビルドと QEMU 実行、結合コードの単体コンパイルも通過しています。

Mac 実機では未実行です。ARM64 と Linux Clang で確認し、x86_64 専用の命令は条件付きで隔離しています。検証結果・対象ソースの SHA-256・公式入力の SHA-256 は `results/run_enumerate_validation.json` にあります。

公式 24 ケースについて、各提出を実行ファイルとしてコンパイルした入出力込みの比較も行いました。通常のファイルを標準入出力に使い、実行順を変えた 7 回の中央値を採用しています。プロセス起動時間を含むため、小さい入力ではその影響があります。

| 指標 | 本実装 | 406234 |
| --- | ---: | ---: |
| 24 ケースの中央値の合計 | 84.78 ms | 83.61 ms |
| `max_random_00` | 5.37 ms | 5.60 ms |
| `fib_str_03` | 9.17 ms | 8.06 ms |

## 再現

```sh
python3 test/benchmark/run_enumerate_reference.py --cache .verify-helper/cache/run_enumerate/reference --output test/benchmark/results/run_enumerate_reference.json
python3 test/verify.py --unit-only --selection test/benchmark/results/run_enumerate_selection.json --cxx g++ clang++
python3 test/verify.py --unit-only --sanitize --selection test/benchmark/results/run_enumerate_selection.json --cxx clang++
python3 .verify-helper/ci.py judge --selection test/benchmark/results/run_enumerate_selection.json
python3 test/benchmark/run_enumerate.py --reference-cache .verify-helper/cache/run_enumerate/reference --output test/benchmark/results/run_enumerate_generic --variants current word vectorized no_index comparison_sort reference_avx2 hash
python3 test/benchmark/run_enumerate.py --reference-cache .verify-helper/cache/run_enumerate/reference --output test/benchmark/results/run_enumerate_native --native --variants current word vectorized no_index comparison_sort reference_avx2 hash
python3 test/benchmark/run_enumerate_judge.py --reference-cache .verify-helper/cache/run_enumerate/reference --tests .verify-helper/cache/dbb44b1ae6c4e0e9ffc19efb65a5a3a3/test --output test/benchmark/results/run_enumerate_judge
```

公式ケースのキャッシュ位置は verification-helper のログに表示されます。CPU 番号を変更する場合は `--cpu` を指定してください。各出力フォルダには生の測定値、集計、コンパイラ・フラグ・CPU・測定対象の SHA-256 を残します。比較実装のソースはキャッシュに置き、リポジトリには取得用スクリプトと出典情報を残します。
