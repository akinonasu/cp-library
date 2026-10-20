# Suffix Tree

2026-10-20。静的なバイト文字列の Suffix Tree を追加した。SA-IS と LCP から構築し、検索・出現回数・出現位置・異なる部分文字列数・LCP・LCA・suffix link を扱う。終端専用のバイトは予約せず、空の接尾辞は根、その他の接尾辞の終点は葉または内部頂点に置く。

## 参考実装

Suffix Tree 自体の Library Checker 問題はないため、構築の中心になる Suffix Array の AC 提出を公式 API の実行時間順で取得した。取得日時・一覧・ソースの SHA-256 は `results/suffix_tree/reference.json` に保存した。

| 提出 | 取得時の時間 | 確認した手法 |
| --- | ---: | --- |
| [403148 / nandhagk](https://judge.yosupo.jp/submission/403148) | 21 ms | SA-IS、LMS の分類をビットで保持、名前が一意なら再帰を省略、AVX2、入力ファイルの mmap |
| [403162 / nandhagk](https://judge.yosupo.jp/submission/403162) | 22 ms | 同系統の誘導ソートと LMS 部分列の比較、短い周期の検出、縮約後の逆順列 |
| [406232 / Rohan_Kapri](https://judge.yosupo.jp/submission/406232) | 24 ms | 同系統の誘導ソート、LMS の数に応じたソート、先頭部分の順位による比較の省略 |

基本手法の対照は [AtCoder Library の string.hpp](https://github.com/atcoder/ac-library/blob/864245a00b00dd008d1abfdc239618fdb7d139da/atcoder/string.hpp)（CC0）。比較用ソースを commit ID で固定し、SHA-256 も保存した。

本実装も SA-IS を使い、縮約後の名前が全て異なる場合は逆順列から順序を作る。元のバイト列を整数配列へコピーせず参照する。部分列の比較と LCP の計算には既存の `components/sequence_compare.hpp` を使い、対応 CPU では長い一致部分を AVX2 で比較する。入力に予約文字・英小文字限定・特定のファイル形式などの条件は加えない。

SA と LCP から単調スタックで木を構築する。子は CSR に格納し、根だけ 256 要素の直接参照、その他は少数なら線形探索、それ以外は二分探索する。辺の先頭バイトも連続配列に保存し、子を選ぶたびに接尾辞配列と元の文字列を間接参照しない。

接尾辞の終点が単一の子を持つ場合、そのような頂点が続く区間をまとめて比較する。区間内で検索語が終わった場合は、接尾辞配列上で終点の深さを二分探索する。単一子の頂点は必ず接尾辞の終点なので、SA 順を逆に走査すれば飛び先を計算できる。この区間は出現範囲の右端を共有するため、飛び先を右端フィールドの負値で表し、別の整数配列を持たない。返す頂点と出現範囲は通常の辺をたどった場合と同じ。

試作では飛び先を各頂点へ追加すると通常の文字列で遅くなったため、頂点を 16 byte のままにした。先頭バイトの保存と飛び先の圧縮を組み合わせた最終版を、飛び先・先頭バイトの保存がない初期版と比較した。

LCP・LCA 用の索引は必要になった時だけ既存の `StaticRangeMin` で作る。通常の検索・出現数には索引を作らない。

参考提出全体とのローカル時間比較は行っていない。以下の ACL は接尾辞配列のみであり、木・LCP・接尾辞の終点などを作る Suffix Tree と同じ処理量ではない。ジャッジ上の最速性は主張しない。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定。コンパイル・単体テスト・他の測定とは同時に実行しない。
- 長さ 500,000、seed `20261020`。`binary`・`letters`・`bytes` はそれぞれ 2・26・256 種類のバイトからなる乱数列、`equal` は `a` の繰り返し、`periodic` は `abracadabra` の繰り返し。
- 構築は入力生成・入出力・checksum・破棄を除き、入力のコピーと作業配列の確保を含む。ACL は符号なしバイトを整数配列へコピーしてから構築する。`induced` は内部の SA-IS 単体、`base` は初期版の木、`tree` は最終版の木。遅延 RMQ 索引は作らない。
- 検索は構築後に 20,000 回行う。ランダムな開始位置から 32 または 128 バイトを切り出し、末尾で短くなる場合はそのまま使う。3 回に 1 回、最後のバイトに XOR 64 を適用する。入力生成・木の構築を除き、返り値の checksum を含む。
- `binary_search` は接尾辞配列上で `string_view::compare` を使う 2 回の二分探索、`base_find` は初期版、`find` は最終版の出現回数検索。
- 実行順を固定 seed で入れ替えて各 7 回実行した中央値。全試行で同じ操作の checksum が一致することを確認。
- AtCoder・Library Checker 実機、macOS 実機での速度は未測定。

## 結果

単位は ms。接尾辞配列の構築と、木まで構築する時間を分けて比較する。

| 文字列 | generic ACL / induced / tree | native ACL / induced / tree |
| --- | ---: | ---: |
| binary | 22.864 / 19.608 / 44.905 | 22.632 / 19.567 / 46.418 |
| letters | 22.527 / 16.853 / 36.290 | 22.994 / 17.668 / 38.550 |
| bytes | 23.149 / 17.151 / 35.930 | 23.014 / 17.825 / 35.987 |
| equal | 7.186 / 4.828 / 14.476 | 7.217 / 4.902 / 14.773 |
| periodic | 10.140 / 7.537 / 20.251 | 9.800 / 7.732 / 20.703 |

木の構築について、同じ実行ファイルで比較した初期版との違い。

| 文字列 | generic base / tree | native base / tree |
| --- | ---: | ---: |
| binary | 42.903 / 44.905 | 42.576 / 46.418 |
| letters | 34.581 / 36.290 | 36.839 / 38.550 |
| bytes | 33.424 / 35.930 | 35.458 / 35.987 |
| equal | 13.304 / 14.476 | 13.984 / 14.773 |
| periodic | 18.598 / 20.251 | 19.376 / 20.703 |

検索 20,000 回の合計時間。

| 文字列 | generic binary_search / base_find / find | native binary_search / base_find / find |
| --- | ---: | ---: |
| binary | 5.288 / 12.553 / 11.849 | 5.217 / 12.607 / 12.350 |
| letters | 5.068 / 7.382 / 5.773 | 5.292 / 7.489 / 6.572 |
| bytes | 5.066 / 5.442 / 4.796 | 5.233 / 5.910 / 5.120 |
| equal | 2.199 / 5.858 / 0.310 | 2.237 / 6.292 / 0.283 |
| periodic | 2.620 / 1.240 / 0.681 | 2.469 / 1.281 / 0.674 |

初期版に対する検索時間は全測定ケースで短縮し、同一文字列で約 19～22 倍、周期列で約 1.8～1.9 倍、乱数列で約 2～22% 改善した。一方、先頭バイトの保存と飛び先の設定により構築時間は約 1.5～9% 増えた。また、今回の短い検索語では binary・letters の接尾辞配列上の二分探索の方が速い。全ての入力・操作で Suffix Tree が速いとは主張しない。LCP・LCA・suffix link のクエリ時間は未測定。

最終値は `results/suffix_tree/final_generic/summary.json` と `final_native/summary.json`、各 `environment.json` に条件とソースの SHA-256、`samples.csv` に全試行を保存した。

初期版単独の測定は `generic`・`native`、そのソースは `initial_sources.tar.gz` に保存した。最終版の測定スクリプトはこのアーカイブのヘッダーを別名で組み込み、同じ実行ファイルで対照として使う。

## 検証

- GCC 15.2 / Clang 21.1.8 のヘッダー単体・単体テスト・既存ライブラリとの同時 include。
- 長さ 0～8 の 3 種類の文字からなる全 9,841 列。全接尾辞の直接ソート、全部分文字列の集合、素朴な一致判定を対照にした。
- 各頂点の辺・文字列・親子・出現区間・接尾辞の終点・suffix link、全接尾辞対の LCP、短い列の全頂点対の LCA。
- 全部分文字列・不一致の検索語について、存在判定・回数・出現位置と、`advance` を繰り返した検索位置を比較。
- 2 冪前後の長さ、2・26・256 種類の乱数列、NUL を含む文字列、全 256 バイト、空列。
- コピー・ムーブ・再構築・自分の `text()` からの再構築・クリア。
- 20 万文字の同一文字列と乱数列。長さ 4096 の周期 1・2・3・11 の文字列に対する長い検索語と不一致の検索語。
- Clang ASan / UBSan。
- 両コンパイラで Suffix Array の公式 50 ケース、Number of Substrings の公式 24 ケースを検証。
- 両ヘッダーの Bundle、使用例のコンパイルと実行、README とドキュメントの対応。

`check.log`・`sanitize.log`・`validation.log`・`validation.json` に結果を保存した。公式ケースはローカルで照合しており、オンライン提出は行っていない。macOS 実機はローカルでは未検証で、macOS CI の対象。

## 再現

```sh
python3 test/benchmark/suffix_tree_reference.py --cache .verify-helper/cache/suffix_tree_references --output test/benchmark/results/suffix_tree/reference.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/suffix_tree/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/suffix_tree_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/suffix_tree.py --output test/benchmark/results/suffix_tree/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/suffix_tree.py --native --output test/benchmark/results/suffix_tree/reproduced_native
```

測定だけを再現する場合は `reference.json` を更新せず、記録した commit ID の ACL を使う。参照一覧を再取得した場合は、比較用ソースの SHA-256 が変わることがある。
