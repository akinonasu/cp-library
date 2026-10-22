# RLE の実装・検証

2026-10-22。`sequence/run_length_encoding.hpp` に `run_length_encode` を追加した。
戻り値は `std::vector<std::pair<T,std::int64_t>>` で、個数は符号付き64 bitとする。

## 参照と手法

- [AtCoder ABC019 B](https://atcoder.jp/contests/abc019/tasks/abc019_2)
- [公式解説](https://img.atcoder.jp/abc019/editorial.pdf) のB問題。文字列を連続する同じ文字の区間に分け、文字と長さに変換する。
- [Nyaan's Library の Run Length Encoding](https://github.com/NyaanNyaan/library/blob/master/string/run-length-encoding.hpp)。代表値と個数を保持し、一度の走査で区間を出力する。

Library Checker の問題カタログ（`1814c4e5205517e368bb57a8d1127eb961cfeaae`）には、連長圧縮そのものを扱う問題は見つからなかった。対応する fastest の速度比較は行っていない。参考実装と同じ一回走査であり、各区間内の個数は局所変数に保持する。入力長に比例する領域は先に予約せず、圧縮後の区間数に応じて出力を拡張する。

参考実装のランダムアクセスと `int` 個数を、constな input range と `std::int64_t` 個数へ拡張した。添字アクセス・要素の既定構築・代入は要求せず、各区間の先頭値をコピーする。比較述語も指定できる。文字列リテラルは明示的に `std::string_view` で渡し、C配列そのものは全要素を処理する。

比較と要素のコピーを定数時間として、長さ $$n$$、区間数 $$r$$ に対して時間 $$O(n)$$、返り値を含む追加空間 $$O(r)$$。空列に対しても安全に空配列を返す。

## 検証

GCC 14.2・Clang 19、`-std=c++2b -O2 -Wall -Wextra -DDEBUG=1`。ヘッダー単独の二重includeでは `-Werror` も付けた。ClangのASan/UBSanでは `-fsanitize=address,undefined -fno-omit-frame-pointer -g` を追加した。

単体テストは長さ0〜8・3種類の値からなる全9,841配列について、圧縮結果の個数が正、隣接区間の値が異なる、復元すると元入力と一致、入力が変化しないことを検査した。文字列、ヌル文字、非連続コンテナ、boolのproxy、文字列の配列、異なる型のsentinel、既定構築不能な要素と独自の比較、100万要素の同一値も確認した。返す個数の型は `std::int64_t` としてコンパイル時にも検査する。

ABC019 Bの提出例を通常include版・標準 `oj-bundle` 版として、それぞれGCC・Clangでコンパイルした。公式3サンプルと独立した区間境界のoracleを使う100ランダム入力、計412実行が成功した。速度順位の計測やリモートjudgeへの提出は行っていない。

[検証記録](results/run_length_encoding/validation.json)にソース・参照資料のハッシュ、コンパイラ、コマンド、通常展開後の参照URL込みサイズを保存した。実装に外部ライブラリの依存や圧縮マクロはない。

```sh
python test/verify.py --unit-only --selection selection.json --cxx g++ clang++ --jobs 2
python test/verify.py --unit-only --sanitize --selection selection.json --cxx clang++
```

`selection.json` は次の内容を使う。

```json
{"headers": ["sequence/run_length_encoding.hpp"], "unit": ["test/unit/run_length_encoding.cpp"], "sanitize": ["test/unit/run_length_encoding.cpp"]}
```
