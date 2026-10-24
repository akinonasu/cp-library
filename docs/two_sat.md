---
title: "2-SAT"
documentation_of: //graph/two_sat.hpp
---

```cpp
#include "lib/graph/two_sat.hpp"

TwoSAT sat(3);
sat.add_clause(0, true, 1, false); // x[0] または !x[1]
sat.add_clause(1, true, 2, true);
if (sat.satisfiable()) {
    auto answer = sat.answer(); // std::vector<bool>
}
```

`TwoSAT(n)` で 0 始まりの `n` 個の bool 変数を作ります。`add_clause(i,f,j,g)` は `(x[i] == f) || (x[j] == g)` を追加します。`satisfiable()` が充足可能性を返し、直前の結果が true の場合だけ `answer()` で代入例を取得できます。代入例は一意ではありません。

同じ変数を両辺に指定できます。`add_clause(i,f,i,f)` は変数の固定、`add_clause(i,true,i,false)` は恒真節です。恒真節の辺と、固定節の重複する辺は追加しません。

`size()` は変数数、`reserve(m)` は節数を基準に辺の格納容量を予約します。変数数は `0 <= n <= INT_MAX / 2`、端点は `[0,n)`、実際に格納する含意辺は `INT_MAX` 以下である必要があります。予約する節数は `INT_MAX / 2` 以下です。空の問題にも対応します。

繰り返しの判定や判定後の節追加に対応し、追加後は再び `satisfiable()` を呼んでください。コピーは独立します。

変数数を $$n$$、節数を $$m$$ として、構築は $$O(n)$$、節追加は償却 $$O(1)$$、判定は $$O(n+m)$$、代入例のコピーは $$O(n)$$、保持・作業空間は $$O(n+m)$$ です。
