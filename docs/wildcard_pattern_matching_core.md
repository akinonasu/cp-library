---
title: ワイルドカード照合の内部部品
documentation_of: //components/wildcard_pattern_matching.hpp
library_category: components/string
---

[ワイルドカード付きパターンマッチング](wildcard_pattern_matching.md) の内部部品です。通常は公開ヘッダーを使います。

本文長を $$n$$、パターン長を $$m$$ とします。`wildcard_matching_detail::filter_ntt(text,pattern,matches,bound,text_has_wildcard,pattern_has_wildcard)` は、符号化した本文・パターンを照合し、`matches` の一致しない位置を `false` にします。既に `false` の位置を `true` に戻すことはありません。返り値は `void` で、本文・パターンは変更しません。引数の既定値はありません。

本文・パターンには同じ型の `std::vector<std::uint16_t>` または `std::vector<std::uint32_t>` を渡します。`0` はワイルドカード、正の値は通常の要素を表します。本文の通常要素とパターンの通常要素について、元の値が等しいとき、かつそのときに限り同じ符号にしてください。`uint16_t` 版は `1`～`256`、`uint32_t` 版は `1`～`UINT32_MAX` を使えます。

`matches` の長さは `n-m+1` とし、`text_has_wildcard`・`pattern_has_wildcard` は、それぞれの入力に `0` が含まれるかを表す `bool` です。重複した値も扱えます。$$1\le m\le n\le \mathrm{INT\_MAX}$$、$$m\le 2^{22}$$ を満たしてください。空列はこの内部部品では扱いません。パターン長が $$2^{22}$$ を超えた場合は `std::length_error` を送出します。

`bound` は、各位置において双方が通常の要素となる組の、符号の差を二乗した総和の上限です。この総和は非負で、一致する場合だけ 0 になります。`uint16_t` 版は `std::uint64_t`、`uint32_t` 版は `__uint128_t` で渡します。例えば、通常要素に使う最小の符号を $$a$$、最大の符号を $$b$$ とすると、$$m(b-a)^2$$ を使えます。`uint16_t` 版で `1`～`256` を使う場合は $$65025m$$ です。

総和が 0 かを複数の法で調べます。厳密な一致判定のため、`uint16_t` 版の上限は $$998244353\cdot469762049$$ 未満、`uint32_t` 版の上限は $$998244353\cdot469762049\cdot167772161$$ 未満にしてください。上の符号・長さの範囲なら、$$m(b-a)^2$$ はそれぞれの積より小さくなります。

`wildcard_matching_detail::filter_prime<mod>(text,pattern,matches,text_has_wildcard,pattern_has_wildcard)` は、指定した 1 個の法だけで同じ総和を調べます。本文・パターン・`matches`・2 個の `bool` の条件と返り値は `filter_ntt` と同じで、`bound` は渡しません。`mod` は上記のいずれかの素数とします。総和が 0 でなくても法の倍数なら残るため、この関数単独では厳密な一致判定になりません。

両操作の時間計算量は $$O(n\log m)$$、`matches` と入力を除く追加空間は $$O(m)$$ です。$$m=1$$ の対数の項は定数として扱います。前計算や共有表を呼び出し後に保持しません。
