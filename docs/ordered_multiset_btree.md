---
title: 順序統計多重集合の共通実装
documentation_of: //components/ordered_multiset_btree.hpp
---

[Top K Sum](top_k_sum.md)が、標準の整数型・比較関数と、整数型の和を指定した場合に使う内部処理です。通常は `TopKSum`・`SmallestKSum`・`LargestKSum` を利用します。汎用型・独自の比較関数では、公開クラスが汎用実装を選びます。

順位・和・個数を多分岐の木で管理します。対応する x86 CPU では節点内の境界検索を SIMD で処理し、それ以外では通常の比較を使います。

異なる値の個数を $$d$$ として、検索・順位・和・削除は $$O(\log d)$$ 時間、挿入は償却 $$O(\log d)$$ 時間です。確保済みの節点数を $$D$$ として、使用空間は $$O(D)$$ です。
