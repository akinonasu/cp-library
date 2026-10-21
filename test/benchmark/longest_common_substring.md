# 最長共通部分文字列

2026-10-21。`string/longest_common_substring.hpp` に、最長共通部分文字列の開始位置と長さを求める実装を追加しました。最長共通部分列を求める `string/lcs.hpp` とは別のライブラリです。

## 参考提出と手法

Library Checker の [Longest Common Substring](https://judge.yosupo.jp/problem/longest_common_substring) について、公式 API の AC 実行時間順で先頭 3 件のソースを取得・確認しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [406239](https://judge.yosupo.jp/submission/406239) | 13 ms | 共通接頭辞・接尾辞などの直接判定、固定長部分文字列のキーによる候補探索、SA-IS への切り替え |
| [403347](https://judge.yosupo.jp/submission/403347) | 13 ms | 同系統の候補探索と SA-IS、SIMD による文字列比較 |
| [403279](https://judge.yosupo.jp/submission/403279) | 13 ms | 同系統の候補探索と SA-IS、SIMD による文字列比較 |

これらの候補探索は固定長の文字列を衝突なく詰めたキーを使い、キーと実際の一致を確認します。探索量の上限を超えた場合や最適性を確定できない場合は SA-IS に切り替えます。SA-IS の経路では、異なる入力に属する隣接 suffix の LCP を調べます。取得時刻・一覧・ソースの SHA-256 は `results/longest_common_substring/reference.json` に保存しています。

本実装では、全入力に対する最悪線形時間と既存部品の再利用を優先し、`components/suffix_array.hpp` の SA-IS と Kasai の LCP 計算を使います。先頭で短い方の入力が全て一致した場合は、その時点で最適解を返します。それ以外では SA-IS を実行します。候補探索用の辞書は追加していません。

連結列は入力の view と添字変換で表し、文字列のコピーや符号化列の確保を省きます。バイトを 1〜256、区切りを 0 として扱うため、NUL を含む全 256 種類のバイトに対応します。共通接頭辞の比較は `components/sequence_compare.hpp` を再利用し、長い一致では実行時に AVX2 の有無を判定します。LCP 配列は保存せず、探索中に最大値だけを更新します。

上の時間は公開 judge の入出力を含めた値です。今回、速度の比較測定や公開 judge への提出は行っていません。

## 検証

- 長さ 0〜6、文字が NUL と `0xff` の全 127 文字列について、16,129 組を独立した動的計画法と照合しました。
- アルファベット数 2・3・26・256、長さ 0〜100 のランダムな 2,000 組を、引数の両順序で照合しました。
- 返された区間の範囲・文字列の一致・最大長・空解の位置・入力の非変更を検査しました。同じ長さの解が複数ある場合も含みます。
- 空の `string_view`、部分 view、入力同士の重なり、全 256 バイト、区切りに使われがちな `#`・`$`・NUL、比較幅の境界前後を確認しました。
- 50 万文字の同一・反復・不一致・周期文字列と、両端に別の文字を付けた長い共通部分を確認しました。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入を確認しました。
- 公式生成器の全 28 ケースを GCC・Clang の両方で実行し、全て公式チェッカーで検査しました。解の開始位置が一意でないため、出力の文字列比較は使っていません。
- 両コンパイラで `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror` を使い、検証コードとドキュメント例をコンパイル・実行しました。

実行環境は Linux x86_64 / AMD EPYC 9V74 です。公式入力のハッシュ・生成器のコミット・実装のハッシュと検証結果は `results/longest_common_substring/validation.json` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/longest_common_substring_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/longest_common_substring_selection.json
oj-verify run test/longest_common_substring.test.cpp
```
