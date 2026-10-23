---
title: "modint998244353 の共通剰余演算"
documentation_of: //components/mint_arithmetic.hpp
---

`mint` 用の階乗生成・多項式評価・NTT が共有する内部演算です。通常は [modint998244353](https://qandori.github.io/cp-library/math/mint.hpp.html) を利用します。NTT の変換処理や変換表には依存しません。

`mint_detail::Ops` は Montgomery 表現のワードを扱い、`mul`・`add`・`sub`、通常表現との変換 `in`・`out`、定数乗算 `fixed_mul` を提供します。`one` は Montgomery 表現の 1、`mod` は 998244353、`twice` はその 2 倍です。

x86 では `vectorized()` で AVX2 の利用可否を確認でき、`load8`・`store8`・`mul8`・`add8`・`sub8`・`fixed_mul8`・`fixed_even8` を提供します。AVX2 の関数は対応 CPU から呼び出します。

各演算は $$O(1)$$ 時間・追加空間です。変換表の前計算や動的なメモリ確保は行いません。
