# 永続セグメント木

2026-10-21。`PersistentSegtree<S,op,e>` を追加。任意の過去版からの一点更新、区間積、境界探索、版の間の区間コピーを扱う。

## 参考提出

Library Checker の公式 API で、対応する問題の AC を実行時間順に各 3 件取得し、下記の実装を中心に調べた。取得条件・提出情報・ソースの SHA-256 は [reference.json](results/persistent_segtree/reference.json) に保存した。下記の時間は取得時のジャッジ上の値であり、本実装との時間比較には使っていない。

| 問題・提出 | 取得時の時間 | 確認した方法 |
| --- | ---: | --- |
| Persistent Range Affine Range Sum [307048 / polyominal](https://judge.yosupo.jp/submission/307048) | 189 ms | `vector` のノードプール、32 bit の子添字、経路コピー、部分木の共有による区間コピー。遅延作用も各ノードに持つ |
| 同 [401007 / Rohan_Kapri](https://judge.yosupo.jp/submission/401007)・[307049 / polyominal](https://judge.yosupo.jp/submission/307049) | 192 / 198 ms | 同じ問題の高速な AC として取得。区間アフィン更新を含むため、今回の一点更新 API とは操作が異なる |
| Persistent Unionfind [409831](https://judge.yosupo.jp/submission/409831) | 15 ms | 先読みした版の木を走査し、必要な分岐で Union Find を巻き戻す。汎用のオンライン永続配列とは異なる |
| Rectangle Sum [393934 / chaihf](https://judge.yosupo.jp/submission/393934) | 84 ms | 提出の `main` は先読みしたクエリを Fenwick Tree の走査線で処理。今回の版を保持する API とは用途が異なる |

本実装も 32 bit の添字と連続したノード領域を採用した。遅延作用を持たないため、点更新にはノードの値と左右の子だけを持つ。点更新は経路を反復的にたどり、下から必要なノードだけを作る。区間積も分岐点まで反復し、左右の端を prefix / suffix の積として求める。

最速提出への勝利や AtCoder 実機での速度は主張しない。公式ケースでの正答確認と、次の同条件の局所比較を行った。

## 測定

- Linux x86_64 / WSL2、Intel Core i7-13700F、GCC 15.2.0。
- generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native はこれに `-march=native -ftrivial-auto-var-init=zero` を追加。CPU 2 に固定し、他の検証・コンパイルとは同時に実行しない。
- 和を持つ長さ 131,073 の疎な初期版から、160,000 操作。seed は `20261021`。
- 比較相手は `persistent_segtree.cpp` 内の `Baseline`。同じ 32 bit 添字の `vector`、空ノード共有、半開区間の分割点を使う再帰型の経路コピー実装。
- 両方とも必要な最大ノード数を事前に予約。操作列の生成と領域予約を計測から除き、操作の実行と回答の保存を計測する。
- 各 5 回、実行順を交互に変えた中央値。全回答と作成ノード数を毎回比較し、不一致なら測定を失敗させる。
- `latest` は最新の版から更新、`branching` は過去版を一様に選んで更新。どちらも一点代入と区間積が約半分ずつ。
- `copy_mix` は過去版からの区間コピーを約 1/4 含む。`read_heavy` は計測前に最新の版へ 20,000 回の更新を行い、計測中は過去版の区間積だけを求める。

| 操作列 | 再帰型 | 本実装 | 時間比 | ノード数 |
| --- | ---: | ---: | ---: | ---: |
| latest | 32.196 ms | 25.391 ms | 0.789 | 1,437,912 |
| branching | 22.623 ms | 20.659 ms | 0.913 | 1,433,917 |
| copy_mix | 24.134 ms | 23.078 ms | 0.956 | 1,264,341 |
| read_heavy | 33.994 ms | 24.672 ms | 0.726 | 360,000 |

native の同じ操作列でも比較した。

| 操作列 | 再帰型 | 本実装 | 時間比 |
| --- | ---: | ---: | ---: |
| latest | 32.402 ms | 26.592 ms | 0.821 |
| branching | 22.549 ms | 21.589 ms | 0.957 |
| copy_mix | 21.781 ms | 21.158 ms | 0.971 |
| read_heavy | 33.976 ms | 24.008 ms | 0.707 |

条件・ソースの SHA-256・全測定値は [generic.json](results/persistent_segtree/generic.json) と [native.json](results/persistent_segtree/native.json)。全ての型・操作比率での優位性を保証する測定ではない。

## 検証

- GCC 15.2 / Clang 21.1.8 の独立ヘッダー、単体テスト、既存ライブラリとの同時 include。
- 任意の過去版からの代入・合成・区間コピーを配列の素朴実装と比較。空、長さ 1、2 冪の前後、`INT_MAX` の疎な範囲を含む。
- 非可換な文字列結合で、左右の順序と単調な述語による境界探索を検証。
- 読み取りでノード数が増えないこと、更新経路のノード数上限、再確保、木のコピー・ムーブ、デフォルト構築できない値の型を検証。
- Clang ASan / UBSan。
- Persistent Unionfind の公式 14 ケース、Rectangle Sum の公式 14 ケースをそれぞれ GCC / Clang で正答。
- 説明の使用例を両コンパイラで実行し、182 件のドキュメント metadata と新しい Bundle を検証。結果は [validation.json](results/persistent_segtree/validation.json)。

```sh
python3 test/benchmark/persistent_segtree.py --output test/benchmark/results/persistent_segtree/generic.json
python3 test/benchmark/persistent_segtree.py --native --output test/benchmark/results/persistent_segtree/native.json
```
