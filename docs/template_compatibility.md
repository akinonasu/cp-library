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

引数があれば `CP_ALL_RANGE` を選び、`std::begin(a), std::end(a)` に展開します。引数がなければ `all()` を残します。マクロ自身の展開中は同じマクロを再展開しないため、後者は再帰しません。

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

この定義では `bits.all()` や `std::bitset` の `all()` と併用できます。引数を取る別の `all(x)` という関数にもマクロが適用される点は元の定義と同じです。元の単一引数マクロを使う場合もヘッダーは読み込めますが、全ビット判定には `(bits.all)()` が必要です。`using namespace std;` を外してもマクロの衝突は解消しません。

検証対象は Linux x86_64 の GCC・Clang、`DEBUG` 定義あり・なしです。全ヘッダーの個別読み込みに加え、テンプレートの入出力演算子・更新演算子と、数値型・FPS・データ構造・グラフ API の併用を検証しています。ACL を含む検証では `<atcoder/all>` と `<bits/stdc++.h>` を先に読み込んでいます。32 bit 環境で有効になる `long` マクロの分岐は、この検証には含みません。

テンプレート側の `array<T, n>` の出力で `rep(i, n)` を使うと、`int` と `size_t` の比較に対する警告が出ます。`for (size_t i = 0; i < n; ++i)` とすれば、この警告を避けられます。
