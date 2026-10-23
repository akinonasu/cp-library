# Library Checker

| 問題 | 単一ファイル | 判定 |
| --- | --- | --- |
| [Shortest Path](https://judge.yosupo.jp/problem/shortest_path) | [shortest_path.cpp](shortest_path.cpp) | 提出用（C++23） |
| [Kth term of Linearly Recurrent Sequence](https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence) | [kth_term_of_linearly_recurrent_sequence.cpp](kth_term_of_linearly_recurrent_sequence.cpp) | 提出用（C++23） |
| [Stern–Brocot Tree](https://judge.yosupo.jp/problem/stern_brocot_tree) | [stern_brocot_tree.cpp](stern_brocot_tree.cpp) | 提出用（C++23） |
| [Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) | [general_matching.cpp](general_matching.cpp) | 提出用（C++23） |
| [Range Chmin Chmax Add Range Sum](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum) | [range_chmin_chmax_add_range_sum.cpp](range_chmin_chmax_add_range_sum.cpp) | 提出用（C++23） |
| [Sum of Multiplicative Function(Large)](https://judge.yosupo.jp/problem/sum_of_multiplicative_function_large) | [sum_of_multiplicative_function_large.cpp](sum_of_multiplicative_function_large.cpp) | 提出用（C++23） |

`shortest_path.cpp` はファイル全体を C++23 として提出します。再生成は `oj-bundle test/dijkstra_path.test.cpp > submissions/shortest_path.cpp` です。

`kth_term_of_linearly_recurrent_sequence.cpp` もファイル全体を UTF-8・C++23 として提出します。256 KiB 未満です。再生成は通常の `oj-bundle -I . test/fps_linear_recurrence.test.cpp` を使います。展開後の追加圧縮は不要です。judge 上の順位は未確認です。

`sum_of_multiplicative_function_large.cpp` はファイル全体を C++23 として提出します。再生成は `oj-bundle -I . test/multiplicative_prefix_sum_large.test.cpp > submissions/sum_of_multiplicative_function_large.cpp` です。
