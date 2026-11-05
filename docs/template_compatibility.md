---
title: 利用者テンプレートとの併用
---

標準ヘッダーと ACL を先に読み込んだ後で、`using namespace std;`、`vec`・`rep`・`per`・`all`・`sz`・`uint`・`ulong` などを定義するテンプレートと併用できます。ライブラリを読み込んでも、これらのマクロは保持されます。

`all` マクロがあると、メンバー関数の呼び出しにもマクロが適用されます。`DynamicBitset` の全ビット判定は、次のように呼び出してください。

```cpp
#include <algorithm>
#include <vector>
#define all(a) a.begin(), a.end()
#include "lib/data_structure/dynamic_bitset.hpp"

int main() {
	std::vector<int> values{3, 1, 2};
	std::sort(all(values));
	DynamicBitset bits(3);
	bits.set_all();
	return (bits.all)() ? 0 : 1;
}
```

検証対象は Linux x86_64 の GCC・Clang、`DEBUG` 定義あり・なしです。全ヘッダーの個別読み込みに加え、テンプレートの入出力演算子・更新演算子と、数値型・FPS・データ構造・グラフ API の併用を検証しています。ACL を含む検証では `<atcoder/all>` と `<bits/stdc++.h>` を先に読み込んでいます。32 bit 環境で有効になる `long` マクロの分岐は、この検証には含みません。

テンプレート側の `array<T, n>` の出力で `rep(i, n)` を使うと、`int` と `size_t` の比較に対する警告が出ます。`for (size_t i = 0; i < n; ++i)` とすれば、この警告を避けられます。
