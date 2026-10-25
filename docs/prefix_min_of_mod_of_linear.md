---
title: Mod of Linear の Prefix Minimum
documentation_of: //math/prefix_min_of_mod_of_linear.hpp
redirect_from:
- /math/floor_sum/prefix_min_of_mod_of_linear.hpp.html
- /lightweight/math/floor_sum/prefix_min_of_mod_of_linear.hpp.html
library_category: math/floor_sum
---

```cpp
#include "lib/math/prefix_min_of_mod_of_linear.hpp"

auto parts = prefix_min_of_mod_of_linear(100, 100, 71, 85);
```

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `prefix_min_of_mod_of_linear(n,m,a,b)` | 最小値が更新される位置・値を圧縮した `vector<ModLinearPrefixMinSegment>` を返す | $$O(\log m)$$ |
| `prefix_min_of_mod_of_linear(n,m,a,b,emit)` | 各区間に対して `emit(part)` を呼ぶ。出力配列を確保しない | $$O(\log m)$$ |
| `part.x` / `part.y` / `part.dx` / `part.dy` / `part.count` | 最初の位置・値／位置の増分／値の減少幅／更新点数 | $$O(1)$$ |
| `part == other` / `part != other` | 全フィールドの比較 | $$O(1)$$ |

$$f(x)=(ax+b)\bmod m$$ の $$0\le x<n$$ において、$$x=0$$ または $$\displaystyle f(x)<\min_{0\le i<x} f(i)$$ を満たす位置・値を等差数列に圧縮して返します。同じ最小値が再び現れる位置は含めません。

引数と各フィールドは `std::uint64_t`、$$m>0$$ です。$$a,b$$ は内部で正規化します。$$n=0$$ では空の結果です。

返り値は `std::vector<ModLinearPrefixMinSegment>`。各要素のフィールドは `x, y, dx, dy, count` です。
$$0\le j<\mathrm{count}$$ に対する位置・値 $$(x+\mathrm{dx}\,j,\ y-\mathrm{dy}\,j)$$ を表し、位置の昇順で、区間どうしの重複はありません。
更新がなければ `{0, b % m, 0, 0, 1}` を返します。

上の例では更新点は $$(0,85),(1,56),(2,27),(6,11),(13,8),(20,5),(27,2),(65,0)$$ です。
任意の prefix の最小値は、その末尾以下の最後の更新点の値です。

各更新点を取り出すには、上の `parts` に対して次のように走査します。`dy` は値の減少幅なので、値の計算では引き算にします。

```cpp
for (const auto &part : parts) {
	for (std::uint64_t j = 0; j < part.count; ++j) {
		auto position = part.x + part.dx * j;
		auto minimum = part.y - part.dy * j;
	}
}
```

この列挙は圧縮を展開するため、時間は更新点の総数に比例します。圧縮したまま処理したい場合は、各 `part` を一つの等差数列として扱います。

第5引数にコールバックを渡すと、動的確保せず各区間を順に渡します。
バッファを繰り返し使う場合にも利用できます。

```cpp
std::vector<ModLinearPrefixMinSegment> buffer;
buffer.reserve(64);
buffer.clear();
prefix_min_of_mod_of_linear(100, 100, 71, 85, [&](ModLinearPrefixMinSegment part) {
	buffer.push_back(part);
});
```

両形式とも時間 $$O(\log m)$$。返す区間数・vector 版の空間は $$O(\log m)$$、コールバック版の追加空間は $$O(1)$$ です。コールバック1回を $$O(1)$$ とします。
