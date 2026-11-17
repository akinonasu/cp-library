---
title: "整数列の比較用ビットマスク"
documentation_of: //components/sequence_bit_masks.hpp
library_category: components/string
---

LCS・編集距離で整数列を扱うための内部部品です。通常は [LCS](lcs.md)・[編集距離](edit_distance.md) を直接使ってください。

```cpp
#include "lib/components/sequence_bit_masks.hpp"
#include <vector>

std::vector<int> s = {1000, -7, 1000}, t = {-7, 1000};
sequence_bit_masks_detail::CompressedPair<int> input(s, t);
sequence_bit_masks_detail::MaskWorkspace workspace(input.symbols.size());
sequence_bit_masks_detail::SparseMasks masks(input.second, workspace);
int index = masks.first(input.first[0]);
std::uint64_t bits = masks.next(index, 0); // 2: t[1] が s[0] と等しい
```

2 列の長さを $$n,m$$、ビットマスクを作る 1 列の長さを $$k$$、作業領域が扱う番号の種類数を `alphabet` とします。

| 操作 | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `CompressedPair<T>(s,t)` | 同じ整数値に同じ番号を割り当て、番号列 `first,second` と元の値の配列 `symbols` を作る | $$O(n+m)$$ |
| `MaskWorkspace(alphabet)` | `alphabet` 種類の番号に対する作業領域を準備 | $$O(\mathrm{alphabet})$$ |
| `SparseMasks(sequence,workspace,reverse=false)` | 各番号が現れる位置を 64 要素ずつのビットマスクにする。`reverse=true` なら逆順の位置を使う | $$O(k)$$ |
| `masks.first(c)` | 番号 `c` の最初のブロック番号を `int` で返す。不在なら `-1` | $$O(1)$$ |
| `masks.next(index,word)` | ブロック `word` のマスクを `std::uint64_t` で返し、該当ブロックがあれば `index` を次へ進める | $$O(1)$$ |
| `SparseMasks` の破棄 | 使った番号の作業領域を初期状態に戻す | $$O(k)$$ |
| `subsequence(s,t)` | `s` が `t` の部分列なら `bool` の `true` を返す | $$O(m)$$ |
| `trim(s,t)` | 共通の接頭辞・接尾辞を両方の範囲から取り除き、その 2 範囲を組として返す | $$O(\min(n,m))$$ |

名前は全て `sequence_bit_masks_detail` 内にあります。`CompressedPair<T>` は同じ型の `std::span<const T>` を受け取り、`T` は `bool` を除く 64 bit 以下の標準整数型です。各列の長さは `INT_MAX` 以下にしてください。番号の型は `std::uint32_t`、範囲は `[0,symbols.size())` で、`symbols[first[i]] == s[i]`・`symbols[second[i]] == t[i]` が成り立ちます。番号の順序を値の大小比較に使わないでください。元の列は変更せず、作成後は元の列を保持する必要もありません。空列・重複・負数・型の最小値と最大値にも対応します。

`MaskWorkspace` の `alphabet` は扱う番号の種類数です。`SparseMasks` の `sequence` は `std::span<const std::uint32_t>`、長さは `INT_MAX` 以下、各番号は `[0,alphabet)` にしてください。同じ作業領域を共有する `SparseMasks` を同時に作らないでください。マスクを作った後は元の番号列を保持する必要はありませんが、作業領域はマスクを破棄するまで保持してください。`SparseMasks` はコピーできません。

`masks.next(index,word)` には、`first(c)` で得た `index` を参照で渡し、`word` を `0,1,...,ceil(k/64)-1` の順で指定します。ビット `j` が 1 なら、位置 `64*word+j` の番号が `c` です。該当する要素がないブロック・列末尾を超えるビットは 0 です。空列なら `first` は常に `-1`、マスクは 0 です。ブロック数は列長以下で、異なる値の数と列長の積に比例する領域は確保しません。

`subsequence`・`trim` は同じ要素型の 2 つの `std::span<const T>` を受け取り、`==` で比較します。各列の長さは `INT_MAX` 以下にしてください。空の `s` は常に部分列で、空の `t` に対しては `s` も空の場合だけ `true` です。`trim` は要素を変更せず、引数の `span` の範囲を変更します。返り値の型は `std::pair<std::span<const T>,std::span<const T>>` で、元の `s` の接頭辞・接尾辞を指します。返された範囲を使う間は元の列を保持してください。接頭辞と接尾辞は重ならず、片方が空なら両方とも空です。

`CompressedPair` の作成・保持は $$O(n+m)$$ 時間・空間、`MaskWorkspace` の作成・保持は $$O(\mathrm{alphabet})$$ 時間・空間です。`SparseMasks` の作成・保持・破棄には $$O(k)$$ 時間・空間を使い、`first`・`next` の追加空間は $$O(1)$$ です。`subsequence`・`trim` の追加空間は $$O(1)$$ で、要素比較は定数時間として扱います。
