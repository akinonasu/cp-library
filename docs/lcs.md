---
title: "Longest Common Subsequence"
documentation_of: //string/lcs.hpp
library_category: string/distance
---

```cpp
#include "lib/string/lcs.hpp"
#include <vector>

int length = lcs_length("abcde", "ace"); // 3
std::string sequence = lcs("abcde", "ace"); // "ace"

std::vector<int> a = {1000, -2, 1000, 7}, b = {-2, 7};
int integer_length = lcs_length(a, b); // 2
std::vector<int> integer_sequence = lcs(a, b); // {-2, 7}
```

長い方の列の長さを $$n$$、短い方を $$m$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `lcs_length(s,t)` | 最長共通部分列の長さを `int` で返す | $$O(n\lceil m/64\rceil)$$ |
| `lcs(s,t)` | 最長共通部分列を 1 つ、文字列なら `std::string`、整数列なら `std::vector<T>` で返す | $$O(n\lceil m/64\rceil+n\log n)$$ |

2 つの文字列を `std::string_view` で受け取ります。文字はバイト単位で扱い、NUL・128 以上も含む全 256 種類に対応します。

整数列は、同じ要素型 `T` の 2 つの `std::vector<T>` または 2 つの `std::span<T>` / `std::span<const T>` で指定できます。`span` は長さを型で指定したものにも対応します。`T` は `bool` を除く 64 bit 以下の標準整数型にしてください。負数・整数型の最小値と最大値もそのまま扱い、値を文字に変換して丸めることはありません。整数列の `lcs` は入力と同じ要素型の `std::vector<T>` を返します。

各列の長さは `INT_MAX` 以下にしてください。入力は変更しません。部分列は要素を飛ばして選べる列です。例の `ace` は連続していませんが、両方の文字列に同じ順序で現れます。重複する要素も扱えます。答えが複数ある場合はそのうち一つを返します。

片方が空なら、`lcs_length` は 0、`lcs` は空文字列または空の `std::vector<T>` を返します。この場合はともに $$O(1)$$ 時間・追加空間です。

長さ計算は $$O(n\lceil m/64\rceil)$$、復元は $$O(n\lceil m/64\rceil+n\log n)$$、補助空間はともに $$O(n+m)$$ です。整数列の前処理に使う $$O(n+m)$$ 時間・空間も含みます。

関連部品: [LCS の内部計算](lcs_kernel.md)。
