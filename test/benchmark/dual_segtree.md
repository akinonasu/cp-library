# 双対セグメント木

2026-10-19。`DualSegtree<S,F,mapping,composition,id,Commutative>` を追加した。区間更新・一点取得・一点更新・一点代入・全値の取り出しを扱う。

## 参考実装

Library Checker の Range Affine Point Get について、公式 API の AC 実行時間順の先頭 3 件を取得した。一覧・取得日時・ソースの SHA-256 は `results/dual_segtree/reference.json` にある。

| 提出 | 取得時の時間 | 比較した手法 |
| --- | ---: | --- |
| [409050 / nandhagk](https://judge.yosupo.jp/submission/409050) | 52 ms | 多分木、SoA、SIMD を利用できる更新型。入力を 64 件ずつ読み、8 件先の対象を prefetch。取得は各祖先の作用を読んで適用 |
| [269952 / mukundan314](https://judge.yosupo.jp/submission/269952) | 55 ms | 8 分木、AVX2 のアフィン変換、Montgomery 演算。取得時の各段の値を保存し、最後にまとめて計算 |
| [269948 / mukundan314](https://judge.yosupo.jp/submission/269948) | 55 ms | 8 分木と AVX2。取得結果を保持し、最後にまとめて剰余を正規化 |

固定された法・アフィン変換・AVX2 を利用する部分や、結果の出力前の一括処理は、一般の `S`・`F` を受け取ってその場で値を返す API に直接は使わない。本実装は任意の型、非可換の作用、AVX2 非対応環境にも同じ API で対応する二分木とした。祖先の作用を読み取る取得、葉と内部の配列の分離、不要な伝播の省略を比較した。参考提出とのローカルの実行時間比較は行っておらず、最速性は主張しない。

## 実装

- 初期値は葉の実要素数だけ、保留中の作用と有無のフラグは内部ノードだけに保持。
- 非可換更新では、更新区間の境界で必要な経路だけを先に伝播。末尾の 0 bit 数から必要な深さを求める。
- 未更新ノードは作用を直接代入し、単位元との合成を省略。`F` の等値比較は不要。
- `get` は葉から根へ作用を適用し、内部状態を変更しない。`const` で取得できる。
- 更新が可換なら、区間更新前の伝播を省略。一点更新は直接葉に作用させる。
- 長さ 1 の区間は一点更新に渡し、両端の伝播を重複して行わない。
- 一点代入は経路の作用を伝播してから上書き。`values` は全体を 1 回伝播してコピーする。

`S`・`F` がデフォルト構築できない場合も検証した。ヘッダーはテンプレート・利用者のマクロ・外部ライブラリに依存しない。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定し、他のコンパイル・テスト・ベンチマークと同時に測定しない。
- $$n=131072$$、$$q=300000$$、seed `20261019`。生成と構築を計測外とし、操作列と取得結果の checksum 計算を測る。最後の全要素の照合も計測外。
- 各比較を 9 回実行し、順序を固定 seed で入れ替える。中央値を採り、全取得結果と最終状態の checksum が一致することを確認。
- 比較相手は `dual_segtree.cpp` 内の `BasicDual`。区間更新の両端と一点取得で毎回根から経路を伝播する通常の二分木実装。法・作用・フラグ・要素配列の分離は本実装と同じ。

AtCoder 実機・macOS 実機での速度は未測定。全入力での優位性は主張しない。入出力は以下の測定に含まない。

## 結果

単位は ms。最終版は `final_generic/`・`final_native/`。取得位置は `hot` 以外では一様乱数。`query_heavy` 以外は更新と取得を概ね半分ずつ生成する。

| 操作列 | 内容 | generic 通常 / 本実装 | native 通常 / 本実装 |
| --- | --- | ---: | ---: |
| random | 一様乱数で選んだ区間へのアフィン変換 | 67.975 / 57.859 | 67.797 / 57.737 |
| full | 全区間へのアフィン変換 | 43.094 / 3.580 | 43.482 / 3.864 |
| point | 長さ 1 の区間へのアフィン変換 | 4.957 / 3.121 | 4.353 / 3.064 |
| prefix | 接頭辞へのアフィン変換 | 57.989 / 37.960 | 58.235 / 38.024 |
| query_heavy | random と同じ区間、取得が概ね 9 割 | 35.860 / 24.935 | 35.760 / 25.165 |
| hot | 32 操作ごとに全区間を更新し、それ以外は中央の点を取得 | 4.662 / 2.395 | 4.160 / 2.720 |

加算だけの `add` ではアフィン変換の係数を 1 に固定し、同じ入力を通常実装・既定の非可換設定・可換設定で比較した。

| ビルド | 通常実装 | 非可換設定 | 可換設定 |
| --- | ---: | ---: | ---: |
| generic | 67.350 | 58.054 | 24.193 |
| native | 67.175 | 56.678 | 24.368 |

初期版の `generic/`・`native/` では、長さ 1 の区間にも両端の処理を行い、point が通常実装より約 3.6% / 13.5% 遅かった。この結果を残したうえで、一点更新の処理に共通化して再測定した。他の区間や更新型にも同じ条件で適用できる処理であり、入力ファイル名や特定の値を条件にしていない。

コンパイラ・CPU 情報、フラグ、ソースの SHA-256 は各 `environment.json`、全測定値は `samples.csv`、中央値は `summary.json` にある。

## 検証

- GCC 15.2 / Clang 21.1.8 でヘッダー単体、単体テスト、既存ヘッダーとの同時 include。
- 空列・長さ 1・2 冪前後・端点・空区間・全区間・初期値・再構築・コピー・移動後の再構築。
- 18 種類の長さで非可換アフィン変換を各 10,000 回、配列への直接適用と比較。係数 0 による代入、一点更新、一点代入、繰り返しの const 取得、全値の取り出しを含む。
- 9 種類の長さで加算を各 10,000 回実行し、配列・可換版・非可換版を比較。
- `S`・`F` のデフォルト構築と `F` の等値比較を使わない型で検証。
- Clang の ASan / UBSan。
- 両コンパイラで Range Affine Point Get の公式 19 ケースが正答。Bundle と説明の 2 つの使用例も確認。

`check.log`・`sanitize.log`・`validation.log`・`validation.json` に記録。macOS 実機はローカルでは未検証で、macOS CI の対象。

## 再現

```sh
python3 test/benchmark/dual_segtree_reference.py --cache .verify-helper/cache/dual_segtree_references --output test/benchmark/results/dual_segtree/reference.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/dual_segtree/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dual_segtree_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dual_segtree.py --output test/benchmark/results/dual_segtree/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/dual_segtree.py --native --output test/benchmark/results/dual_segtree/reproduced_native
```
