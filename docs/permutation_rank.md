---
title: 順列の辞書順順位・復元
documentation_of: //sequence/permutation_rank.hpp
library_category: sequence
---

`0, ..., n-1` の順列について、辞書順で何番目かを求め、順位から元の順列を復元します。順位は **0 始まり**です。入力は変更しません。

```cpp
#include "lib/sequence/permutation_rank.hpp"
#include <cassert>

std::vector<int> p{2, 0, 3, 1};
auto rank = permutation_rank(p);
assert(rank == 13);
auto restored = permutation_unrank(4, rank);
assert(restored && *restored == p);
assert(!permutation_unrank(4, 24));
```

順列の長さを $$n$$ とします。

| 操作 | 引数・返り値 | 時間計算量 |
| --- | --- | --- |
| `permutation_rank<Rank = std::uint64_t>(p)` | 順列 `p` の辞書順順位を `Rank` で返す | $$O(n\log n)$$。$$n\le64$$ なら $$O(n)$$ |
| `permutation_unrank(n, rank)` | 長さ `n` の順位 `rank` の順列を `std::optional<std::vector<int>>` で返す。整数型 `Rank` は引数から推論 | `Rank` が 64 bit 以下の標準整数型なら $$O(n)$$、それ以外は $$O(n\log n)$$ |
| `permutation_rank<Rank = std::uint64_t>(p,alphabet)` | 指定したラベルの順序による `p` の辞書順順位を `Rank` で返す | $$O(n\log n)$$ |
| `permutation_unrank(alphabet,rank)` | 指定したラベルを使う順列を `std::optional<std::vector<T>>` で返す。`T` はラベルの型 | $$O(n\log n)$$ |

`p` は `std::span<const int>` に変換できる `vector<int>`・`array<int, n>`・C 配列などを渡します。長さは `INT_MAX` 以下で、各整数が `0` 以上 `n` 未満にちょうど 1 回ずつ現れるものとします。要素が範囲外・重複していれば `std::invalid_argument`、長さが上限を超えれば `std::length_error` を送出します。

順位は $$0\le r<n!$$ です。空順列は順位 0 の 1 通りです。`permutation_unrank` は負の順位・`n!` 以上の順位には `std::nullopt`、`n < 0` には `std::invalid_argument` を送出します。`n!` 自体が `Rank` に収まる必要はありません。

`Rank` には整数型を指定します。`permutation_rank` の結果が固定幅整数型に収まらない場合は `std::overflow_error` を送出します。既定型では長さ 20 まで全順列の順位が収まります。それ以上の長さでも、実際の順位が収まれば使えます。`__int128_t`・`__uint128_t`・`boost::multiprecision::cpp_int` や固定幅の多倍長整数も指定できます。順列の長さが `Rank` に収まる必要はありません。Boost は利用する側で include します。

```cpp
#include "lib/sequence/permutation_rank.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <cassert>
#include <numeric>

std::vector<int> p(50);
std::iota(p.rbegin(), p.rend(), 0);
auto rank = permutation_rank<boost::multiprecision::cpp_int>(p);
assert(permutation_unrank(50, rank) == p);
```

順位の剰余だけが必要なら、`permutation_rank<Mint>(p)` に modint 型を指定できます。型は `mod()` と整数からの構築・乗算代入・加算代入を備えるものとします。modint からの復元はできません。

`alphabet` を指定すると、整数・文字列・`pair` など任意のラベルを使えます。**`alphabet` に並べた順番**が、小さいラベルから大きいラベルへの順序です。例えば `{"red","blue","green"}` なら `"red"` を最小、`"green"` を最大とします。文字列自体の辞書順とは別です。

```cpp
#include "lib/sequence/permutation_rank.hpp"
#include <cassert>
#include <string>

std::vector<std::string> alphabet{"red", "blue", "green"};
std::vector<std::string> p{"green", "red", "blue"};
assert(permutation_rank(p, alphabet) == 4);
assert(permutation_unrank(alphabet, 4) == p);
assert(!permutation_unrank(alphabet, 6));
```

`p` と `alphabet` は同じ要素型にし、`vector`・`array`・`span`・`deque`・`string` などを渡します。固定長・可変長の `span`、独自の allocator を使う `vector` にも対応します。入力は変更しません。復元する列は常に `vector<T>` で、例えば `string` をラベルの集合として渡した場合は `optional<vector<char>>` を返します。

各ラベルは `<` で比較できるものとします。`x<y` と `y<x` が両方 `false` になることと、同じラベルであることが一致するようにしてください。復元ではラベルをコピーできる必要もあります。`alphabet` の長さは `INT_MAX` 以下、各ラベルは異なるものにしてください。`p` には `alphabet` の各ラベルをちょうど 1 回ずつ含めます。ラベルの重複・不在・長さの不一致は `std::invalid_argument`、長さの上限超過は `std::length_error` です。`permutation_unrank` でも `alphabet` の重複を検査します。空の `alphabet` は順位 0 の空順列 1 通りです。順位型・範囲外の順位・オーバーフローの扱いは整数の順列と同じで、`permutation_rank<Mint>(p,alphabet)` による順位の剰余にも対応します。

`alphabet` を省略した整数順列の時間は $$O(n\log n)$$、追加空間は $$O(n)$$ です。順位取得は $$n\le64$$ なら $$O(n)$$ 時間・$$O(1)$$ 追加空間です。復元は `Rank` が 64 bit 以下の標準整数型なら $$O(n)$$ 時間、返り値を除く追加空間は $$O(1)$$ です。`alphabet` を指定する 2 操作は時間 $$O(n\log n)$$・追加空間 $$O(n)$$ で、ラベルの検査も含みます。比較・コピー・整数・modint の演算を $$O(1)$$ とした計算量で、長い文字列や多倍長整数などでは各演算のコストが加わります。空順列は $$O(1)$$ 時間・空間です。
