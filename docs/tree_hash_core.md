---
title: 木ハッシュの共通部品
documentation_of: //components/tree_hash_core.hpp
library_category: components/tree
---

[根付き木のハッシュ](rooted_tree_hash.md)・[木のハッシュ](tree_hash.md) で使う共通部品です。通常は各公開ヘッダーを include してください。

`TreeHashValue` は 2 要素の `std::array<std::uint64_t,2> value` を持つ値型です。等価比較と辞書順の大小比較に対応します。`TreeHashValue::Hasher` を `std::unordered_map`・`std::unordered_set` のハッシュ関数として指定できます。

内部関数へ渡すハッシュ値は、各成分を `2^61-1` で割った余りとし、`0` 以上 `2^61-1` 未満に収めてください。根付き化は、無向木で根を指定して各頂点の親と子を決める処理です。

頂点数を $$n$$ として、根付き化・ハッシュ計算はそれぞれ $$O(n)$$ 時間・空間です。ハッシュの比較・ハッシュ関数は $$O(1)$$ 時間です。


頂点ラベル付きの計算にも同じ名前空間 `tree_hash_detail` の部品を使います。`labels` は長さ $$n$$ で 定数時間の `size()` と添字アクセスが使える列とし、要素型は `bool` を含む 64 bit 以下の標準整数型にしてください。同じ要素型・seed・実際の値から同じラベルの初期値を作るため、別々の木の値も直接比較できます。整数の演算は定数時間として扱います。

| 操作 | 内容・返り値 | 時間計算量 | 追加空間 |
| --- | --- | --- | --- |
| `root_tree(graph,root)` | 無向木から親・走査順・高さ・部分木サイズを `Rooting` で返す | $$O(n)$$ | $$O(n)$$ |
| `root_parents(parents)` | 親配列から同じ `Rooting` を返す | $$O(n)$$ | $$O(n)$$ |
| `hashes(tree,seed)` / `hashes(graph,tree,seed)` | ラベル無しの各部分木の `vector<TreeHashValue>` | $$O(n)$$ | $$O(n)$$ |
| `hashes(tree,labels,seed)` | ラベル付きの各部分木の `vector<TreeHashValue>` | $$O(n)$$ | $$O(n)$$ |
| `label_hash(label,seed)` | ラベルの初期値を `TreeHashValue` で返す | $$O(1)$$ | $$O(1)$$ |
| `salt_at(height,seed)` | 高さごとの乱数を `TreeHashValue` で返す | $$O(1)$$ | $$O(1)$$ |
| `append(hash,child,salt)` | 子の値を親の `hash` に掛け合わせる。返り値は `void` | $$O(1)$$ | $$O(1)$$ |
| `mul(a,b)` | 2 数の積を `2^61-1` で割った余りを `uint64_t` で返す | $$O(1)$$ | $$O(1)$$ |
| `default_seed()` | プロセス内で共通の seed を `uint64_t` で返す | $$O(1)$$ | $$O(1)$$ |

`Rooting` は頂点順の `parent,height,size` と、親が子より先に来る `order` を持ちます。根の親は `-1` です。無向木・親配列の条件は公開ライブラリと同じで、空の木にも対応します。`hashes` の入力は変更せず、返した配列は元の木・ラベル列の寿命に依存しません。`hashes(graph,tree,seed)` の `graph` と `tree` は同じ木を表すものにしてください。`labels` と `tree.parent` の長さも揃えてください。

`label_hash` の 2 成分はそれぞれ `1` 以上 `2^61-1` 未満です。異なるラベルが衝突する可能性はあり、ラベル付きとラベル無しの結果は比較できません。`salt_at` の `height` は非負の高さです。`append`・`mul` の入力は先に説明した剰余の範囲とし、`append` は第 1 引数だけを書き換えます。追加の共有表や、ラベルごとの前計算は保持しません。
