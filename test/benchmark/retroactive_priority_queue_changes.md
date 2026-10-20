# Retroactive Priority Queue の変更通知

## 目的と参考資料

現在の集合の最小値だけでは、[ABC363 G](https://atcoder.jp/contests/abc363/tasks/abc363_g) の報酬和を取得できなかった。`PartiallyRetroactivePriorityQueue` の更新に変更通知を追加し、内部を変更せずに総和・XOR・頻度・外部の集合などを管理できるようにした。要素型や比較関数に集計用の特殊な副作用を要求しない。

[noshi91 による解説](https://atcoder.jp/contests/abc363/editorial/10496) は、日付を後ろから処理する最大優先度キューと、現在残る集合の変化を管理する方法を説明している。[Demaine らの論文 §5.4](https://erikdemaine.org/papers/Retroactive_TALG/paper.pdf) の橋による更新法は従来どおり使う。今回の変更は集計を取り出す API で、構築アルゴリズムの置き換えではない。

2026-10-18 に Library Checker の [専用問題の提案 #1213](https://github.com/yosupo06/library-checker-problems/issues/1213) を再確認したが、まだ Open で専用の fastest 提出は取得していない。ABC363 G の実行時間順 AC 一覧も取得を試みたがログインへ転送された。解説から参照される [55828915](https://atcoder.jp/contests/abc363/submissions/55828915) は 403 となり、ソースを取得・測定できなかった。取得結果は `environment.json` に残す。最速提出との比較や AtCoder 実機での AC は主張しない。

## API

`insert(t,x,f)`・`insert_pop(t,f)`・`set(t,x,f)`・`erase(t,f)` は、現在の集合に増減する要素について `f(挿入時刻,const T&,bool added)` を呼ぶ。通知はコピーを挟まず、コールバックをキューに保存しない。ノードのフィールドも増やしていない。失敗した変更では通知しない。

挿入・pop 追加・操作取消は 1 要素だけが増減する。値変更は追加と削除を各高々 1 回、削除を先に通知する。内部の「新しい push を追加して古い push を取り消す」途中の状態は隠し、同じ要素の一時的な追加と削除を相殺する。時刻をキーとする外部 map も正しく更新できる。

`assign(updates,f)` は旧集合を全て削除して新集合を追加する。検証に失敗した場合は通知しない。`clear(f)` は現在の全要素を削除し、`for_each(f)` は現在の全要素を挿入時刻順に渡す。通知なしの `assign` では列挙をコンパイル時に省く。

コールバックが `O(1)` なら、逐次更新は償却期待 `O(log m)` のまま。空間も従来どおりで、通知なし API と要素型・時刻型・比較関数のテンプレート引数は維持する。任意の過去時点をクエリる fully retroactive 版は変更していない。

## ABC363 G

[ドライバー](retroactive_priority_queue_scheduling.cpp) と [公開 API だけを使う実装](../fixtures/retroactive_scheduling.hpp) を用意した。

- 日付を `N,N-1,...,1` の順に並べ、締切がその日の仕事を push した後、その日の最大報酬を pop する。
- 報酬 0 の予備要素を最初に `N` 個入れる。これで仕事がない日の pop と、更新中の一時的な仕事取消も有効な履歴になる。
- 通知で現在残る報酬和を保ち、全仕事の報酬和から引く。
- 締切変更は旧 push の取消と新 push の追加、報酬だけの変更は `set` を使う。

時刻を `N + (N-D_i)*(N+1) + i` とし、各日の pop はその日最後に置く。時刻は 64 bit。履歴は常に高々 `3N` 操作で、クエリを先読みせず、全体が償却期待 `O((N+Q) log N)` 時間、`O(N)` 空間になる。

## 測定条件

2026-10-18、WSL2 / Linux x86_64、Core i7-13700F、GCC 15.2.0。CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native -ftrivial-auto-var-init=zero`、assert 有効。ウォームアップ 1 回、7 試行の中央値。測定中は他のコンパイル・テストを実行していない。AtCoder 実機の時間ではない。

構築部分は 10 万操作の初期履歴と 1 万更新。入力 seed 5834509、実行順 seed 10496。入出力と入力生成は除き、更新後の最小値・個数・成功可否の checksum を含む。`before` は変更前の `2a4d721b9a8fc2ed25a7cf05ff7312782a162653`、`current` は通知なし、`notified` は通知で総和を計算する。実行順をシャッフルし、成功件数・checksum・確保領域が全候補で一致することも確認する。

| 入力 | 指標 | before | current | notified |
| --- | --- | ---: | ---: | ---: |
| growing | 構築 ms | 10.7761 | 11.0507 | 12.4318 |
| growing | 変更 ms | 14.6395 | 14.2905 | 15.4511 |
| balanced | 構築 ms | 14.2066 | 14.4097 | 15.2845 |
| balanced | 変更 ms | 14.6397 | 14.5413 | 15.1376 |
| steady | 構築 ms | 11.7840 | 12.4452 | 12.5228 |
| steady | 変更 ms | 13.8918 | 14.8222 | 14.1061 |

確保領域は全候補で 7,920,144 byte。通知なしの steady は変更前より約 6.7% 遅い結果だった。通知なしも完全に無償とする結果ではない。通知ありの結果も入力やコード生成に依存するため、単一の上乗せ率は主張しない。

ABC363 G は `N=Q=100000`、seed 36310496。Linux `/tmp` の通常ファイルで stdin/stdout をリダイレクトし、プロセス起動・入力・初期構築・全更新・出力・終了を含む。入力生成、コンパイル、出力の SHA-256 計算は時間外。全試行の出力ハッシュを照合し、Clang の独立した実行とも一致させた。

| 締切の入力 | 中央値 ms |
| --- | ---: |
| random：初期値・更新ともランダム | 371.7810 |
| one：全て 1 | 218.2681 |
| all：全て N | 276.2455 |
| alternating：1 と N を交互に生成 | 278.3780 |

これは生成した最大サイズでの動作確認で、AtCoder の隠しケースや AC 記録ではない。

## 検証

- 従来の単体テスト、GCC 13.4 / 15.2、Clang 21.1.8、独立ヘッダーとマクロ非依存。
- 通知を受け取る外部 map・総和と、全履歴を再実行する独立 oracle を毎回照合。min/max、unsigned 時刻、同順位、文字列 payload、非デフォルト構築型、コピー不可コールバック、拒否操作、再構築、clear、列挙を検証。
- ABC363 G の全 3 サンプル、独立した締切昇順の貪欲解との 100 ケース × 300 更新。8 要素以下では全ての仕事部分集合も列挙して検証。
- 10 万仕事で締切 1・N、報酬 `10^9`、2000 更新ずつを検証。
- ASan/UBSan と NDEBUG。GCC と Clang でドライバーの全サンプル、Clang で単一ソースへの展開と NDEBUG ビルド。
- 既存の [Double-Ended Priority Queue](https://judge.yosupo.jp/problem/double_ended_priority_queue) の公式 18 ケースを GCC・Clang の両方で通過。

[全試行](results/retroactive_priority_queue_changes/results.json)・[条件とソースハッシュ](results/retroactive_priority_queue_changes/environment.json)。

```sh
python3 test/benchmark/retroactive_priority_queue_changes.py --output test/benchmark/results/retroactive_priority_queue_changes
oj-bundle test/benchmark/retroactive_priority_queue_scheduling.cpp > /tmp/abc363_g.cpp
g++ -std=c++23 -O2 /tmp/abc363_g.cpp -o /tmp/abc363_g
```
