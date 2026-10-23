---
title: 整数点の凸包（補助部品）
documentation_of: //components/packed_convex_hull.hpp
---

[凸包](geometry_convex.md)で使う、整数座標の値域を確認してから適用する共通処理です。座標を平行移動し、radix sort と整数の向き判定で凸包を求めます。対応しない値域は呼び出し元の汎用処理を使います。

点数を $$n$$ として、時間・追加空間は $$O(n)$$ です。
