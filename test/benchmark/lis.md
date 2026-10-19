# 最長増加部分列

2026-10-19。`string/lis.hpp` に長さ・添字列の復元・各位置を終点とする長さを追加した。狭義増加・広義増加・独自の比較関数に対応する。

## 参考実装

Library Checker の Longest Increasing Subsequence について、公式 API の AC 実行時間順で先頭 3 件を取得した。取得日時・一覧・ソースの SHA-256 は `results/lis/reference.json` に保存した。

| 提出 | 取得時の時間 | 比較した手法 |
| --- | ---: | --- |
| [372659 / IceKylin](https://judge.yosupo.jp/submission/372659) | 10 ms | 末尾に追加できる場合の早期終了、比較結果を位置の加算に変換する二分探索、各位置の長さから後方へ復元 |
| [372657 / IceKylin](https://judge.yosupo.jp/submission/372657) | 10 ms | 同じ探索・復元方式。添字計算の書き方が異なる |
| [238857 / sortA0329](https://judge.yosupo.jp/submission/238857) | 10 ms | 比較関数・射影を受け取る実装。既に比較した末尾を探索対象から除外し、各位置の長さで復元 |

本実装も末尾候補の配列と二分探索を使い、末尾にそのまま追加できる場合は探索しない。最後の候補は判定済みなので、探索範囲から除く。復元には各位置の長さを使い、親配列を別に持たない。

境界探索は再利用できる `components/binary_search.hpp` に分離した。`lis_length` では各位置の長さを保存しない。16 byte 以下で自明にコピー可能な要素は値を連続配列へ置き、それ以外と `bool` は元の列の添字を置く。要素のコピーができない場合も添字方式を使う。番兵、座標圧縮、値の加減算は不要。

参考提出とのローカル時間比較は行っておらず、ジャッジ上での最速性は主張しない。以下は同じ条件で実行した標準的な実装との比較。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定。コンパイル・単体テスト・他の測定とは同時に実行しない。
- 長さ 500,000、seed `20261019`。同じ列で 3 回計算する全体を測定。入力生成と入出力を除き、作業配列の確保・計算・返り値の checksum を含む。
- 実装の順序を固定 seed で入れ替えて各 9 回実行した中央値。各試行で checksum が一致することを確認。
- 整数型は `int`。その他の要素型の速度は未測定。

`standard` は各要素で `std::lower_bound` / `std::upper_bound` を呼ぶ実装、`append` はそれに末尾へ追加できる場合の早期終了を加えた実装。作業領域の確保方法と、長さから後ろ向きに復元する処理は本実装と揃えている。

`random` は 0～INT_MAX の乱数、`increasing`・`decreasing` は単調列、`equal` は全要素 7、`few` は乱数を 16 で割った余り、`alternating` は偶数位置で `n-i`、奇数位置で `i` とする。

## 結果

単位は ms。各行は 3 回分の合計時間。

| 操作・列 | generic standard / append / 本実装 | native standard / append / 本実装 |
| --- | ---: | ---: |
| 長さ・random | 48.997 / 48.055 / 15.804 | 47.380 / 45.684 / 15.501 |
| 長さ・increasing | 16.108 / 2.969 / 3.233 | 15.794 / 2.924 / 3.196 |
| 長さ・decreasing | 1.248 / 1.527 / 0.907 | 1.254 / 1.562 / 1.024 |
| 長さ・equal | 1.239 / 1.517 / 0.902 | 1.264 / 1.537 / 1.003 |
| 長さ・few | 19.507 / 18.816 / 12.858 | 18.487 / 18.886 / 12.730 |
| 長さ・alternating | 22.542 / 15.487 / 16.518 | 21.931 / 15.510 / 16.480 |
| 復元・random | 51.159 / 49.251 / 17.732 | 52.024 / 52.358 / 17.462 |
| 復元・increasing | 19.816 / 6.482 / 5.213 | 19.583 / 6.327 / 5.232 |
| 復元・decreasing | 2.586 / 2.795 / 2.281 | 2.673 / 2.789 / 2.315 |
| 復元・equal | 2.563 / 2.768 / 2.208 | 2.668 / 2.812 / 2.319 |
| 復元・few | 21.275 / 20.423 / 14.087 | 21.356 / 21.983 / 14.234 |
| 復元・alternating | 25.262 / 17.696 / 18.296 | 25.371 / 18.613 / 17.607 |
| 広義の長さ・random | 47.722 / 48.859 / 15.795 | 46.620 / 46.010 / 15.545 |
| 広義の長さ・equal | 16.277 / 3.158 / 3.375 | 15.888 / 2.903 / 3.230 |
| 広義の長さ・few | 41.999 / 42.648 / 20.899 | 41.648 / 39.898 / 20.376 |

乱数列では、標準実装に対して長さ計算の時間が約 1/3、復元の時間が約 1/2.9 になった。一方、早期追加を持つ対照より遅い場合もあり、例えば native の広義・全要素同値では約 11% 遅い。全ての列・要素型での優位性は主張しない。AtCoder 実機・macOS 実機での速度は未測定。

条件とソースの SHA-256 は各 `environment.json`、全測定値は `samples.csv`、中央値は `summary.json` に保存した。

## 検証

- GCC 15.2 / Clang 21.1.8 のヘッダー単体・単体テスト・既存ライブラリとの同時 include。
- 長さ 9 まで、3 値からなる全 29,524 列について、狭義・広義・昇順・降順を独立した二乗時間 DP と比較。
- 2 冪前後・乱数列・単調列・同値列・空列。復元した添字の順序、値の順序、最適長、各位置の長さを確認。
- 符号付き 64 bit の最小値・最大値、128 bit、bool、浮動小数点、文字列、deque、C 配列、span、string_view、transform view、コピーできない要素、デフォルト構築できない要素、大きい要素と独自比較。
- 20 万要素の単調列・同値列。
- 境界探索は長さ 0～257 の全境界と、長さ INT_MAX / 2・INT_MAX−1・INT_MAX の代表的な境界で検証。標準の lower_bound / upper_bound とも比較し、添字が範囲外へ出ないことを確認。
- Clang ASan / UBSan。
- 両コンパイラで Longest Increasing Subsequence の公式 19 ケースが正答。最適長は公式出力、添字と値の狭義増加は独立した検証で確認。
- 両ヘッダーの Bundle、LIS の 2 例と境界探索の使用例を両コンパイラでコンパイル。

`check.log`・`sanitize.log`・`validation.log`・`validation.json` に検証結果を保存した。macOS 実機はローカルでは未検証で、macOS CI の対象。

## 再現

```sh
python3 test/benchmark/lis_reference.py --cache .verify-helper/cache/lis_references --output test/benchmark/results/lis/reference.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/lis/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/lis_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/lis.py --output test/benchmark/results/lis/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/lis.py --native --output test/benchmark/results/lis/reproduced_native
```
