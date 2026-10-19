# 動的遅延セグメント木

2026-10-20。`DynamicLazySegtree<S,op,e,F,mapping,composition,id,initial,Coord>` を追加した。区間更新・区間集約・一点代入・一点更新・境界探索を扱う。座標を事前に集めずに処理できる。

## 参考実装

Library Checker の Range Affine Range Sum (Large Array) について、公式 API の AC 実行時間順の先頭 3 件を取得し、ソースを調べた。取得日時・メタデータ・ソースの SHA-256 は `results/dynamic_lazy_segtree/reference.json` にある。

| 提出 | 取得時の時間 | 比較した手法 |
| --- | ---: | --- |
| [405491 / toomer](https://judge.yosupo.jp/submission/405491) | 77 ms | 全クエリの端点を radix sort で圧縮。静的な探索木で端点を一括検索し、長さを持つ遅延セグメント木で処理 |
| [372144 / qdot3](https://judge.yosupo.jp/submission/372144) | 88 ms | 全端点を radix sort で圧縮。区間の長さと和を持ち、反復型の遅延セグメント木で処理 |
| [393925 / chaihf](https://judge.yosupo.jp/submission/393925) | 109 ms | 全端点をソート・圧縮し、二分探索で端点を変換。圧縮後の区間長を持つ再帰型の遅延セグメント木 |

これらは全操作を先読みできる場合の方法。本ライブラリでは座標圧縮を必要としないオンラインの API を採用した。区間長を集約値に含める方法を使い、未作成区間の集約値を `initial(l,r)` から得ることで、巨大な座標範囲にも対応する。

通常版 Range Affine Range Sum の上位 3 件も取得し、メタデータとソースの SHA-256 を `reference_dense.json` に保存した。特に [402323 / nandhagk](https://judge.yosupo.jp/submission/402323) は 16 分木・AVX2・先のクエリの prefetch を利用する。このような固定された法や更新型、操作の先読みに依存する部分は、一般の型をオンラインで扱う動的木には直接導入していない。

参考提出をそのまま使ったローカル時間比較は行っておらず、ジャッジ上の最速性は主張しない。

## 実装

- ノードを `vector` にまとめ、子を 32 bit の添字で保持。ポインタや一点ごとの動的確保は不要。
- 初期状態の実ノード数は 0。未更新区間は `initial(l,r)` で集約する。
- 部分更新では、保留中の作用がなければ、更新する側だけ作成する。保留中なら必要な子へ伝播する。
- 全区間更新では 1 ノードだけで集約値と作用を保持する。
- `prod` は子から得た集約値に祖先の作用を適用し、伝播・作用の合成・ノード作成を行わない。
- 境界探索は祖先の作用を合成して持ち降り、未作成区間も仮想的に分割する。探索でもノード作成は不要。
- `F` の等値比較を使わず、更新の有無をフラグで保持。非可換な作用を適用順に合成する。
- 分割点は `std::midpoint`、長さは符号なし型で計算し、負の座標や符号付き 64 bit の差のオーバーフローを避ける。
- 配列からの構築ではちょうど $$2n-1$$ ノードを作る。コピー・ムーブ・クリア・領域予約にも対応する。

ヘッダーは利用者のマクロ・外部ライブラリ・CPU 固有命令に依存しない。`S` と `F` がデフォルト構築できない場合も扱う。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定。他のコンパイル・テスト・ベンチマークとは同時に実行しない。
- 区間アフィン更新と区間和。型と演算には本ライブラリの `mint` を共通で使用する。
- 操作数 40,000、seed `20261020`。生成・初期構築・最終状態の集約・入出力を計測外にし、操作列と問い合わせ結果の checksum を測定する。
- 比較相手は `dynamic_lazy_segtree.cpp` 内の `PushTree`。同じ添字付き `vector`・保留フラグを使い、問い合わせでも必要に応じて子へ伝播する再帰型の動的遅延セグメント木。分割点は非負で加算が収まるこの測定範囲で `(l+r)/2` を使う。
- 作業領域はどちらも事前予約しない。測定中の容量拡張とノード作成を含む。
- 固定 seed で実行順を入れ替え、各 7 回の中央値。取得結果と最終集約の checksum が全試行で一致することを確認する。

`sparse` は範囲長 $$2^{40}$$、一様乱数の位置から長さ 1～1024 の区間を操作する。`broad` は同じ範囲から両端を一様に選ぶ。`dense` は範囲長 $$2^{17}$$ で同じ生成方法。更新と問い合わせは概ね半分ずつ。

`read_heavy` は範囲長 $$2^{40}$$ で、計測前に全体へ `2x+3` を 1 回作用させ、その後は乱数区間の問い合わせのみを行う。問い合わせだけでノードを増やさない効果を確認するケース。

条件とソースの SHA-256 は各 `environment.json`、全測定値は `samples.csv`、中央値とノード数は `summary.json` に保存した。AtCoder 実機・macOS 実機の速度は未測定。全入力や全ての型での優位性は主張しない。

## 結果

単位は ms。ノード数は全操作後の実ノード数。

| 操作列 | generic 通常 / 本実装 | native 通常 / 本実装 | ノード数 通常 / 本実装 |
| --- | ---: | ---: | ---: |
| sparse | 32.323 / 32.535 | 39.728 / 40.422 | 830,717 / 830,717 |
| broad | 138.484 / 83.425 | 172.788 / 101.635 | 3,650,826 / 1,918,776 |
| read_heavy | 110.618 / 0.572 | 157.283 / 0.880 | 3,649,491 / 1 |
| dense | 27.478 / 22.795 | 31.213 / 24.807 | 179,778 / 135,509 |

広い乱数区間を操作する `broad` では時間を約 40～41%、ノード数を約 47% 削減した。全体更新後の問い合わせだけではノードを 1 個のまま維持できる。狭い区間中心の `sparse` は同程度で、今回の中央値では約 0.7～1.7% 遅かった。

## 検証

- GCC 15.2 / Clang 21.1.8 での単体ヘッダー、単体テスト、既存ライブラリとの同時 include。
- 長さ 0・1・2 冪前後・100、負と正の原点について、区間更新・一点更新・代入・取得・集約・境界探索を配列の素朴実装と比較。
- 非可換な文字列結合と文字の置換作用を組み合わせ、更新の合成順と左右の集約順を検証。
- 符号付き・符号なしの 8 / 16 / 32 / 64 bit の最小値・最大値付近。幅が符号付き 64 bit に収まらない区間を含む。
- 保留中の全体更新に対する取得と境界探索で、ノード数が増えないことを確認。
- 非ゼロの初期値、座標によって異なる初期値、`initial` 省略、デフォルト構築できない `S`・`F`、空区間、コピー・ムーブ・再構築・領域予約・クリア。
- Clang ASan / UBSan。
- GCC / Clang の両方で通常版の公式 19 ケース、巨大配列版の公式 31 ケースが正答。期待出力と全問い合わせを比較。
- Bundle とドキュメントの使用例を両コンパイラでコンパイル。

`check.log`・`sanitize.log`・`validation.log`・`validation.json` に検証結果とソースの SHA-256 を保存した。macOS 実機はローカルでは未検証で、macOS CI の対象。

## 再現

```sh
python3 test/benchmark/dynamic_lazy_segtree_reference.py --cache .verify-helper/cache/dynamic_lazy_segtree_references --output test/benchmark/results/dynamic_lazy_segtree/reference.json
python3 test/benchmark/dynamic_lazy_segtree_reference.py --problem range_affine_range_sum --cache .verify-helper/cache/dynamic_lazy_segtree_references --output test/benchmark/results/dynamic_lazy_segtree/reference_dense.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/dynamic_lazy_segtree/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_lazy_segtree_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_lazy_segtree.py --output test/benchmark/results/dynamic_lazy_segtree/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dynamic_lazy_segtree.py --native --output test/benchmark/results/dynamic_lazy_segtree/reproduced_native
```
