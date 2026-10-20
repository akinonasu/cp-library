# Rollback Union Find

2026-10-20。`data_structure/rollback_union_find.hpp` を追加した。union by size を使い、経路圧縮は行わない。検索は読み取りのみで、各 `merge` と `undo` が1対1に対応する。snapshot・巻き戻し・成分数・成分列挙に対応する。標準 C++ のみを使う。

## 参考提出

Library Checker の [Persistent Unionfind](https://judge.yosupo.jp/problem/persistent_unionfind) で AC 上位20件を取得し、C++ 上位3件を確認した。一覧・取得時刻・ソースの SHA-256 は [references.json](results/rollback_union_find/references.json)。

| 提出 | 取得時の最大時間 | 確認した方式 |
| --- | ---: | --- |
| [409831](https://judge.yosupo.jp/submission/409831) | 15 ms | バージョン木の不要分岐を削除し、兄弟がある場所だけ保存。巻き戻しの段が同じ親同士では経路を半分に短縮 |
| [404578](https://judge.yosupo.jp/submission/404578) | 18 ms | バージョン木を CSR にし、DFS と Rollback Union Find で処理。2頂点・元の値2個・併合の有無を履歴に保存 |
| [218108](https://judge.yosupo.jp/submission/218108) | 18 ms | 同じ Rollback Union Find とオフラインのバージョン木探索 |

最速提出の経路短縮は、専用のバージョン木探索における巻き戻し段を使う。一般の `merge` 1回を自由に取り消す API へそのまま入れると、検索による親の変更も復元する必要がある。今回は検索を const に保ち、サイズに基づく併合で深さを制限した。ドライバーはバージョン木を反復 DFS し、深い入力でも再帰を使わない。

## 履歴

併合された子の頂点と、その子の元の負のサイズだけを保存する。取り消し時には `parent[child]` が併合先を指しているので、その根のサイズから子のサイズを引き戻し、子を根へ戻す。後から行った変更を先に戻し、経路圧縮をしないため、この復元が可能。成功しない併合も番兵で記録する。

32 bit `int` では1操作8バイト。比較用の版では同じクラスから履歴の形式だけを変え、両方の根と元の値を保存する16バイトの履歴にした。生成コードは計測スクリプト内にある。併合の成功・連結判定・最後の履歴長・成分数をチェックサムとともに照合した。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。`-std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero -DONLINE_JUDGE`。ウォームアップ1回、7回測定し、各ラウンドの順番を固定 seed でシャッフル。他のコンパイル・テストと同時には測定しない。

公式生成器の commit は `1814c4e5205517e368bb57a8d1127eb961cfeaae`。対象ソース・各入力のハッシュ、CPU・コンパイラ、全試行は [measurements.json](results/rollback_union_find/measurements.json)。Mac での速度は未測定。

次は入力後から、オブジェクトの構築・履歴の reserve・全操作を計測した中央値、ms。入力・出力・破棄は含めない。

| 操作列 | 今回の8バイト履歴 | 16バイト履歴 |
| --- | ---: | ---: |
| 併合100回と問い合わせ20回の分岐を2000回 | 0.637 | 0.642 |
| 平衡に併合した後、30万回問い合わせ | 11.737 | 12.066 |
| 連結済み頂点への併合を多数含む | 1.191 | 1.734 |
| 併合・問い合わせ・undo・部分 rollback の混在 | 2.286 | 2.348 |

今回の条件では同程度か短縮したが、操作列によって差がある。常に一定倍率で速いという結果ではない。

次は公式入力で、プロセス起動・入力・バージョン木構築・探索・出力・終了を含む中央値、ms。通常ファイルから入力し、出力は `/dev/null`。最速提出は不要な枝の削除や専用の経路短縮も行うため、Union Find 単体の比較ではない。

| 入力 | 今回のドライバー | 提出409831 |
| --- | ---: | ---: |
| max_random_00 | 13.74 | 11.50 |
| max_random_04 | 13.07 | 11.41 |
| conchon_filliatre_tle_00 | 10.19 | 7.98 |
| conchon_filliatre_tle_01 | 10.31 | 8.74 |

この比較では最速提出の方が速かった。公開ジャッジ時間との直接比較や、オンラインでの AC・最速を主張するものではない。

## 検証

- 4頂点・深さ3の全ての併合列を、成分ラベルのコピーと比較。
- ランダム200通り、各500操作。併合・undo・任意の祖先への巻き戻し・分岐・コピー・resetを照合。
- 全頂点対の連結性・成分サイズ・成分数・groups・履歴長を確認。検索や成分列挙が履歴を増やさないことも確認。
- 空の状態、自己ループ、既に連結な頂点への併合、同じ状態への rollback、履歴全体の取り消し。
- 262144頂点の平衡な併合を全て取り消し、全頂点が独立に戻ることを確認。
- GCC 15.2.0・Clang 21.1.8、Clang ASan/UBSan。
- Persistent Unionfind の公式14ケースを両コンパイラで確認。参考提出409831も全14ケースで正答を確認。
- 説明内コードの実行、GitHub Pages の生成、ヘッダーと提出コードの Bundle。

範囲は [selection.json](results/rollback_union_find/selection.json)、結果は同じディレクトリの `unit.log`・`sanitize.log`・`docs.log`・`benchmark.log`。

```sh
python3 test/benchmark/rollback_union_find.py --references
flock /tmp/cp-wm-opt.lock python3 test/benchmark/rollback_union_find.py
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/rollback_union_find/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/rollback_union_find/selection.json
```
