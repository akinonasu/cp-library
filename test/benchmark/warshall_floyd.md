# ワーシャルフロイド法

`graph/warshall_floyd.hpp` の単一スレッド実装です。32 頂点のタイル、512 頂点の大ブロック、4 行 × 2 ベクトルをレジスタに保持する min-plus 積を組み合わせています。AVX-512F / AVX2 を実行時に選び、負辺と到達不能値を扱います。安全性を証明できる入力では公開 API の距離型が 64 bit でも内部を 32 bit にします。範囲が足りない場合は逆に拡張し、符号付き加算のオーバーフローを避けます。

## 参照した実装・資料

2026-10-20 に取得しました。取得 URL・時刻・SHA-256 は [参照記録](results/warshall_floyd_references.json) に保存しています。第三者のコードは本体へ取り込んでいません。

- [maspy の検証済み実装](https://maspypy.github.io/library/graph/shortest_path/warshall_floyd.hpp): 到達不能の行・要素を除外する通常の k-i-j ループ。比較用 `baseline` はこの構造と同じ通常実装を独立に記述しています。
- [Luzhiled の実装](https://ei1333.github.io/library/graph/shortest-path/warshall-floyd.hpp): 到達不能判定付きの三重ループ。ブロック化や明示的な SIMD はありません。
- [Spiral: Program Generation for All-Pairs Shortest Paths](https://radio.spiral.net/software/apsp.html): キャッシュ分割、レジスタでの展開、SIMD による高速化を比較しています。
- [Blocked All-Pairs Shortest Paths Algorithm on Intel Xeon Phi KNL Processor: A Case Study](https://arxiv.org/abs/1811.01201): 三段階のブロック化、アラインメント、AVX-512 による実装を扱います。本ライブラリは OpenMP を使いません。
- 対応する検証問題は [AOJ GRL_1_C](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_1_C) です。AOJ の提出一覧 API は取得時に HTTP 410 を返し、fastest 提出の特定・同一環境での比較はできませんでした。以下は検証済み通常実装と手法を比較したローカル測定で、オンラインジャッジの最速記録を主張するものではありません。

## 測定条件

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、`-std=c++23 -O2 -DNDEBUG`。`-march=native` は使わず、ライブラリ内の関数単位の target 指定と GCC の O3 指定を使います。CPU 2 に固定し、他の本作業のコンパイル・テストと測定はファイルロックで分離しました。仮想環境での測定です。

全て `WarshallFloyd<std::int64_t>` を使用しています。`auto` はこの CPU では AVX-512F、`avx2` は AVX-512 のみを無効化、`scalar` は明示的な SIMD を無効化したものです。`scalar` でもコンパイラによる通常の最適化は許可しています。型変換・範囲確認・結果の書き戻しも `solve()` の時間に含めます。

入力は全て、自己辺を除く全ての有向辺を持つ密グラフです。乱数はドライバに実装した SplitMix64、seed = 20261020 です。

| 入力名 | 重み | 内部演算 |
| --- | --- | --- |
| `positive32` | 1〜1,000,000 の非負乱数 | 32 bit |
| `positive64` | `positive32` の各辺を 10,000,000 倍 | 64 bit |
| `negative32` | 1〜1,000 の正の乱数 + `potential[to] - potential[from]`。potential は −10,000〜10,000 | 32 bit |
| `negative64` | `negative32` の各辺を 10,000,000 倍 | 64 bit |

potential による負辺付き入力には負閉路がありません。公開 API が 64 bit でも常に 32 bit で計算できるわけではなく、負辺では範囲判定も厳しくなります。128 bit へ拡張する必要がある極端な重みはこの速度表の対象外です。

各ケース・各実装を同じサイズで 1 回ウォームアップし、その後 7 回測りました。比較実装の順番はラウンドごとにシャッフルしています。全測定値・コンパイルコマンド・CPU 情報・ソースとバイナリの SHA-256 は [測定 JSON](results/warshall_floyd.json) にあります。

## 通常実装との比較

以下に 1,024 頂点の `solve()` の中央値を示します。全実装で全成分を走査した checksum が一致しています。

| 入力 | 通常の三重ループ | ブロック・SIMD無効 | ブロック・AVX2 | ブロック・AVX-512 | 通常実装比 |
| --- | ---: | ---: | ---: | ---: | ---: |
| positive32 | 639.71 ms | 214.08 ms | 51.48 ms | 24.88 ms | 25.7倍 |
| positive64 | 649.87 ms | 760.08 ms | 138.04 ms | 44.35 ms | 14.7倍 |
| negative32 | 634.32 ms | 911.86 ms | 49.84 ms | 35.23 ms | 18.0倍 |
| negative64 | 631.18 ms | 854.17 ms | 125.61 ms | 64.47 ms | 9.8倍 |

## 2秒以内の処理規模

`solve()` の時間と、辺リストの読み込みから全距離行列の出力・終了までのプロセス全体の実時間を分けて測っています。後者には起動、確保、入力、構築、計算、文字列化、出力、解放を含めます。入力形式は `n m` に続く `from to weight`、出力は全ての距離です。入力・出力とも通常のファイルを使い、`/dev/null` への出力ではありません。ウォームアップ後なので OS のファイルキャッシュを使い、永続化のための `fsync` は行っていません。

| 入力 | 計算のみで全7回が2秒未満の最大測定サイズ | 計算時間 中央値 / 最大 | 入出力込みで全7回が2秒未満の最大測定サイズ | 全体時間 中央値 / 最大 |
| --- | ---: | ---: | ---: | ---: |
| positive32 | 4,608 | 1.897 / 1.952 秒 | 3,840 | 1.664 / 1.698 秒 |
| positive64 | 3,584 | 1.882 / 1.903 秒 | 3,200 | 1.878 / 1.891 秒 |
| negative32 | 3,968 | 1.867 / 1.886 秒 | 3,456 | 1.855 / 1.997 秒 |
| negative64 | 3,072 | 1.734 / 1.875 秒 | 2,688 | 1.597 / 1.663 秒 |

境界の前後の結果も測定 JSON に残しています。非負 32 bit の 4,096 頂点は、入出力込みの中央値が約2秒でした。3,968 頂点には全体時間が一度 2.50 秒になった試行があり、中央値では2秒未満でも、上の全7回の基準からは外しています。

「全7回で2秒未満」は今回測ったサイズの中での最大値です。境界付近を主に 128 刻みで調べており、厳密な最大値や任意の入力での保証ではありません。2秒付近のサイズは余裕が少なく、提出先の CPU・入力構造・メモリ制限・入出力方式によって変わります。

メモリについても、上表の各サイズを別に1回ずつ実行して Python の `resource.getrusage(RUSAGE_CHILDREN)` で Linux の最大 RSS を測りました。[全記録](results/warshall_floyd_memory.json)では、入力を mmap するドライバのメモリも含めています。

| 入力 | 計算のみのサイズ / 最大 RSS | 入出力込みのサイズ / 最大 RSS |
| --- | ---: | ---: |
| positive32 | 4,608 / 244.5 MiB | 3,840 / 399.6 MiB |
| positive64 | 3,584 / 99.5 MiB | 3,200 / 306.3 MiB |
| negative32 | 3,968 / 181.8 MiB | 3,456 / 308.8 MiB |
| negative64 | 3,072 / 73.5 MiB | 2,688 / 206.9 MiB |

本体だけなら内部変換を伴う 64 bit 指定で約 `12 * p * p` バイト、変換しない 64 bit 指定で約 `8 * p * p` バイトです。`p` は32の倍数に切り上げた頂点数です。公開型を `int32_t` にすると、拡張が不要な範囲では約 `4 * p * p` バイトです。入力・出力・利用者側の行列などは別に必要です。

余裕を持たせる場合、今回の入出力込み測定では `positive32`: 3,840、`positive64`: 3,072、`negative32`: 3,328、`negative64`: 2,688 頂点が全7回とも約1.71秒以内でした。

## 正しさ・移植性

- 独立した 128 bit の通常実装と比較。32 / 64 bit、空行列、32 / 512 頂点境界、到達不能、負辺、負閉路、自己辺、多重辺、カスタム `inf`、整数の上限・下限、オーバーフロー検出、再計算を確認しました。負閉路は 512 頂点をまたぐ場合も確認しています。
- 通常演算・AVX2・AVX-512 の各内部経路を直接検査しました。
- GCC 14.2 / Clang 19 でヘッダー単独・二重 include と単体テストが成功。GCC の ASan + UBSan も成功しました。
- AOJ GRL_1_C の公式全50ケースを GCC / Clang / AVX2 指定 / SIMD 無効でそれぞれ照合し、全て一致しました。オンライン提出は行っていません。
- 検証コマンド、検証対象ソースと入力のハッシュは [検証記録](results/warshall_floyd_validation.json) にあります。

## 再現

リポジトリルートで次を実行します。`--work` のディレクトリを再利用すると、最初に記録したソース・コマンド・バイナリのハッシュ一致を要求します。ソース変更後は別のディレクトリを使ってください。

```sh
python3 test/benchmark/warshall_floyd.py \
  --plan test/benchmark/results/warshall_floyd_plan.json \
  --work /tmp/warshall-floyd-bench \
  --output /tmp/warshall-floyd-results.json --cpu 2
```

`--cpu` は実行可能な論理 CPU 番号へ変更できます。大きい入力・出力はケースごとに生成・削除し、作業ディレクトリにはバイナリとハッシュ記録を残します。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ \
  --selection test/benchmark/results/warshall_floyd_selection.json
python3 test/verify.py --unit-only --cxx g++ --sanitize \
  --selection test/benchmark/results/warshall_floyd_selection.json

g++ -std=c++23 -O2 -I. test/warshall_floyd.test.cpp -o /tmp/warshall-floyd-judge
oj download --system \
  'https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=GRL_1_C' \
  -d /tmp/warshall-floyd-cases
oj test -c /tmp/warshall-floyd-judge -d /tmp/warshall-floyd-cases
```
