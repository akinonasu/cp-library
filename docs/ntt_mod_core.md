---
title: 法指定NTTの共通部品
documentation_of: //components/ntt_mod_core.hpp
---

[法指定 NTT](https://qandori.github.io/cp-library/components/ntt_mod.hpp.html) と任意法畳み込みが共有する内部部品です。通常は `ntt_mod.hpp` の公開関数を使います。

法は $$2<p<2^{31}$$ を満たす素数、原始根を指定します。変換長 $$n$$ は $$p-1$$ を割り切る 2 の冪で、空配列は渡せません。係数は法未満の `uint32_t` とします。

時間は $$O(n\log n+\log p)$$、追加空間は $$O(n)$$ です。
