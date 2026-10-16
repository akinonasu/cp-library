# Lazy Reversible Treap の比較

## 参考実装

[dynamic_sequence_range_affine_range_sum の最速提出 402977](https://judge.yosupo.jp/submission/402977) を調査しました。2026-10-16 の公式 API では 451 ms でした。葉に最大512要素を持つ平衡木と、mod 998244353 の区間 affine 更新・和に特化した AVX2 演算を使います。

今回の実装は任意のモノイド・更新と非可換な積に対応する implicit treap です。参照提出とは構造・演算・入出力が異なるため、提出時間との直接比較は行っていません。

## 候補

`lazy_reversible_treap_base.hpp` は最初の実装を保存したものです。`lazy_reversible_treap_variants.py` で候補を生成します。

| 候補 | 変更 |
| --- | --- |
| `standard` | 挿入・区間積・更新・反転を split/merge で処理 |
| `direct` | 挿入・区間積・更新を直接走査し、反転は区間を含む最小部分木で処理 |
| `priority64` | 優先度を32 bitから64 bitに拡張 |
| `split_reverse` | `direct` の反転だけ split/merge に戻す |
| `leaf` | `direct` に、葉への遅延タグ・反転タグの省略と値の重複計算の省略を追加 |
| `packed` | `direct` の遅延フラグを要素数・反転フラグと同じ32 bitに格納 |

`*_ordered` は可換性の最適化を無効にし、逆順の積も保持する場合です。`current` は実際のライブラリを直接 include します。全候補は Cartesian tree による線形構築、連続領域のノードプール、削除ノードの再利用を使います。

```sh
python3 test/benchmark/lazy_reversible_treap_variants.py /tmp/treap-variants
g++ -std=c++23 -O2 -DNDEBUG -I . -I /tmp/treap-variants test/benchmark/lazy_reversible_treap.cpp -o /tmp/treap-bench
/tmp/treap-bench 100000 200000 5 > /tmp/treap.csv
```

初期10万要素、挿入・削除・反転・affine 更新・区間和の計20万操作を等確率で生成します。mod は998244353、乱数は `std::mt19937_64(769345)` です。候補の実行順を入れ替えて5回測定し、全候補のチェックサム一致を確認します。`run_ms` は入出力・クエリ生成・初期構築を含みません。予約容量は初期要素数と操作数の和です。

| 入力 | 区間の分布 |
| --- | --- |
| `random` | 2つの端点を一様に選ぶ |
| `local` | 長さ128以下 |
| `whole` | 半数の操作は全区間、残りは一様な端点 |

## 初回の Linux/GCC 比較

[実行ログ](https://github.com/qandori/cp-library/actions/runs/38877555452)。Ubuntu 24.04、Intel Xeon Platinum 8573C、GCC 13.3、C++23、`-O2 -DNDEBUG`。以下は5回の中央値です。

| 候補 | random ms | local ms | whole ms | 確保領域 MB |
| --- | ---: | ---: | ---: | ---: |
| standard | 418.11 | 225.80 | 255.32 | 10.80 |
| direct | 334.40 | 140.74 | 212.67 | 10.80 |
| priority64 | 334.25 | 148.13 | 212.60 | 14.40 |
| split_reverse | 341.86 | 156.95 | 217.55 | 10.80 |
| leaf | 338.86 | 142.39 | 216.23 | 10.80 |

直接走査で約17–38%短縮し、短い区間の反転では最小部分木だけを処理する方式も有効でした。64 bit優先度はメモリを約33%増やし、速度に利点がありませんでした。葉での特別処理も改善が安定しないため採用していません。`direct` の初期構築は約2.3 msでした。

この実行では候補の単体テスト・チェックサム比較・GCC/Clang・ASan/UBSan は成功し、公式検証用コードは入出力 API の呼び出し誤りでコンパイルに失敗しました。検証コードを修正して最終版で再実行しています。

## 最終比較

[実行ログ](https://github.com/qandori/cp-library/actions/runs/38878356617)、測定スナップショット `d3d2542e4cbe27d447e1034dedc415125ebb5793`。Ubuntu 24.04、AMD EPYC 9V74、GCC 13.3、C++23、`-O2 -DNDEBUG`、5回の中央値です。初回とは CPU が異なるため、比較はこの表の中で行います。

| 候補 | random ms | local ms | whole ms | 確保領域 MB |
| --- | ---: | ---: | ---: | ---: |
| standard | 354.46 | 185.56 | 206.18 | 10.80 |
| direct | 281.93 | 119.06 | 173.75 | 10.80 |
| priority64 | 300.00 | 124.07 | 180.29 | 14.40 |
| split_reverse | 283.11 | 129.54 | 173.89 | 10.80 |
| leaf | 279.38 | 118.29 | 177.44 | 10.80 |
| packed | 280.98 | 114.95 | 172.86 | 9.60 |
| current（採用版） | 285.68 | 114.40 | 176.36 | 9.60 |
| direct_ordered | 349.26 | 139.68 | 219.85 | 12.00 |
| packed_ordered | 349.76 | 139.00 | 222.03 | 10.80 |

`packed` と `current` は同じ処理で、後者は実際のヘッダーを直接使っています。数%以内の差を過大評価せず、省メモリ化と短区間での改善を採用理由としています。この区間和・affine 更新の型では、可換時は36から32バイト、逆順の積を保持する場合は40から36バイトへ減りました。採用版は `standard` より約15–38%短縮しています。

assert 有効でも全候補のチェックサムが一致し、採用版は `random` 280.05 ms、`local` 116.72 ms、`whole` 174.79 msでした。単体テストでは配列とのランダム比較、非可換な文字列連結と遅延更新、affine 更新の合成順、空区間、全区間、区間移動・回転、コピー・ムーブ、削除ノードの再利用、`bool`、デフォルト構築不可の型、20万要素の線形構築を確認しています。

GCC・Clang の単体テスト、ASan・UBSan、Library Checker の次の公式ケースはすべて成功しました。

- `dynamic_sequence_range_affine_range_sum`: 各コンパイラ33ケース
- `range_reverse_range_sum`: 各コンパイラ20ケース

採用版と検証スナップショットのソース一致を確認し、測定用ブランチはローカル・リモートとも使用後に削除しています。
