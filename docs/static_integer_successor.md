---
title: Static Integer Successor Index
documentation_of: //components/static_integer_successor.hpp
---

複数の静的な昇順整数列に対する successor 検索の内部部品です。`StaticThresholdSearch` の重いパス内の検索に使います。

```cpp
#include "lib/components/static_integer_successor.hpp"
#include <array>
#include <cassert>

int main() {
	static_integer_successor_detail::Index index;
	std::array<std::uint64_t, 4> keys{2, 5, 7, 10};
	auto stride = index.add(0, keys, 64);
	assert(index.lower_bound<false>(0, keys, 64, stride, 6) == 2);
	assert(index.lower_bound<true>(0, keys, 64, stride, 10) == 4);
}
```

`add(unsigned group, span<const uint64_t> keys, int bits)` はグループを登録し、検索へ渡す補助値 `uint64_t stride` を返します。`bits` は 8・16・32・64 のいずれか、`keys` はそのビット幅に収まる値の厳密昇順列で、長さは `INT_MAX` 以下です。同じグループ番号を再登録しないでください。

`lower_bound<Strict>(group,keys,bits,stride,x)` は、`Strict=false` なら `>=x`、`Strict=true` なら `>x` の最初の位置を `int` で返します。検索には登録時と同じ列・ビット幅・補助値を渡します。元の配列の参照は保持しません。不在なら列長、空列なら 0 を返します。`x` も `bits` の範囲に収めてください。引数の既定値はありません。

短い列は二分探索、等差数列は算術で検索します。その他の列は値の上位ビットによるディレクトリで絞り込み、該当区間が大きい場合は `bits` 個ごとの標本に対する X-fast 型の prefix hash へ切り替えます。ハッシュは存在確認に完全なキーを使うため、衝突しても結果は変わりません。

ワード幅を $$w$$、全列長を $$m$$ とすると、登録はハッシュ参照を期待定数時間として期待 $$O(m)$$ 時間・$$O(m)$$ 空間、検索は期待 $$O(\log w)$$ 時間・追加空間 $$O(1)$$ です。空列は定数時間です。最悪時間の保証ではありません。一般の利用には、この部品より `StaticThresholdSearch` を使います。
