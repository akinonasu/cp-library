---
title: コンパイル時の警告
---

GCC・Clang では `-Wall -Wextra` を有効にして利用できます。ライブラリの属性による警告もコンパイラの警告設定に従います。

| 対象 | 警告を出す使い方 | 属性 |
| --- | --- | --- |
| `mint`、静的・動的 modint、`Fraction`、`BigInt` | ローカル変数を作って使わない | `[[gnu::warn_unused]]` |
| 通常版・軽量版 FPS、`Matrix`・`LightMatrix`、`DynamicBitset` | ローカル変数を作って使わない | `[[gnu::warn_unused]]` |
| 上記の値型の四則演算・逆数・累乗などの計算と照会 | 計算結果を捨てる | `[[nodiscard]]` |
| SegTree・遅延 SegTree、Fenwick Tree、Union Find | 要素・集約値・代表元などの照会結果を捨てる | `[[nodiscard]]` |
| 木の中心・直径 | 返された中心・直径を捨てる | `[[nodiscard]]` |

例えば `x.pow(3);` は `x` 自身を書き換えません。戻り値の使い忘れを検出するために警告します。`x *= 3;`、`x++`、木やデータ構造の更新、辺の追加、入出力のように、操作自体が目的になる API の戻り値は捨てられます。

未使用の変数を意図して宣言する場合は `[[maybe_unused]]`、結果を意図して捨てる場合は `static_cast<void>(expression)` または `(void)expression` を使ってください。

```cpp
#include "lib/math/mint.hpp"
#include <cassert>

int main() {
	mint x = 2;
	auto cube = x.pow(3);
	assert(cube == mint(8));
	assert(x == mint(2));
	x++;
	assert(x == mint(3));
	(void)x.inv();
	[[maybe_unused]] mint reserved = 0;
}
```

未初期化変数、符号の異なる整数の比較、非 `void` 関数の戻り忘れなどは、コンパイラの警告設定で検査します。縮小変換を伴うリスト初期化や、`Fraction`・`BigInt` の明示的な変換を暗黙に行うコードは、C++ の型検査の対象です。各 API が意図して認める整数からの変換は引き続き利用できます。

`[[maybe_unused]]` は未使用変数に対する指定であり、`[[nodiscard]]` が付いた呼び出しの結果を捨てる代わりにはなりません。結果を変数に保存して使うか、呼び出し式を明示的に `void` へ変換してください。
