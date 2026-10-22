---
title: 木ハッシュの共通部品
documentation_of: //components/tree_hash_core.hpp
---

[根付き木のハッシュ](rooted_tree_hash.md)・[木のハッシュ](tree_hash.md) で使う共通部品です。通常は各公開ヘッダーを include してください。

`TreeHashValue` は 2 要素の `std::array<std::uint64_t,2> value` を持つ値型です。等価比較と辞書順の大小比較に対応します。`TreeHashValue::Hasher` を `std::unordered_map`・`std::unordered_set` のハッシュ関数として指定できます。

内部には `2^61-1` を法とする乗算、プロセスで共通の既定 seed、再帰を使わない無向木の根付き化、高さごとの乱数を使う積によるハッシュ計算をまとめています。内部関数へ渡すハッシュは正規化済みの剰余です。

頂点数を $$n$$ として、根付き化・ハッシュ計算はそれぞれ $$O(n)$$ 時間・空間です。ハッシュの比較・ハッシュ関数は $$O(1)$$ 時間です。
