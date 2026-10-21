# 転倒数

2026-10-21。`sequence/inversion_count.hpp` に、入力を変更せずに転倒数を求める関数を追加しました。同値の組は数えず、比較関数とコピー不能な要素にも対応します。

## 参考問題と手法

直接対応する検証問題は [AOJ ALDS1_5_D](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=ALDS1_5_D) です。

関連する Library Checker の [Static Range Inversions Query](https://judge.yosupo.jp/problem/static_range_inversions_query) について、公式 API の AC 実行時間順で先頭 3 件のソースを確認しました。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [393939 / chaihf](https://judge.yosupo.jp/submission/393939) | 80 ms | 基数ソートによる順位付け、区間クエリの並べ替え、差分イベントとブロック累積度数の走査 |
| [181308 / defnotmee](https://judge.yosupo.jp/submission/181308) | 102 ms | 区間クエリの並べ替え、端点移動のイベント化と累積度数の走査 |
| [36979 / Goshkata](https://judge.yosupo.jp/submission/36979) | 106 ms | 座標圧縮、区間クエリの並べ替え、差分イベントの走査 |

取得時刻・一覧・ソースの SHA-256 は `results/inversion_count/reference.json` に保存しています。この問題は多数の区間クエリを扱い、今回の列全体の転倒数とは対象が異なります。本実装は区間クエリ用のデータ構造を構築せず、全体を 1 回数えるためのマージ方式を使います。公開提出の時間と今回の測定値は直接比較しません。

通常の選択肢である「座標圧縮＋Fenwick Tree」と比較しました。本実装は反復的なマージ時に右半分の要素を先に取り出す際、左半分の未処理要素数を加算します。同値では左側を先に取り出すため重複を数えません。小さな単純コピー可能型は値を保持し、それ以外は添字を保持します。整列済みなら作業配列を確保せず終了し、既に順序が合っているマージ区間では比較処理を省きます。

## 検証

- 値が 0・1・2 の長さ 0〜8 の全列 9,841 個、長さ 1〜8 の全順列 46,233 個、重複を含むランダム列 2,000 個。通常・逆順の比較について、全添字対を数える独立した参照実装と照合しました。
- 空列、整数の最小値・最大値、`array`・C 配列・`span`・`deque`・文字列・`vector<bool>`・iota view、既定構築できない要素、大きな要素、コピー不能な要素。
- 長さ 1,000,000 の昇順・降順・全要素同値・大きい値と小さい値が半分ずつ並ぶ列で、64 bit の答えを確認しました。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入。
- AOJ の公開 API から取得した公式 30 ケースを、GCC・Clang の両方で実行し、全出力が一致しました。公開 judge への提出は行っていません。
- `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror` で検証コードをコンパイルしました。ドキュメント例も未使用変数の警告だけを除外して確認しています。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、`-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror`。CPU 2 に固定し、長さ 500,000 の `vector<int>` を使いました。各方法をウォームアップしてから 7 回測定し、中央値を記録しています。実装順は seed `20261021` で入れ替え、毎回答えが一致することを確認しました。

入力生成・入出力は測定外、作業領域の確保・計算・解放は測定内です。コンパイル・単体テスト・他の測定とは同時に実行していません。AtCoder・Library Checker 実機の測定値ではありません。

`random` は一様にシャッフルした順列、`few` は 0〜15 の乱数、`equal` は全要素 7 です。比較用の Fenwick 方式は `sort`・`unique`・各要素の `lower_bound` で圧縮し、既存の `FenwickSum<int>` を使います。単位は ms。

| 入力 | 座標圧縮＋Fenwick | 本実装 |
| --- | ---: | ---: |
| random | 107.445 | 42.159 |
| increasing | 30.572 | 0.137 |
| decreasing | 29.684 | 6.304 |
| equal | 3.869 | 0.136 |
| few | 18.886 | 17.845 |

測定値・ソースのハッシュ・公式ケースの入力と出力のハッシュは `results/inversion_count/` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/inversion_count_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/inversion_count_selection.json
oj-verify run test/inversion_count.test.cpp
g++ -std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -I . test/benchmark/inversion_count.cpp -o /tmp/inversion-count-benchmark
flock /tmp/cp-fps-tuning.lock taskset -c 2 /tmp/inversion-count-benchmark
```
