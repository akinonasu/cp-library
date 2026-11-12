---
title: FPS の射影の汎用実装
documentation_of: //components/fps_projection_generic.hpp
---

`fps_detail::composition_fallback` の内部部品です。通常は [power projection](power_projection.md) の公開ヘッダーを使います。引数は同じ FPS 型 `F` とし、入力は変更しません。

`power_projection(f,weights,deg = -1)` は、各 `k` について `f(x)^k` の係数と `weights` の内積を求め、長さ `deg` の `F` で返します。`deg = -1` なら `weights.size()` 項を求めます。`0 <= deg <= 1 << 22`、`weights.size() <= 1 << 21` とします。`deg = 0` なら空列、重みが空なら0列です。

係数型には四則演算・逆元・0との比較、FPS 型には積・打ち切りが必要です。法は必要な整数の逆元が存在するものを使います。対象の係数数を $$n$$ とし、畳み込みが $$O(n\log n)$$ 時間で行える場合、時間は $$O(n\log^2 n)$$、追加空間は $$O(n\log n)$$ です。
