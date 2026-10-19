---
title: "Mo 法の処理順 (クエリ先読み)"
documentation_of: //components/mo_order.hpp
---

クエリを全て先読みし、区間をまとめて渡します。区間の左端をブロックに分け、右端をブロックごとに昇順・降順へ交互に並べます。区間そのものは変更せず、元のクエリ番号を処理したい順で返します。

```cpp
#include "lib/components/mo_order.hpp"

std::vector<std::pair<int,int>> ranges{ {0,3}, {2,5}, {0,1} };
auto order = mo_order(ranges);
for (int id : order) {
	auto [l,r] = ranges[id];
	// 現在の区間を [l,r) へ動かし、答えを answer[id] に保存
}
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `mo_order(ranges,block_size=0)` | `vector<pair<int,int>>` の区間を処理する順番を、元の添字の `vector<int>` で返す | $$O(Q\log Q)$$ |

各区間は `0 <= l <= r`、区間数は `INT_MAX` 以下、ブロック幅は非負にします。0 の場合は、左端と右端それぞれの値域の幅と区間数から幅を選びます。空の入力は空の結果を返します。

区間数を $$Q$$ として、$$O(Q\log Q)$$ 時間、$$O(Q)$$ 空間です。
