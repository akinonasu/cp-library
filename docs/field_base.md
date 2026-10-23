---
title: 体の基本演算
documentation_of: //components/field_base.hpp
---

要素型 `T` の演算をそのまま使う `field_detail::Field<T>` です。行列などの内部で使います。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `Field<T>::add(a,b)` / `mul(a,b)` / `neg(a)` / `inv(a)` | 加算・乗算・符号反転・逆元 | 型 `T` の演算による |
| `Field<T>::zero(a,eps)` / `better(a,b)` | 0 判定／ピボットの比較 | $$O(1)$$ |
| `Field<T>::dot(a,b,n)` | 長さ `n` の内積 | $$O(n)$$ |
| `Field<T>::scale(a,n,x)` / `axpy(a,b,n,x)` | 定数倍／`a += x*b` | $$O(n)$$ |

要素の演算を $$O(1)$$ とすると、配列の操作は時間 $$O(n)$$、追加空間は $$O(1)$$ です。浮動小数点では `eps` 以下を 0 と判定し、絶対値の大きなピボットを優先します。
