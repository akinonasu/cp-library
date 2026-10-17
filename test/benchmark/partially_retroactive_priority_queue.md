# Partially Retroactive Priority Queue

## 参考文献・比較対象

[Demaine, Iacono, Langerman (2007), Retroactive Data Structures, §5.4](https://erikdemaine.org/papers/Retroactive_TALG/paper.pdf) の橋を使う部分的 retroactive priority queue を独立に実装しました。

現在まで残る挿入の重みを0、削除済みの挿入を1、削除操作を−1とすると、累積重みが0の場所が橋になります。履歴変更による現在の生存集合の変化は高々1要素です。直前の橋以降の削除済み要素の最大値、または直後の橋以前の生存要素の最小値を使い、その要素だけを移します。

履歴を時刻順の treap に保存し、重みの和・最小累積値、キュー個数の和・最小累積値、生存要素の最小値、削除済み要素の最大値を各部分木に保存します。現在の最小値・要素数は根から直接取得します。履歴の不正判定も部分木の要約だけで行います。専用の値順木は作らず、連続ノードプールと削除領域の再利用で管理します。乱数生成には既存の `SplitMix64` を使います。

2026-10-16 時点で Library Checker の専用問題は[提案 #1213](https://github.com/yosupo06/library-checker-problems/issues/1213)で、対応する fastest 提出はありません。[Double-Ended Priority Queue](https://judge.yosupo.jp/problem/double_ended_priority_queue) に対し、min/max の2個の構造を使う検証を用意しました。一方の削除に合わせて他方の挿入操作を過去に遡って取り消します。この問題の専用 heap との速度比較ではなく、過去の操作取消を含む正当性の検証です。

## 比較候補

- `separate_incremental`: 橋の位置を求めた後で範囲の最大・最小値を探索。変更対象の所属部分木も時刻で調べる。初期構築も逐次更新。
- `separate_bulk`: 探索は同じ。初期生存集合を heap で計算し、Cartesian tree を線形時間で構築する。
- `fused_bulk`: 橋を探す経路で最大・最小値も同時に集約する。
- `current`: 検索結果に所属部分木も持たせ、追加の時刻探索を省く。
- `fully256`: 既存の fully retroactive 版、ブロック幅256。今回と同じ現在時点だけの問い合わせを行う。

どの partially 版も更新は償却期待 $$O(\log m)$$、現在時点の問い合わせは $$O(1)$$ です。逐次構築をまとめて処理することで構築時の木の更新回数を減らし、探索の融合で更新時の木の走査回数を減らします。

## 再現方法

この比較候補の生成には、変更通知を追加する前の `2a4d721b9a8fc2ed25a7cf05ff7312782a162653` のヘッダーと単体テストを使う。変更通知の比較は [変更通知の測定](retroactive_priority_queue_changes.md) を参照。

```sh
python3 test/benchmark/partially_retroactive_priority_queue_variants.py /tmp/partial-pq-variants
g++ -std=c++23 -O2 -DNDEBUG -I . -I /tmp/partial-pq-variants test/benchmark/partially_retroactive_priority_queue.cpp -o /tmp/partial-pq-bench
/tmp/partial-pq-bench 100000 10000 5 > /tmp/partial-pq.csv
```

10万操作の初期履歴を構築し、1万回の履歴変更それぞれの直後に現在の `top`・`size` を問い合わせます。乱数は `std::mt19937_64(5834509)`、値は64 bit整数です。変更は挿入・削除操作の追加・値変更・操作取消を等確率で選び、拒否された変更も含みます。計測は入出力と入力生成を含みません。`build_ms` は構築と `reserve`、`mixed_ms` は1万回の変更・問い合わせの合計です。

- `growing`: 初期履歴は挿入だけ。
- `balanced`: 最初の4分の1が挿入で、その後は削除・挿入を交互に行う。
- `steady`: 最初の512回が挿入で、その後は削除・挿入を交互に行う。

実行順を入れ替えて5回測定し、全候補の成功可否・最小値・要素数のチェックサムを照合します。小規模入力ではベンチマーク内でも独立した `multiset` による全履歴の再実行と照合します。

## Linux/GCC の測定結果

[実行ログ](https://github.com/qandori/cp-library/actions/runs/38887726714)、測定スナップショット `cfcd8768103b94c310e30978356badabccabe7b5`。2026-10-16、Ubuntu 24.04、AMD EPYC 7763、GCC 13.3、C++23、`-O2 -DNDEBUG`。以下は5回の中央値です。

| 入力 | 候補 | 構築 ms | 変更＋問い合わせ ms | 確保領域 MB |
| --- | --- | ---: | ---: | ---: |
| growing | separate_incremental | 46.14 | 18.39 | 7.92 |
| growing | separate_bulk | 11.61 | 18.14 | 7.92 |
| growing | fused_bulk | 11.57 | 17.80 | 7.92 |
| growing | current | 11.66 | 18.11 | 7.92 |
| growing | fully256 | 24.14 | 636.97 | 7.34 |
| balanced | separate_incremental | 68.70 | 18.75 | 7.92 |
| balanced | separate_bulk | 16.57 | 18.47 | 7.92 |
| balanced | fused_bulk | 16.54 | 19.02 | 7.92 |
| balanced | current | 16.67 | 18.06 | 7.92 |
| balanced | fully256 | 18.48 | 2239.32 | 7.60 |
| steady | separate_incremental | 57.52 | 17.19 | 7.92 |
| steady | separate_bulk | 13.48 | 17.02 | 7.92 |
| steady | fused_bulk | 13.50 | 17.22 | 7.92 |
| steady | current | 13.50 | 16.48 | 7.92 |
| steady | fully256 | 16.42 | 133.59 | 7.67 |

一括構築は逐次更新の約4.0–4.3倍速くなりました。橋と範囲集約の融合だけでは安定した改善は見られませんでしたが、所属部分木の探索も省いた `current` は、基準実装に対して全3入力で同等以上でした。基準実装からの更新時間の差は約1.5–4.2%にとどまり、大きな改善は fully 版から部分的 retroactive 版への計算量の変更によるものです。

同じ現在状態だけの問い合わせでは、`current` は `fully256` の約8.1–124倍速くなりました。メモリは約7.92 MBで、fully 版の約7.34–7.67 MBより少し増えます。操作成功件数は順に8,622・7,693・7,351件でした。

assert 有効の測定でも全候補・全試行のチェックサムが一致しました。`current` の変更＋問い合わせは、順に19.18・18.42・16.31 msでした。

## 検証

GCC・Clang の単体テストと ASan・UBSan、独立した時系列シミュレーターとの3万2千回のランダム変更、7時刻・2種類の値を使う全16,384パターンとその操作取消を照合しました。同順位・状態付き比較・非デフォルト構築型・時刻端点・無効履歴の拒否・コピーとムーブ・ノード再利用も確認しています。一括構築後にもランダム変更を続ける検証を追加しました。

Double-Ended Priority Queue の全18ケースを GCC・Clang で通過しました。過去の挿入取消はこの問題で、過去の挿入・削除操作の追加や値変更は単体テストと比較ベンチマークで検証しています。

追加した一括構築後のランダム変更は[別実行](https://github.com/qandori/cp-library/actions/runs/38888090912)で検証しました。成功済みの計測・外部問題テストは再実行していません。ヘッダー・検証コードと測定スナップショットの一致を確認し、一時測定ブランチはローカル・リモートの両方から削除しました。
