# Sortable Segtree

## 参考実装

2026-10-18 に [Point Set Range Sort Range Composite](https://judge.yosupo.jp/problem/point_set_range_sort_range_composite) の AC 一覧を実行時間順に取得した。

- [393919 / chaihf / 286 ms](https://judge.yosupo.jp/submission/393919)：最速。Patricia trie でソート済みブロックを保持し、階層ビット集合でブロック境界を管理する。前向き・逆向きの積、位置ごとの Segment Tree を使う。問い合わせではブロックを分割しない。
- [401387 / Rohan_Kapri / 294 ms](https://judge.yosupo.jp/submission/401387)：同系統の Patricia trie 実装。未変更の提出全体を比較する。
- [363210 / ssmkybb / 299 ms](https://judge.yosupo.jp/submission/363210)：meldable binary trie、fastset、前向き・逆向きモノイドを調査した。

393919 に同梱された treap 方式も比較した。main が使うクラスを `TreapSortableSegmentTree` に置き換え、キー配列をその API が受け取る `vector<int>` に変更した。公式のキーは `0..10^9` で、値の変換による意味の変化はない。入出力と関数合成は同じコードを使う。この版は公開ジャッジでの独立した AC 記録ではなく、今回全公式ケースを checker で確認した比較用の版である。

[取得記録](results/range_sort_reference.json) に一覧・日時・参照 URL・ソースの SHA-256 を保存した。上の時間は公開ジャッジの記録で、以下の手元の測定値とは直接比較しない。

## 実装

`SortableSegtree<Key,S,op,e,commutative>` として実装した。値のモノイドは関数合成に限定せず、逆元や可換性も要求しない。可換なら逆向きの積を保持する領域を省ける。

キーは符号付き・符号なしの 64 bit 以下の整数。符号 bit を反転して通常の数値順に正規化する。重複キーは固定した葉 ID を内部の第 2 キーとして区別する。安定ソートではない。ソート後も同じ葉を使い、一点でキーを変更する場合だけその要素を含むブロックを切り分ける。値だけの更新はブロックを切り分けない。

ブロック内は二分岐だけを保持する Patricia trie、ブロック間の集約は Segment Tree とする。ノードを配列で管理し、不要になった分岐は再利用する。区間集約は両端ブロックの部分積と中央の完全なブロックの積から求め、問い合わせではブロックを分割しない。境界を管理する階層ビット集合は `components/predecessor_set.hpp` に分離した。

初期構築は `O(n)`、保持空間は `O(n)`。キーと葉 ID を合わせた bit 数の上限を `w` として、全 `q` 操作の償却合計は `O((n+q)(w+log n))` となる。仮想的に圧縮前の完全な深さ `w` の trie を考えると、初期状態のノード数は `O(nw)`。ブロックの分割による増加は 1 回 `O(w)` で、meld の探索は重なる仮想ノードの破棄に対応する。境界の追加は更新・ソートごとに高々 2 個なので、全境界削除数も `O(n+q)`。位置側の Segment Tree の更新はそれぞれ `O(log n)`。ソート 1 回を最悪 `O(log n)` とするものではない。

## 測定条件

2026-10-18、WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2.0、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native -ftrivial-auto-var-init=zero`、assert 有効。各条件 1 回ウォームアップ後、実行順を seed 393919 でシャッフルして測定した。最終版は 7 試行、試作比較は 3 試行の中央値。単位 ms。

実際の検証ドライバーと参照提出全体をコンパイルする。Linux `/tmp` の通常ファイルで stdin/stdout をリダイレクトし、プロセス起動・入力・構築・全クエリ・出力・終了を含む。コンパイル、入力のコピー、公式 checker と出力 SHA-256 の計算は時間外。各入力の最初の出力を checker で確認し、以降の出力はハッシュの一致を確認する。出力が変わった場合は再度 checker を実行する。測定中は他のコンパイル・テストを実行していない。

今回の API は重複キー・符号付きと 64 bit キーなどにも対応する。参照は相異なる 32 bit キーを前提とし、入出力も異なる。機能と入出力の差を含めた比較で、処理時間差の全てを木の実装だけに帰するものではない。AtCoder・Library Checker 実機での速度は未測定。

## 最終版

| ケース・指標 | 採用版 | 393919 | 401387 | 393919 の treap 版 |
| --- | ---: | ---: | ---: | ---: |
| 各ケースの中央値の最大値 | 191.0356 | 181.6824 | 182.4234 | 268.9322 |
| many_0_query_00 | 156.1191 | 139.2978 | 140.8815 | 221.5763 |
| many_1_query_00 | 82.2784 | 72.7814 | 75.2862 | 111.6984 |
| many_2_query_00 | 187.1903 | 163.9916 | 169.6081 | 239.0831 |
| many_3_query_00 | 189.8187 | 181.6824 | 182.4234 | 252.3324 |
| max_random_00 | 164.8235 | 152.1555 | 156.8768 | 268.9322 |
| sort_only_short_00 | 40.9490 | 39.5299 | 39.5626 | 43.7973 |

最大の中央値は最速提出より約 5.1% 大きい。個別には many_2_query_00 で約 14.1% 遅い。今回の実装が fastest だとは主張しない。公開ジャッジへの提出は行っていない。

## 試作比較

`inline` は代表キーを各ノードに複製し、キー配列への間接参照をなくす案。キー配列自体も残す比較用の試作である。`split` は問い合わせでも両端のブロックを切り分けて位置側の Segment Tree だけを参照する案。

| ケース | 採用方式 | inline | split |
| --- | ---: | ---: | ---: |
| many_1_query_00 | 80.7979 | 83.5866 | 153.7738 |
| many_2_query_00 | 185.6471 | 181.2351 | 181.6844 |
| max_random_00 | 158.5476 | 159.0375 | 187.1108 |
| sort_only_short_00 | 41.4167 | 42.4131 | 43.0657 |

`inline` は改善が一貫しないので採用しない。`split` は問い合わせが多い入力で約 1.90 倍遅くなるため採用しない。試作段階と最終測定の時間をまたいで改善率を計算しない。

## 検証と再現

- 公式 23 ケースを GCC・Clang の両方で checker により検証。Clang の単一ソース展開・NDEBUG ビルドも全 23 ケースを検証。
- 文字列連結の独立した配列モデルと、ソート・キー変更・値だけの変更・区間集約・全体集約・全要素取得を照合。
- 重複キー、負数、符号付き・符号なしの 64 bit 端点、8 bit キー、非デフォルト構築型、可換なモノイド、空列、空区間、再構築、コピー、ムーブを確認。
- 10 万要素でソート・キー変更を繰り返し、ノード再利用と確保領域を確認。
- Predecessor Set は小さい全ての集合と、64・4096・262144 の境界を含む入力を `std::set` と照合。
- GCC 13.4 / 15.2、Clang 21.1.8、ASan/UBSan、NDEBUG、独立ヘッダー、マクロ非依存、ドキュメント例を確認。

[最終版の全試行](results/range_sort/results.json)・[条件とハッシュ](results/range_sort/environment.json)、[試作の全試行](results/range_sort_exploration/results.json)・[条件](results/range_sort_exploration/environment.json)。

```sh
python3 test/benchmark/range_sort_reference.py --cache .verify-helper/cache/range-sort-reference --output test/benchmark/results/range_sort_reference.json
python3 test/benchmark/range_sort.py --references .verify-helper/cache/range-sort-reference --official /path/to/library-checker-problems/data_structure/point_set_range_sort_range_composite --output test/benchmark/results/range_sort
python3 test/benchmark/range_sort.py --references .verify-helper/cache/range-sort-reference --official /path/to/library-checker-problems/data_structure/point_set_range_sort_range_composite --output test/benchmark/results/range_sort_exploration --explore --trials 3
```
