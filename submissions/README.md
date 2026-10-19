# Library Checker

| 問題 | 単一ファイル | 判定 |
| --- | --- | --- |
| [Shortest Path](https://judge.yosupo.jp/problem/shortest_path) | [shortest_path.cpp](shortest_path.cpp) | 提出用（C++23） |
| [Kth term of Linearly Recurrent Sequence](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence) | [kth_term_of_linearly_recurrent_sequence.cpp](kth_term_of_linearly_recurrent_sequence.cpp) | 提出用（C++23） |
| [Stern–Brocot Tree](https://judge.yosupo.jp/problem/stern_brocot_tree) | [stern_brocot_tree.cpp](stern_brocot_tree.cpp) | 最有力 |
| [Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) | [general_matching.cpp](general_matching.cpp) | 同率最速の候補 |
| [Range Chmin Chmax Add Range Sum](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum) | [range_chmin_chmax_add_range_sum.cpp](range_chmin_chmax_add_range_sum.cpp) | 比較用、今回は見送り |
| 測定・再生成 | [fastest_candidates.md](../test/benchmark/fastest_candidates.md) | C++23 |

`shortest_path.cpp` はファイル全体を C++23 として提出します。再生成は `oj-bundle test/dijkstra_path.test.cpp > submissions/shortest_path.cpp` です。

`kth_term_of_linearly_recurrent_sequence.cpp` もファイル全体を C++23 として提出します。再生成は `oj-bundle -I . test/fps_linear_recurrence.test.cpp > submissions/kth_term_of_linearly_recurrent_sequence.cpp` です。[検証記録](../test/benchmark/results/kth_term_submission.json)と[既存の速度比較](../test/benchmark/fps_round4.md)があります。judge 上の順位は未確認です。
