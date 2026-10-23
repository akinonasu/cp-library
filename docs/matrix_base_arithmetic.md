---
title: 行列の基本演算
documentation_of: //components/matrix_base_arithmetic.hpp
---

通常版・軽量版が共有する、要素型の演算を使う行列積・消去・整数行列式です。

| 操作 | 内容 | 計算量 |
| --- | --- | --- |
| `matrix_detail::basic_multiply(a,b,n,k,m)` | 行優先の配列で表した $$n\times k$$ と $$k\times m$$ の行列積 | $$O(nkm)$$ |
| `matrix_detail::eliminate<Ops>(a,h,stride,columns,reduced,eps)` | 消去してピボット列とピボットの積を返す | $$O(h\cdot\mathrm{stride}\cdot\min(h,\mathrm{columns}))$$ |
| `matrix_detail::integer_determinant(a,n)` | Bareiss 法による符号付き整数の行列式 | $$O(n^3)$$ |

行列積の追加空間は $$O(nm)$$、消去はピボット列に $$O(\min(h,\mathrm{columns}))$$、整数行列式は $$O(n^2)$$ です。要素の演算・コピーを $$O(1)$$ とします。
