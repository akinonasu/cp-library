---
title: 素数判定の基数表
documentation_of: //components/is_prime_bases.hpp
---

[素数判定](is_prime.md)で使う Bradley Berg の基数表です。32 bit 用の 256 要素と 64 bit 用の 16,384 要素を持ち、各要素は `std::uint16_t` です。通常は公開ヘッダーを include してください。

出典は Bradley Berg の [32 bit 用の表](https://techneon.com/download/is.prime.32.base.data)・[64 bit 用の表](https://techneon.com/download/is.prime.64.base.data)です。再配布時はヘッダーに含まれる著作権表示と利用許諾を残してください。
