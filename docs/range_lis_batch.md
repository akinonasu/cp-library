---
title: Range LIS の一括処理
documentation_of: //components/range_lis_batch.hpp
---

[Range LIS Query](range_lis_query.md) の `range_lis_queries_sqrt` が使う部品です。通常はこの一括関数を利用してください。

`range_lis_detail::batch_sqrt(p,intervals,answers)` は `0,1,...,n-1` の順列を `span<const int>`、区間を `span<const pair<int,int>>` で受け取ります。各区間は `0 <= l <= r <= n` の半開区間 `[l,r)` です。長さ `intervals.size()` の `vector<int>` を `answers` として渡し、空区間の要素は先に0にしておきます。空でない区間の最長増加部分列の長さを対応する位置へ書き込み、`void` を返します。順列・区間は変更せず、空入力や重複する区間にも対応します。順列と区間列の長さはそれぞれ `INT_MAX` 以下とします。

列の長さを $$n$$、区間数を $$q$$ として、時間は $$O(n\sqrt{n}\log n+q\log n)$$、作業空間は $$O(n+q)$$ です。
