---
title: "非負整数の一括入力"
documentation_of: //components/fast_io_bulk.hpp
---

`uint32_t` の整数をまとめて読みます。Linux x86_64 / GCC では AVX2 を利用できる CPU で高速な解析を選び、それ以外では `FastInput::read` を使います。

```cpp
#include "lib/components/fast_io_bulk.hpp"
#include <array>
#include <vector>

FastInput in;
int n, m;
in.read(n);
std::vector<std::uint32_t> a(n);
read_unsigned(in, a);

in.read(m);
std::vector<std::array<std::uint32_t, 3>> edges;
edges.reserve(m);
read_unsigned_records<3>(in, m, [&](auto u, auto v, auto w) {
	edges.push_back({u, v, w});
});
```

`read_unsigned_records<3>` は 3 個の整数を読んでから `emit(u, v, w)` を呼びます。上の例では、各辺の始点・終点・重みを読み、入力順に `edges` へ追加します。`Fields` は 1 以上のコンパイル時定数で、コールバックはその個数の `uint32_t` を受け取ります。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `read_unsigned(in, out)` | `span<uint32_t>` の全要素を入力で埋める | $$O(L)$$ |
| `read_unsigned_blocks(in, count, emit)` | `count` 個を順に読み、`emit(const uint32_t* values, size_t n)` に分割して渡す | $$O(L)$$ + コールバックの処理時間 |
| `read_unsigned_records<Fields>(in, count, emit)` | `Fields` 個ずつ `count` 組を読み、各組の整数を引数として `emit` を呼ぶ | $$O(L)$$ + コールバックの処理時間 |

空白区切りの十進整数が、指定した個数以上存在することが前提です。符号なしの `0` から `4294967295` までに対応し、先頭の 0・連続する空白・タブ・CRLF も使えます。途中の EOF・範囲外・符号・数字以外を含むトークンは扱いません。個数が 0 なら入力を進めず、コールバックも呼びません。指定個数の後にある入力は、続けて `in.read(x)` などで読めます。

`read_unsigned_blocks` の分割位置と各回の個数は実行環境で変わります。渡された領域はコールバックの実行中だけ有効です。コールバック内から同じ `in` を読み進めないでください。複数列の入力には、分割位置を気にせず使える `read_unsigned_records` が便利です。

`Fields * count` が `size_t` の範囲を超えると `length_error` を送出します。配列・コールバックが参照するデータの寿命は呼び出しが終わるまで保ってください。

読み進める文字数を $$L$$ として時間は $$O(L)$$、一括入力用の追加空間は定数です。組単位の入力にはさらに $$O(\mathrm{Fields})$$ の空間を使います。入力元のバッファ・mmap と出力先の空間は含みません。
