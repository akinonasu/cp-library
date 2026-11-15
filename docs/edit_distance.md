---
title: "Edit Distance"
documentation_of: //string/edit_distance.hpp
library_category: string/distance
---

```cpp
#include "lib/string/edit_distance.hpp"
#include <vector>

int distance = edit_distance("kitten", "sitting"); // 3

std::vector<int> a = {1000, -2, 7}, b = {1000, 7, 9};
int integer_distance = edit_distance(a, b); // 2
```

長い方の列の長さを $$n$$、短い方を $$m$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `edit_distance(s,t)` | 挿入・削除・置換のコストを各 1 とする編集距離を `int` で返す | $$O(n\lceil m/64\rceil)$$ |

挿入・削除・置換をそれぞれコスト 1 として、一方の列をもう一方に変えるためのコストの最小値を返します。2 つの文字列を `std::string_view` で受け取り、文字はバイト単位で扱います。NUL・128 以上も含む全 256 種類に対応します。

整数列は、同じ要素型 `T` の 2 つの `std::vector<T>` または 2 つの `std::span<T>` / `std::span<const T>` で指定できます。`span` は長さを型で指定したものにも対応します。`T` は `bool` を除く 64 bit 以下の標準整数型にしてください。負数・整数型の最小値と最大値もそのまま扱い、値を文字に変換して丸めることはありません。重複する要素も扱えます。各列の長さは `INT_MAX` 以下にしてください。入力は変更しません。

片方が空なら、もう片方の長さを返します。この場合は $$O(1)$$ 時間・追加空間です。

時間は文字列・整数列ともに $$O(n\lceil m/64\rceil)$$、補助空間は文字列が $$O(m)$$、整数列が $$O(n+m)$$ です。整数列の前処理に使う $$O(n+m)$$ 時間・空間も含みます。
