# Dynamic Pattern Matching

2026-10-20。[String Matching with a Dynamic Pattern](https://arxiv.org/html/2506.11318v1) の固定本文版を実装した。文字の挿入・削除・置換と、部分文字列の削除・移動・コピー後の出現数を求める。第7節の本文への文字追加は、今回の対象外としてユーザーに確認した。

## 参考実装

- [論文著者の実装](https://github.com/brunomaletta/DynamicPatternMatching/tree/31e3e21a9125df98af9d9b82642b3af4bdd78fc5)。`matching.cpp` の出現区間の結合・切り出し、`treap.cpp` の局所的な断片の結合を調査した。著者版は mutable treap、DC3、ブロック RMQ を使う。今回の実装は既存の SA-IS と Suffix Tree を共有し、LCP 配列のセグメント木で境界を直接探索する。本文の前処理は線形時間・空間とし、文字列の代入は Suffix Tree をたどって貪欲に分割する。部分文字列のコピーと状態の保存のため、永続 AVL 木を新規実装した。
- この動的パターン照合を直接検証する Library Checker 問題は確認できなかった。構築の中心である Suffix Array の AC を公式 API の実行時間順で取得し、[403148](https://judge.yosupo.jp/submission/403148)（21 ms）、[403162](https://judge.yosupo.jp/submission/403162)（22 ms）、[406232](https://judge.yosupo.jp/submission/406232)（24 ms）を調査した。これらは canard の induced suffix array と AVX2、Linux の regular-file 入力を利用する。今回のライブラリは入出力方式を指定せず、既存の可搬な SA-IS と列比較を使う。接尾辞配列だけを作る提出と、木・LCP・編集用索引まで作る処理の時間を直接比較していない。

取得日時・固定 commit・取得ソースの SHA-256 は `results/dynamic_pattern_matching/reference.json` に保存した。外部ソースは `.verify-helper/cache/dynamic_pattern_matching_references` に保存し、リポジトリには含めない。

## 実装上の境界

本文にないバイトは長さ 1 の断片として残す。空パターンの出現数は `n+1`。全 256 バイト、空本文、本文より長いパターンにも対応する。空接尾辞を接尾辞配列の先頭に置くことで、結合時に本文末尾へ到達する場合も逆配列で判定できる。

断片の途中で分割した場合だけ、両側の境界を修復する。既存の断片間で分割した場合は修復が不要で、追加のコピーと探索を省く。AVL 木には乱数を使わず、同じ部分木のコピーを繰り返しても高さが偏らない。

パターン長が本文長を超えることを許すため、更新は `O(log n + log m)`。本文長以下なら論文と同じ `O(log n)` になる。配列への節点追加を含めた時間は償却。出現数と個々の出現位置は `O(1)`。パターンの長さは `uint64_t` で保持する。

部分文字列の削除を長さによらず処理するため、切り離した節点を直ちに全て解放しない。編集 1 回に追加する節点は `O(log m)` であり、保持量は操作履歴にも依存する。`compact()` は現在の根から到達する節点だけを、共有を保って移す。古い節点数に比例する補助配列を使うため、時間・追加空間は `O(A)`。自動的な回収は行わない。

## 測定条件

- WSL2 Ubuntu、Intel Core i7-13700F、CPU 2 に固定。
- GCC 15.2.0、`-std=c++2b -O2 -DNDEBUG`。generic と、`-march=native -ftrivial-auto-var-init=zero` を追加した native の 2 条件。
- 本文 100,000 バイト、初期パターン 20,000 バイト、1,000 反復。5 試行の中央値。各試行で実装の実行順を固定 seed でシャッフルした。
- `build` は本文索引の構築だけ。その他は索引構築・初期パターン作成を計測対象から除き、編集と `count()` を計測した。操作位置の乱数生成は両実装で同じように計測に含める。
- `edit` はランダム位置への 1 バイト挿入・照合・削除・照合。`equal` は `a`、それ以外は `z` を挿入する。
- `splice` は長さ 5,000 の区間をランダム位置にコピー・照合し、挿入した区間を削除・照合する。
- `move` は長さ 5,000 の区間をランダム位置に移動し、照合を 2 回行う。
- `equal` は同一文字列、`binary` は 2 種類の乱数列、`letters` は 26 種類の乱数列、`periodic` は周期 7。初期パターンは本文の部分文字列。`fragmented` は本文を 2 種類、パターンを 3 種類の乱数列にし、本文にない文字を含ませる。
- 比較対象の KMP 版は `std::string` を編集し、各 `count()` で failure 配列を作り直して本文を線形走査する。文字列の編集時間も含む。全試行で出現数の checksum が一致した。

操作位置は反復内で生成する。初期化で用いる文字列生成の乱数処理は時間に含めない。入出力時間を含まず、AtCoder・Library Checker 上の実測ではない。

## 結果

単位は ms。各更新の欄は 1,000 反復全体の値。

| 本文・パターン | 操作 | 新実装 generic | KMP generic | 新実装 native | KMP native |
| --- | --- | ---: | ---: | ---: | ---: |
| equal | edit | 1.020 | 291.257 | 1.078 | 299.744 |
| equal | splice | 1.561 | 287.161 | 1.584 | 297.103 |
| equal | move | 1.165 | 287.860 | 1.201 | 301.609 |
| binary | edit | 0.338 | 692.746 | 0.394 | 718.887 |
| binary | splice | 0.978 | 709.435 | 0.990 | 731.486 |
| binary | move | 6.660 | 788.604 | 7.597 | 803.215 |
| letters | edit | 0.407 | 110.839 | 0.501 | 122.782 |
| letters | splice | 0.875 | 114.097 | 1.043 | 125.398 |
| letters | move | 5.875 | 115.827 | 7.202 | 127.180 |
| periodic | edit | 0.455 | 100.507 | 0.561 | 125.349 |
| periodic | splice | 1.071 | 103.410 | 1.254 | 128.678 |
| periodic | move | 5.657 | 97.095 | 6.602 | 108.405 |
| fragmented | edit | 4.081 | 717.845 | 4.717 | 776.848 |
| fragmented | splice | 10.185 | 785.560 | 11.215 | 787.520 |
| fragmented | move | 5.803 | 737.653 | 6.867 | 731.706 |

| 本文 | 索引構築 generic | 索引構築 native |
| --- | ---: | ---: |
| equal | 2.931 | 2.880 |
| binary | 8.177 | 8.437 |
| letters | 5.967 | 6.965 |
| periodic | 2.785 | 3.086 |
| fragmented と同じ binary 本文 | 7.266 | 8.411 |

著者実装との比較は、公開 API の共通部分である `equal / edit` に限った。著者版は generic 7.322 ms、native 7.546 ms、新実装はそれぞれ 1.020 ms、1.078 ms。出現数の checksum は一致した。著者版の初期パターンはその `insert(0,string)` を使って作り、初期化は計測に含めない。著者版は他の入力分布・部分文字列操作の測定対象にはしていない。

今回の分布では、新実装は毎回 KMP を実行する方式より速かった。native 条件は generic より遅い欄もあり、native オプションの追加だけで高速になるとはいえない。オンラインジャッジ上の最速性や、全ての入力での優劣は主張しない。

`fragmented / splice` の終了時には 277,552 節点を保持した。これは履歴も含み、現在のパターンの文字数・断片数ではない。メモリを回収する `compact()` の時間とピーク RSS は測定していない。

全試行は `generic/samples.csv`・`native/samples.csv`、条件・コンパイラ・CPU・ソースの SHA-256 はそれぞれの `environment.json` に保存した。

## 検証

- GCC・Clang で 3 ヘッダーの単独 include、既存ヘッダーとの併用。
- 長さ 0～5 の 2 種類の本文・パターンを総当たり。全位置で挿入・削除・置換、全区間と挿入先でコピー・移動を検証。
- ランダムな本文・200,000 回の操作を `std::string` と素朴な重なり込み照合に対して比較。状態の保存・復元・`compact()` も含む。
- 接尾辞区間の結合・全区間の切り出し・最長一致接頭辞を素朴解と比較。
- 全 256 バイト、空本文・空パターン、本文にないバイト、同一文字 200,000 バイト、編集で出現数が 0 になった後の復元。
- 自己コピーによる長さ `UINT64_MAX` のパターンを展開せず構築し、添字アクセス・移動・削除・回収を検証。
- Clang ASan/UBSan。
- 新実装を使う接尾辞配列のドライバーで Library Checker の公式 50 ケースを GCC・Clang ともに通過。動的編集そのもののオンライン AC を意味しない。
- `oj-bundle` でヘッダー・ドライバーを結合し、両コンパイラで構築。説明内のコード例も両方で実行。

`selection.json`・`unit.log`・`sanitize.log`・`validation.json` に対象・結果・入力ハッシュを保存した。Apple Clang はローカルで未測定で、既存の macOS CI により検証する。

## 再実行

```sh
python3 test/benchmark/dynamic_pattern_matching_reference.py --cache .verify-helper/cache/dynamic_pattern_matching_references --output test/benchmark/results/dynamic_pattern_matching/reference.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_pattern_matching_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_pattern_matching.py --output test/benchmark/results/dynamic_pattern_matching/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_pattern_matching.py --native --output test/benchmark/results/dynamic_pattern_matching/reproduced_native
```

公式入力がキャッシュにない場合は、先に `oj download --system --directory .verify-helper/cache/<suffixarray の URL の MD5>/test https://judge.yosupo.jp/problem/suffixarray` を実行する。WSL の GCC・Clang・`oj`・Python の設定は既存の検証環境を使う。
