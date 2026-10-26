---
title: "Online Monge Minima"
documentation_of: //components/online_monge_minima.hpp
---

行を順に確定し、その結果を後続の列の評価に使う Monge DP の最小値の列を求めます。行 `row` では列 `[0,row)` を対象にし、選んだ列をコールバックで受け取ります。

```cpp
#include "lib/components/online_monge_minima.hpp"
#include <cassert>
#include <vector>

int n = 6;
std::vector<long long> dp(n);
auto value = [&](int row, int col) {
	return dp[col] + 1LL * (row - col) * (row - col);
};
online_monge_minima(n,
	[&](int row, int old_col, int new_col) {
		return value(row, new_col) < value(row, old_col);
	},
	[&](int row, int best_col) {
		dp[row] = value(row, best_col);
	});
assert((dp == std::vector<long long>{0, 1, 2, 3, 4, 5}));
```

`online_monge_minima(n, better, set)` の返り値は `void` です。`n` は非負の `int` で、行・列の番号は `[0,n)` です。行 `1` から `n-1` の順に、最良の列 `best_col` を選んで `set(row,best_col)` を 1 回ずつ呼びます。行 `0` は処理しないので、その値は呼び出し前に用意してください。`n == 0` または `n == 1` では、どちらのコールバックも呼びません。

`better(row,old_col,new_col)` は、新しい列を選ぶときに `true` を返します。各行で比較が一貫した順序を定める必要があります。最小値なら `<` で比較し、同値では `false` を返すと列番号が小さいものを選びます。`better` に渡される列は全てその行未満で、既に確定した列です。

`set(row,best_col)` の処理後に、`row` 自身を新しい列候補として使います。行を先読みして比較するので、`better` は未確定の行の答えに依存せず、確定済みの列の値と行番号から評価できる必要があります。確定済みの列を後から更新して、その列の比較結果を変えてはいけません。

任意の列 $$x<y$$ について、有効な行 $$row>y$$ を増やしたとき、`better(row,x,y)` は `false` から `true` の方向にだけ変化することが必要です。これは候補を任意に絞っても最小値の列が単調になるための条件です。Monge コストに確定済み列ごとの定数を加えた行列は、この条件を満たします。最小値の列の単調性だけでは足りず、条件の検査は行いません。

各候補が最良になる行区間を単調な包絡線で管理し、新候補との境界を二分探索します。行列を構築せず、`better` で必要な比較だけを行います。

コールバックを $$O(1)$$ とすると、時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。`better` の呼び出し回数は $$O(n\log n)$$、`set` の呼び出し回数は `n >= 1` で `n-1` 回です。`n == 0` または `n == 1` は $$O(1)$$ 時間・空間です。
