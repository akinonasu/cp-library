---
title: 利用者テンプレートとの併用
---

標準ヘッダーと ACL を先に読み込んだ後で、`using namespace std;`、`vec`・`rep`・`per`・`all`・`sz`・`uint`・`ulong` などを定義するテンプレートと併用できます。ライブラリを読み込んでも、これらのマクロは保持されます。

`all(v)` を維持しながら `bits.all()` も通常どおり呼び出すには、C++20 以降の `__VA_OPT__` で引数の有無を判定します。テンプレートの `#define all(a) ...` を次の3行に置き換えてください。既存の `all(v)` や `UNIQUE` の変更は不要です。

```cpp
#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)
```

`std::begin`・`std::end` を使うため、`<iterator>` をマクロ定義より先に読み込みます。`<bits/stdc++.h>` を使う元のテンプレートでは追加不要です。元のマクロと同じく引数を二度評価するため、副作用のある式は渡さないでください。

```cpp
#include <algorithm>
#include <iterator>
#include <vector>
#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)
#include "lib/data_structure/dynamic_bitset.hpp"

int main() {
	std::vector<int> values{3, 1, 2};
	std::sort(all(values));
	DynamicBitset bits(3);
	bits.set_all();
	return bits.all() ? 0 : 1;
}
```

この定義では `bits.all()` や `std::bitset` の `all()` と併用できます。ライブラリの宣言・呼び出しとも通常の `all()` 表記を使えます。引数を取る別の `all(x)` という関数にもマクロが適用される点は元の定義と同じです。`using namespace std;` を外してもマクロの衝突は解消しません。
