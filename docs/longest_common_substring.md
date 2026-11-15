---
title: 最長共通部分文字列
documentation_of: //string/longest_common_substring.hpp
library_category: string/substring
---

2 つの文字列の最長共通**部分文字列**を 1 つ求めます。選ぶ文字は連続している必要があります。飛び飛びの文字も選べる[最長共通部分列](lcs.md)とは異なります。

```cpp
#include "lib/string/longest_common_substring.hpp"
#include <string>

std::string s = "ABABC", t = "BABCA";
auto result = longest_common_substring(s, t);
int i = result.s_pos;   // 1
int j = result.t_pos;   // 0
int len = result.length; // 4
auto common = s.substr(i, len); // "BABC"
auto [a, b, length] = longest_common_substring(s, t);
```

`longest_common_substring(s,t)` は `std::string_view` を受け取り、`LongestCommonSubstringResult` を返します。`std::string`・文字列リテラル・`std::string_view` を渡せます。返す `s_pos`・`t_pos` はそれぞれの入力での 0 始まりの開始位置で、`length` は長さです。

半開区間 `s[s_pos,s_pos+length)` と `t[t_pos,t_pos+length)` が一致します。解が複数ある場合はそのうち 1 つを返し、開始位置の優先順は保証しません。一方が空、または共通する文字がない場合は全フィールドが `0` です。

入力を変更せず、結果は入力を参照しません。NUL を含む全 256 種類のバイトに対応します。埋め込み NUL は長さを指定した `string_view` または `string` で渡してください。添字・長さはバイト単位なので、UTF-8 の文字境界は扱いません。入力の長さの合計は `INT_MAX-1` 以下としてください。

同じ関数に、同じ型の整数列 2 つを `std::vector<T>` または `std::span<T>`・`std::span<const T>` で渡せます。`T` は `bool` を除く 64 bit 以下の整数型です。負数や `uint64_t` の最大値も区別して扱います。`span` は固定長・動的長のどちらでもよく、変更可能なものと読み取り専用のものを組み合わせられます。返り値は同じ `LongestCommonSubstringResult` で、開始位置・長さは要素単位になります。空列・共通要素がない場合・複数解の扱いも同じです。

```cpp
#include "lib/string/longest_common_substring.hpp"
#include <cassert>
#include <vector>

int main() {
	std::vector<int> s{4, -1000, 7, -1000};
	std::vector<int> t{8, -1000, 7, -1000, 9};
	auto result = longest_common_substring(s, t);
	assert(result.s_pos == 1 && result.t_pos == 1 && result.length == 3);
}
```


全てのバイトで一致を確認し、ハッシュ値の一致だけで答えを確定しません。

入力長をそれぞれ $$n,m$$ とすると、文字列は $$O(n+m)$$ 時間、整数列は $$O((n+m)\log(n+m))$$ 時間です。追加空間はどちらも $$O(n+m)$$ です。片方が空なら $$O(1)$$ 時間・追加空間で返します。
