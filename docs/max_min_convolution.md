---
title: "max-min・min-max 畳み込み"
documentation_of: //convolution/max_min_convolution.hpp
library_category: convolution
---

```cpp
#include "lib/convolution/max_min_convolution.hpp"

std::vector<long long> a{3, 1, 4}, b{2, 5};
auto c = max_min_convolution(a, b); // {2,3,2,4}
auto d = min_max_convolution(a, b); // {3,2,4,5}
```

入力長の合計を $$L=\lvert a\rvert+\lvert b\rvert$$ とし、要素の比較・コピーを定数時間として計算量を示します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `max_min_convolution(a,b)` | $$\displaystyle c_k=\max_{i+j=k}\min(a_i,b_j)$$ | $$\displaystyle O(L\sqrt{L\log L})$$ |
| `min_max_convolution(a,b)` | $$\displaystyle c_k=\min_{i+j=k}\max(a_i,b_j)$$ | $$\displaystyle O(L\sqrt{L\log L})$$ |
| `max_min_convolution(a,b,compare)` | `compare` で定める順序で max-min 畳み込みを計算する | $$\displaystyle O(L\sqrt{L\log L})$$ |
| `min_max_convolution(a,b,compare)` | `compare` で定める順序で min-max 畳み込みを計算する | $$\displaystyle O(L\sqrt{L\log L})$$ |

例えば `c[1]` は `max(min(a[0],b[1]),min(a[1],b[0])) = 3` です。返り値は入力と同じ型の配列で、両方が非空なら長さは `a.size()+b.size()-1`、どちらかが空なら空配列です。入力は変更しません。

`max_min_convolution` は $$\displaystyle \max_{i+j=k} \min(a_i,b_j)$$、`min_max_convolution` は $$\displaystyle \min_{i+j=k} \max(a_i,b_j)$$ を計算します。係数には大小比較できる型を使えます。第 3 引数を省略した場合は、型の通常の大小関係を使います。出力長は $$2^{23}$$ 以下です。

第 3 引数には `compare(const T &x,const T &y)` の形で呼べる関数を渡します。`x` を `y` より小さいと扱う場合に `true`、それ以外では `false` を返してください。`std::sort` に渡せるものと同じ条件で、呼び出し中に比較結果を変えず、小さい・同順位という関係に矛盾がないものを使います。両方の関数で、この比較関数が定める大小に沿って `min` と `max` を選びます。`T` 自体の比較演算子は不要です。比較しても互いに小さくない 2 要素は同順位とし、同順位の要素のうちどれを返すかは未規定です。

```cpp
#include "lib/convolution/max_min_convolution.hpp"
#include <cassert>
#include <string>
#include <utility>
#include <vector>

int main() {
	using Item = std::pair<int, std::string>;
	std::vector<Item> a{ {3, "A"}, {1, "B"} }, b{ {2, "C"}, {4, "D"} };
	auto compare = [](const Item &x, const Item &y) { return x.first < y.first; };
	auto c = max_min_convolution(a, b, compare);
	assert(c[0].first == 2 && c[1].first == 3 && c[2].first == 1);
}
```

返り値は `std::vector<T>` です。添字 `k` は `[0,a.size()+b.size()-1)` で、既定引数はありません。比較・コピーに定数時間より長くかかる型では、そのコストも必要です。

時間 $$\displaystyle O(L \sqrt{L \log L})$$、追加空間 $$O(L)$$ です。
