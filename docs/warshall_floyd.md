---
title: ワーシャルフロイド法
documentation_of: //graph/shortest_path/warshall_floyd.hpp
redirect_from:
  - /graph/warshall_floyd.hpp.html
  - /lightweight/graph/warshall_floyd.hpp.html
---

重み付き有向グラフの全点対最短距離を求めます。負辺・多重辺・自己辺に対応し、グラフ全体の負閉路を検出します。頂点番号は 0 始まりです。

```cpp
#include "lib/graph/shortest_path/warshall_floyd.hpp"

int main() {
	WarshallFloyd<> graph(4);
	graph.add_edge(0, 1, 5);
	graph.add_edge(1, 2, -7);
	graph.add_edge(0, 2, 3);
	if (!graph.solve()) return 0;
	assert(graph(0, 2) == -2);
	assert(graph(3, 0) == graph.infinity());
}
```

距離型 `T` の既定は `std::int64_t` です。符号付きの 32 bit・64 bit 整数型を指定できます。到達不能を表す `inf` の既定は `std::numeric_limits<T>::max()`。有限の辺重み・最短距離は `T` の範囲内かつ `inf` 未満である必要があります。有限の負距離には `-1` も使えるため、到達不能の判定は `infinity()` との比較で行います。

`solve()` は負閉路がなければ `true`、あれば `false` を返します。`false` の後の距離は計算途中の値で、最短距離として使えません。負無限大になる全ての頂点対を列挙する機能や経路復元はありません。

行列から構築したり、既存の行列を更新したりすることもできます。対角成分は 0 と入力値の小さい方、到達不能な成分は `inf` としてください。

```cpp
std::int64_t inf = std::numeric_limits<std::int64_t>::max();
std::vector<std::vector<std::int64_t>> d{
	{0, 5, inf},
	{inf, 0, -7},
	{inf, inf, 0}
};
bool ok = warshall_floyd(d);
assert(ok && d[0][2] == -2);
```

`add_edge` は既存の辺との最小値を保持します。成功した `solve()` の後に辺を追加して再計算することもできます。辺の削除や重みの増加には、新しいオブジェクトを構築してください。

加算時の符号付き整数オーバーフローと、到達不能値への加算を避けます。入力範囲から安全性を確認できる場合、64 bit の指定でも内部を 32 bit にします。必要なら逆に 64 bit・128 bit へ拡張し、負閉路がなく有限の最短距離が `T` の範囲外または `inf` 以上なら `std::overflow_error` を送出します。極端に大きい重みでの 128 bit 計算は SIMD を使いません。

`n < 0`、`inf <= 0`、正方行列でない入力、`weight >= inf` は `std::invalid_argument` です。頂点番号は範囲内である必要があります。

32 頂点の小ブロックと 512 頂点の大ブロックで分割し、主な更新では複数行をレジスタ内に保持します。GCC・Clang の x86 では実行時に AVX-512F / AVX2 を判定して使います。対応しない CPU や macOS arm64 では通常の整数演算で同じ結果を求めます。単一スレッドで動作し、`-march=native` や全体への `#pragma GCC target` は不要です。

`WARSHALL_FLOYD_DISABLE_AVX512`、`WARSHALL_FLOYD_DISABLE_SIMD`、`WARSHALL_FLOYD_DISABLE_NARROWING` を include 前に定義すると、それぞれ AVX-512・明示的な SIMD・64 bit から 32 bit への変換を無効にできます。

実行時間は CPU・重みの範囲・負辺の有無・入出力に依存します。

頂点数を $$n$$、32 の倍数に切り上げた頂点数を $$p=32\lceil n/32\rceil$$ とします。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `WarshallFloyd<T>(n,inf)` | 辺なし、対角 0 の行列を構築。`inf` は省略可 | 時間・空間 $$O(p^2)$$ |
| `WarshallFloyd<T>(matrix,inf)` | 正方行列から構築 | 時間・空間 $$O(p^2)$$ |
| `size()` / `infinity()` | 頂点数 / 到達不能値 | $$O(1)$$ |
| `add_edge(from,to,weight)` | 有向辺を追加 | $$O(1)$$ |
| `solve()` | 全点対最短距離を計算し、負閉路がなければ `true` | 時間 $$O(p^3)$$、型変換時の追加空間 $$O(p^2)$$ |
| `operator()(i,j)` | 現在の距離を値で取得 | $$O(1)$$ |
| `distances()` | 現在の行列を `vector<vector<T>>` で取得 | 時間・追加空間 $$O(n^2)$$ |
| `warshall_floyd(matrix,inf)` | 行列を更新し、負閉路がなければ `true` | 時間 $$O(p^3)$$、追加空間 $$O(p^2)$$ |

本体は `sizeof(T) * p * p` バイトを保持します。型変換時は内部型の同じ要素数の作業領域が加わります。例えば 64 bit から 32 bit へ変換する場合、本体と作業領域で合計約 `12 * p * p` バイトです。行列版は呼び出し元の行列も別に保持するため、大きい入力では辺から直接 `WarshallFloyd` を構築するとメモリを抑えられます。
