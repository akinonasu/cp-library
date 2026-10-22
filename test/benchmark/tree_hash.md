# 木・根付き木のハッシュ

2026-10-22。無向木の隣接リストを受け取り、子の順序に依存しないハッシュを求めます。根付き版は各頂点以下の部分木を、根なし版は重心を根にした木を扱います。

## 参考実装

- [Nyaan's Library: RootedTreeHash](https://github.com/NyaanNyaan/library/blob/master/tree/rooted-tree-hash.hpp)：部分木の高さに応じた乱数を各子のハッシュへ加え、その積を取ります。子をソートせず、他の木の処理履歴に依存しない値を求める手法を参考にしました。
- [Nyaan's Library: tree_hash](https://github.com/NyaanNyaan/library/blob/master/tree/tree-hash.hpp)：重心が 1 個なら同じ根付きハッシュを 2 個、2 個ならそれぞれを根にしたハッシュを昇順に返します。この戻り値の形式を採用しました。
- [Library Checker Rooted Tree Isomorphism Classification](https://judge.yosupo.jp/problem/rooted_tree_isomorphism_classification) の [106481 / Felerius](https://judge.yosupo.jp/submission/106481)、[106483 / Felerius](https://judge.yosupo.jp/submission/106483)、[405048 / Rohan_Kapri](https://judge.yosupo.jp/submission/405048) を取得して確認しました。取得時の AC 時間順の先頭 3 件で、それぞれ 26・33・37 ms です。子の局所的な分類 ID をハッシュ化して加算し、ハッシュ表で親の ID を決めます。パス入力の特別扱いや、子が 1 個の場合の ID 再利用も用いられています。

分類 ID を各入力内で割り当てる最速提出は、独立に構築した木同士でハッシュを比較する今回の API と役割が異なります。今回は Nyaan 版の高さごとの積を用い、剰余を 61 bit で正規化しながら 2 本計算します。2 本の乱数列は同じ seed から生成します。既定 seed はプロセス内で共通にし、明示した seed でも再現できます。走査は再帰を使わず、長いパスをそのまま処理します。

取得した提出の一覧・ソースの SHA256・Nyaan 版のソースの SHA256 は `results/tree_hash/references.json` に保存しています。最速提出や Nyaan 版との実行時間比較、公開 judge への提出は行っていません。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2、`-std=c++23 -O2 -DNDEBUG`。50 万頂点の隣接リストを先に用意し、CPU 0 に固定してウォームアップ 1 回、続いて 7 回測定した中央値です。構築中のメモリ確保と結果の解放を含み、グラフ生成・入出力・同型クラスへの ID 割り当てを含みません。seed は 42 です。

| 形状 | 根付き木・全ての部分木 | 根を持たない木 |
| --- | ---: | ---: |
| パス | 15.507 ms | 29.780 ms |
| 星 | 8.551 ms | 10.562 ms |
| 二分木 | 8.303 ms | 13.494 ms |
| ランダムな親への接続 | 74.737 ms | 85.054 ms |

全試行・コンパイラ・フラグ・ソースの SHA256 は `results/tree_hash/measurements.json` に保存しています。公開提出の時間は judge 上で入出力・分類処理を含む値なので、この表と直接比較できません。

## 検証

- 10 頂点までの根付き・順序なし木の全 1,205 形状を生成しました。独立した「子の括弧列をソートする」厳密な表現と、全ての部分木のハッシュを双方向に照合しました。根を無視した分類は、全頂点を根にした括弧列の最小値と比較しました。
- seed 0・42・`UINT64_MAX` のそれぞれについて全形状と頂点のランダムな付け替えを検証し、隣接順の変更、異なる木同士の部分木の比較を確認しました。さらに各 seed で 300 個のランダム木・ランダムな根を検証しました。
- 親・部分木サイズ・高さ、独立した成分サイズ計算で求めた重心からのハッシュ、空の木・1 頂点・重心 2 個、既定 seed の共有、再構築・コピー・move、ハッシュ表のキーとしての利用を確認しました。
- 50 万頂点のパスと星を確認しました。パスは両端を根にした全ての部分木を逆順で比較しています。
- Mersenne 素数の乗算を、境界値と乱数 10 万組について 128 bit の剰余演算と比較しました。
- GCC・Clang の単独・二重 include と単体テスト、Clang ASan・UBSan を確認しました。constexpr 上限は通常 CI と同じです。
- Library Checker の公開 17 ケースを生成し、GCC・Clang の検証コードを公式 checker で検証しました。各コンパイラで全ケースを通過しました。検証コードは `NDEBUG`・`-Wall -Wextra -Werror` でコンパイルしました。ケースのハッシュと生成器のコミットも測定 JSON に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2 --selection test/benchmark/tree_hash_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/tree_hash_selection.json
python3 "$LC_ROOT/generate.py" "$LC_ROOT/tree/rooted_tree_isomorphism_classification/info.toml"
python3 test/benchmark/tree_hash.py --problem "$LC_ROOT/tree/rooted_tree_isomorphism_classification" --build-dir /tmp/tree-hash --output /tmp/tree-hash/measurements.json
```

`LC_ROOT` は Library Checker の問題リポジトリのパスです。AtCoder 用テストは追加していません。ハッシュの衝突がないことを保証するテストではありません。
