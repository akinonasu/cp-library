---
title: Data Dependent Tree
documentation_of: //string/data_dependent_tree.hpp
library_category: string
---

連結・分割・編集できる文字列・整数列を管理し、LCP と辞書順比較を求めます。同じ管理オブジェクト内では、内容が同じ列は構築・編集の履歴によらず同じ木になります。一致判定に誤判定はありません。

```cpp
#include "lib/string/data_dependent_tree.hpp"

int main() {
	DataDependentTree tree;
	int a = tree.build("banana");
	int b = tree.set(a, 4, 'd');
	assert(tree.str(b) == "banada");
	assert(tree.str(a) == "banana");
	assert(tree.lcp(a, b) == 4);
	assert(tree.lcp(a, 1, a, 3) == 3);
	assert(tree.lcp(a, 1, a, 3, 2) == 2);
	assert(tree.compare(a, b) > 0);

	auto [left, right] = tree.split(a, 3);
	assert(tree.str(left) == "ban");
	assert(tree.str(right) == "ana");
	assert(tree.equal(tree.concat(left, right), a));
	assert(tree.substr(a, 1, 4) == right);
	int c = tree.insert(a, 3, "!");
	assert(tree.str(c) == "ban!ana");
	assert(tree.erase(c, 3, 4) == a);
	assert(tree.str(tree.repeat(right, 2)) == "anaana");
}
```

整数列には `BasicDataDependentTree<T>` を使います。`T` は `bool`・`char` 以外の 64 bit 以下の組み込み整数型で、全ての入力・置換値の型を揃えます。入力は `std::span<const T>` として受け取り、同じ要素型の `std::vector`・`std::array`・`std::span` を直接渡せます。負の値や 64 bit 整数の最小値・最大値も区別します。`at` は `T`、`str` は `std::vector<T>` を返し、辞書順は `T` の数値の大小で比較します。

```cpp
#include "lib/string/data_dependent_tree.hpp"
#include <cstdint>
#include <vector>

int main() {
	BasicDataDependentTree<std::int64_t> tree;
	std::vector<std::int64_t> values{1, 257, -1000000000000LL, 1};
	int a = tree.build(values);
	int b = tree.set(a, 1, std::int64_t(1));
	assert(tree.lcp(a, b) == 1);
	assert(tree.at(a, 1) == 257);
	assert(tree.str(a) == values);
	assert(tree.compare(b, a) < 0);
}
```

以下の操作は両方の型で共通です。`DataDependentTree` は `BasicDataDependentTree<char>` の別名です。列は根の番号 `int` で扱い、`0` は空列です。全ての編集は新しい根を返し、既存の根は変更しません。過去の列との比較や、異なるバージョンを連結する操作にも対応します。`build` も既存の列を消去しません。

番号は取得した `DataDependentTree` に属します。異なるオブジェクトで得た番号を混ぜないでください。全体をコピーした場合は、コピー時点までの番号を両方で使えます。コピー後に追加した番号は、その追加先だけで使います。乱数シードを揃えるだけでは番号・木の形の共有にはなりません。

文字列の入力は `std::string_view`、または要素型が `char` の `std::vector`・`std::array`・`std::span` で渡し、NUL を含む全 256 種類のバイトに対応します。`char` の辞書順は符号なしバイトの順序です。入力はコピーするため、呼び出し後に変更・破棄できます。要素数・位置・LCP 長の型は `size_type = std::uint64_t`、区間は半開区間 `[l,r)` です。要素数が `UINT64_MAX` を超える連結・挿入・繰り返しは `std::length_error` を送出します。位置と区間は列の範囲内にしてください。

乱数は木の形と実行時間に使い、文字列の同一性は内容を表す木の構造まで照合して判定します。ハッシュ表の衝突による誤判定はありません。既定では実行時にシードを選び、`DataDependentTree(seed)` で固定できます。計算量は乱数の選択に対する期待値で、最悪時の高さを保証するものではありません。

作成したノードは管理オブジェクトが保持し続けます。使わなくなった根を捨てても、そのノードの領域は解放しません。多数のバージョンを作る場合は、現在の文字数だけでなく編集回数に応じたメモリが必要です。

