---
title: "Mod of Linear の等差数列分割"
documentation_of: //math/partition_mod_of_linear.hpp
---

```cpp
#include "lib/math/partition_mod_of_linear.hpp"

auto parts = partition_mod_of_linear(4, 5, 2, 1);
std::vector<std::uint64_t> values(4);
for (const auto &part : parts) {
	for (std::uint64_t j = 0; j < part.count; ++j) {
		values[part.x + part.dx * j] = std::uint64_t(
			__int128_t(part.y) + __int128_t(part.dy) * j);
	}
}
// values = {1,3,0,2}

std::vector<ModLinearSegment> buffer;
partition_mod_of_linear(4, 5, 2, 1, [&](ModLinearSegment part) {
	buffer.push_back(part);
});
```

| 関数・型・メンバ | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `partition_mod_of_linear(n,m,a,b)` | $$(ai+b)\bmod m$$ の列を等差数列に分割し、`vector<ModLinearSegment>` を返す | $$O(\log m+k)$$ |
| `partition_mod_of_linear(n,m,a,b,emit)` | 分割ごとに `emit(segment)` を呼ぶ。返り値は `void`、出力用の動的確保は不要 | $$O(\log m+k)$$ |
| `ModLinearSegment` | 一つの等差数列。構造化束縛の順序は `x,y,dx,dy,count` | —（型） |
| `x` / `y` | 最初の添字／その値 | $$O(1)$$ |
| `dx` / `dy` | 添字の刻み／値の公差。`dx > 0`、`dy` は負の場合もある | $$O(1)$$ |
| `count` | 含まれる項数。常に正 | $$O(1)$$ |
| `operator==` | 区間の全メンバを比較 | $$O(1)$$ |

$$0\le i<n$$ の添字と値を分割します。`n,m,a,b` と `x,y,dx,count` は `uint64_t`、`dy` は `int64_t` です。`m > 0` とし、それぞれの整数型の全域を扱えます。`a,b` は内部で `m` を法として正規化します。`n == 0` では空配列を返し、コールバックも呼びません。

各区間は $$0\le j<\mathrm{count}$$ に対して、添字 $$x+\mathrm{dx}\,j$$ の値が $$y+\mathrm{dy}\,j$$ であることを表します。全ての添字を重複なく一度ずつ含み、全区間の `dx,dy` は共通です。出力は添字を `dx` で割った余りの順で、同じ余りでは添字の昇順です。全体が `x` の昇順に並ぶとは限りません。

値を復元する際、大きい値と負の公差を混在させるなら、`__int128_t(part.y) + __int128_t(part.dy) * j` のように計算します。数学的な結果は常に $$0\le y+\mathrm{dy}\,j<m$$ です。

旧 `sqrt_floor`・`math/sqrt_floor.hpp` を置き換えた API です。以前は共通の公差と区間配列を別々に返していましたが、各区間に公差も含める形に変更しました。配列を保存する必要がなければ、コールバック版を使います。

添字の刻みは $$1\le d\le\lfloor\sqrt n\rfloor$$ の範囲で $$(ad)\bmod m$$ の絶対値が最小になるものを選びます。ここで絶対値は、正負の代表元のうち絶対値が小さい方です。同値なら最小の `d`、正負も同値なら非負の公差を選びます。探索には prefix minimum の内部処理を使い、列挙には正負の公差を直接使います。

[解説](https://atcoder.jp/contests/abc429/editorial/17143) の分割方法に基づきます。出力区間数を $$k=O(\sqrt n)$$ とすると、時間は $$O(\log m+k)$$、追加空間は配列版が $$O(k)$$、コールバック版が $$O(1)$$ です。コールバック 1 回の時間は $$O(1)$$ とします。
