---
title: Flat Hash Map
documentation_of: //data_structure/flat_hash_map.hpp
library_category: data_structure/associative
---

キーに対応する値を追加・更新・削除する連想配列です。

```cpp
#include "lib/data_structure/flat_hash_map.hpp"
#include <cassert>
#include <cstdint>

int main() {
	FlatHashMap<std::uint64_t, std::uint64_t> values;
	values.reserve(200000);
	values.set(10, 3);
	values[20] += 5;
	auto [it, inserted] = values.try_emplace(30, 7); // 追加され、inserted は true
	assert(inserted && it->second == 7);
	it->second += 1;
	auto a = values.get(10); // optional(3)
	auto b = values.get(40); // nullopt
	assert(a == 3 && !b);
	if (auto it = values.find(20); it != values.end()) it->second += 2;
	for (auto&& [key, value] : values) value += key;
	for (auto it = values.begin(); it != values.end();) {
		if (it->first == 10) it = values.erase(it);
		else ++it;
	}
	assert(values.size() == 2 && values.at(20) == 27 && values.at(30) == 38);
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `FlatHashMap<Key,T,Hash=std::hash<Key>,KeyEqual=std::equal_to<Key>>()` | 空のマップを作成 |
| `FlatHashMap(n)` | `n` 種類のキーを格納するための領域を確保 |
| `set(key,value)` | 値を追加または上書き |
| `insert(value)` | `value_type` のキーと値を追加。既存の値は変更しない。`std::pair<iterator,bool>` を返す |
| `emplace(args...)` | 引数からキーと値の組を構築して追加。既存の値は変更しない。`std::pair<iterator,bool>` を返す |
| `try_emplace(key,args...)` | キーがなければ引数から `T` を構築して追加。既存の値は変更しない。`std::pair<iterator,bool>` を返す |
| `insert_or_assign(key,value)` | 値を追加または上書き。`std::pair<iterator,bool>` を返す |
| `operator[](key)` | 値の `T&`。未登録なら `T{}` を追加 |
| `find(key)` | キーに対応する `iterator`。未登録なら `end()`。const オブジェクトでは `const_iterator` |
| `begin()` / `end()` | 列挙の先頭／終端。空なら同じ。const オブジェクトでは `const_iterator` |
| `cbegin()` / `cend()` | const イテレータで列挙する先頭／終端 |
| `get(key)` | 値を `std::optional<T>` で返す。未登録なら `nullopt` |
| `at(key)` | 値への参照。未登録なら `std::out_of_range` |
| `contains(key)` / `count(key)` | 存在判定／存在すれば 1、なければ 0 |
| `erase(key)` | キーを削除。存在すれば `true` |
| `erase(it)` | `iterator` または `const_iterator` が指す要素を削除し、次の要素の `iterator` を返す。最後なら `end()`。`end()` は渡せない |
| `size()` / `empty()` | 登録されたキー数／空判定 |
| `prefetch(key)` | 後でアクセスするキーを先読みするヒント。内容は変更しない |
| `reserve(n)` | `n` 種類のキーを格納するための領域を確保 |
| `clear()` | 全要素を削除 |
| `swap(other)` | 内容を交換 |

`prefetch` は `const` オブジェクトでも呼び出せる `noexcept` の操作で、未登録のキーも指定できます。環境によっては何も行いません。

`insert`・`emplace`・`try_emplace`・`insert_or_assign` の返り値は、対象の要素のイテレータと、新しく追加したかどうかの組です。既存キーなら `false` で、`insert_or_assign` だけは値を上書きします。`insert({key,value})` のように指定でき、`insert` はコピーと移動の両方を受け付けます。`try_emplace` は既存キーの値を構築せず、渡した引数からの移動も行いません。`emplace` は既存キーでもキーと値を構築することがあります。

`emplace` の引数は `std::pair<Key,T>` を構築できる形で指定します。例えば `emplace(key,value)` や `emplace(std::piecewise_construct,key_args,value_args)` を使えます。`try_emplace` はキーを受け取り、残りの引数から `T` を直接構築します。`insert_or_assign` は値の構築と代入を必要とします。キーのコピー・移動、値のコピー・移動・構築に必要な条件は、呼び出す操作で満たしてください。

`it != map.end()` で存在を判定し、`it->first` でキー、`it->second` で値を参照します。キーは変更できません。`iterator` から `const_iterator` への変換と両者の比較ができます。`++it`・`it++` で次の要素へ進みます。終端の参照やインクリメントは行わないでください。列挙順序は未指定です。

`const` オブジェクトの `find`・`begin`・`end` と、`cbegin`・`cend` は `const_iterator` を返し、値も変更できません。`at` は `const T&` を返します。読み取り専用で列挙する場合は `std::as_const(map)` または `cbegin`・`cend` を使えます。

専用のハッシュテーブルでは、`*it` は `std::pair<const Key&,T&>` を値で返します。const イテレータでは値側も `const T&` になります。列挙には `for (auto&& [key,value] : map)` を使ってください。この記法は `std::unordered_map` を使う型でも利用できます。専用テーブルでは `auto& entry = *it` のように組そのものへの変更可能な参照を受け取ることはできません。

`auto entry = *it` や `auto [key,value] = *it` でも、専用テーブルではキー・値をコピーせず参照を保持します。独立したコピーが必要なら `value_type entry(*it)` としてください。`const auto&` だけでは参照先の値を const にできないため、読み取り専用には const イテレータを使います。取得した参照の組は、イテレータを進めたり破棄したりした後も、キー・値そのものが有効な間は使えます。

イテレータや取得したキー・値への参照は、マップへの挿入・`set`・`operator[]`・`insert_or_assign`・`reserve`・`clear`・代入・移動・`swap` の後には取得し直してください。既存キーの更新でも領域を確保し直すことがあります。`insert`・`emplace`・`try_emplace`・`insert_or_assign` の返すイテレータは、その操作後の有効な位置を指します。`erase` は削除した要素のイテレータ・参照だけを無効にするため、例のように削除しながら列挙できます。イテレータは取得元のマップが有効な間だけ使い、異なるマップのイテレータは比較しないでください。`erase(it)` にはこのマップの有効な要素を指すイテレータを渡します。

`bool` を除く 64 bit 以下の標準整数キーと、格納されているバイト列のコピーで複製できる値では専用のハッシュテーブルを使います。値の型は `std::is_trivially_copyable_v<T>` を満たし、コピー・`T{}` による構築・代入が例外を送出しないことが必要です。その他の型や独自の `Hash`・`KeyEqual` では `std::unordered_map` を使います。`Hash`・`KeyEqual` は既定構築し、同値なキーのハッシュ値は等しくしてください。

`get` は値のコピー、`operator[]` は値の既定構築を必要とします。コピーは独立したマップを作り、移動元は再利用できる空のマップになります。`size_type` は `std::size_t`、`value_type` は `std::pair<const Key,T>` です。

引数なしの構築・`size`・`empty`・`prefetch`・`end`・`cend`・`swap`・移動構築、イテレータのコピー・比較・参照は $$O(1)$$ 時間です。検索・`insert`・`emplace`・`try_emplace`・`insert_or_assign`・`set`・`operator[]`・`erase(key)` は期待償却 $$O(1)$$ 時間です。確保した領域を $$C$$ として、使用空間・コピー・`clear` は $$O(C)$$、`FlatHashMap(m)` は $$O(m)$$、`reserve(m)` は期待 $$O(C+m)$$ 時間です。既に十分な領域があれば `reserve` は定数時間です。コピー・移動代入には、代入先がそれまで確保していた領域の破棄時間も加わります。空のマップの構築・コピー・削除にも定数時間かかります。値・キー・ハッシュ・比較の演算を定数時間とします。

確保した領域を $$C$$ として、専用のハッシュテーブルの全要素の列挙は $$O(C)$$ 時間、列挙の追加空間は $$O(1)$$ です。`begin`・`cbegin` と 1 回のインクリメント・`erase(it)` は最悪 $$O(C)$$ 時間で、空のマップの `begin` は $$O(1)$$ です。`it = erase(it)` を使って全要素を削除する場合も全体で $$O(C)$$ 時間です。`std::unordered_map` を使う型では、要素数を $$n$$ として全列挙は $$O(n)$$、`begin` は $$O(1)$$、インクリメント・`erase(it)` は平均 $$O(1)$$ です。追加操作・`erase(it)` が使う追加空間は、必要に応じたテーブルの拡張を除いて $$O(1)$$ です。

ハッシュ値が十分に分散することを仮定しています。