要素数の最大値を $$n$$、追加する列の長さを $$m$$、繰り返し回数を $$k$$、管理しているノード数を $$M$$ とします。さらに、各ノードに保存した子番号の総数を $$C$$、これまでに `reserve` へ渡した最大の予約数を $$R$$ とすると、保持空間は $$O(M+C+R)$$ です。整数列でこれまでに使った異なる値も各 1 ノードとして保持し、$$M$$ に含みます。予約しない場合は $$R=0$$ とします。更新の時間・追加領域の評価は期待償却です。

ノード番号には `int` を使います。保持ノード数と `reserve` の予約数は `INT_MAX` 未満とし、上限を超える追加・予約には `std::length_error` を送出します。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| 既定構築 / シードを渡した構築 | 空の列の集合を作成 | $$O(1)$$ |
| `build(s)` | 列 `s` の根を返す | 期待 $$O(m)$$ 時間・追加領域 |
| `size(root)` / `empty(root)` / `height(root)` | 要素数 / 空判定 / 木の高さ | $$O(1)$$ |
| `equal(a,b)` | 内容の一致判定 | $$O(1)$$ |
| `lcp(a,b)` / `lcp(a,b,limit)` | 先頭からの LCP 長。`limit` があればその長さまで | 期待 $$O(\log n)$$ |
| `lcp(a,p,b,q)` / `lcp(a,p,b,q,limit)` | `a` の位置 `p` と `b` の位置 `q` からの LCP 長 | 期待 $$O(\log n)$$ |
| `compare(a,b)` | 辞書順で小さい / 等しい / 大きいとき `-1` / `0` / `1` | 期待 $$O(\log n)$$ |
| `at(root,p)` | 位置 `p` の要素を返す。文字列は `char`、整数列は `T` | 期待 $$O(\log n)$$ |
| `concat(a,b)` | `a` と `b` を連結した根 | 期待 $$O(\log n)$$ 時間・追加領域 |
| `split(root,p)` | `[0,p)` と `[p,size(root))` の根の `pair<int,int>` | 期待 $$O(\log n)$$ 時間・追加領域 |
| `substr(root,l,r)` | 部分列 `[l,r)` の根 | 期待 $$O(\log n)$$ 時間・追加領域 |
| `insert(root,p,other)` | 位置 `p` に別の根 `other` を挿入した根 | 期待 $$O(\log n)$$ 時間・追加領域 |
| `insert(root,p,s)` | 位置 `p` に列 `s` を挿入した根 | 期待 $$O(m+\log n)$$ 時間・追加領域 |
| `erase(root,l,r)` / `set(root,p,c)` | 区間の削除 / 1 要素の置換を行った根 | 期待 $$O(\log n)$$ 時間・追加領域 |
| `repeat(root,k)` | `k` 回連結した根。`k == 0` は空列 | 期待 $$O(\log k\log n)$$ 時間・追加領域 |
| `str(root)` | 文字列は `std::string`、整数列は `std::vector<T>` に展開 | 期待 $$O(n)$$ 時間、出力に $$O(n)$$ 空間 |
| `storage_nodes()` | 保持ノード数。文字列は全 256 種類の文字ノード、整数列はこれまでに使った値のノードを含む | $$O(1)$$ |
| `reserve(count)` | ノード・同一性照合表の容量を予約 | 期待 $$O(M+count)$$ |
| `swap(other)` / コピー / ムーブ構築 | 管理内容を交換 / 全体を複製 / 全体を移動 | 交換・ムーブ構築 $$O(1)$$、コピーは保持量に比例 |

LCP・一致判定・辞書順比較・要素アクセスはノードを追加しません。LCP・辞書順比較の一時領域は期待 $$O(\log n)$$、要素アクセス・一致判定・サイズの問い合わせの追加空間は $$O(1)$$ です。LCP の開始位置は末尾でもよく、その場合は `0` です。空列の高さは `0` です。ムーブ元は空の列の管理に再利用できます。代入は、代入先が保持していた領域の破棄に必要な時間もかかります。

参考: Jean Cardinal・John Iacono, [Modular Subset Sum, Dynamic Strings, and Zero-Sum Sets, §4](https://arxiv.org/abs/2008.08417)。静的な文字列の Z 配列だけが必要な場合は、線形時間の [Z algorithm](z_algorithm.md) を使えます。
