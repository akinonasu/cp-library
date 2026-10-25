---
title: "軽量 FPS の共通実装"
documentation_of: //components/fps_light_backend.hpp
---

軽量 FPS と追加演算の共通実装です。NTT と畳み込みは [軽量 FPS の NTT](fps_light_ntt.md) を共有します。積木は FPS の型で共通化した[多項式の共通積木](polynomial_product_tree.md)を使い、合成・射影は[汎用実装](fps_composition_generic.md)を共有します。通常の C++ と ASCII の識別子で記述し、コード長を縮めるための特殊文字マクロは使いません。AVX2 の実行時判定、スカラー経路、Newton 更新での中間結果の再利用は保持しています。

FPS の公開ヘッダーは `fps/fps_light.hpp`・`fps/fps_light_algorithms.hpp`、積木は `components/fps_light_product_tree.hpp` です。NTT だけを使う場合は `components/fps_light_ntt.hpp` を読み込みます。複数を include しても定義は一度だけで、通常版 FPS の実装は読み込みません。

FPS の型と基本演算は [基本演算の共通部品](fps_light_core.md) に置き、このヘッダーで積木と追加演算を組み合わせます。基本演算だけが必要な内部部品は `components/fps_light_core.hpp` を include できます。利用者は通常の `#include`・`oj-bundle` だけで使え、配布コード生成の手順はありません。
