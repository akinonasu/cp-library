---
title: "軽量 FPS の共通実装"
documentation_of: //components/fps_light_backend.hpp
---

軽量 FPS と追加演算の共通部品です。通常は公開ヘッダーを include してください。

FPS の公開ヘッダーは `fps/fps_light.hpp`・`fps/fps_light_algorithms.hpp`、積木は `components/fps_light_product_tree.hpp` です。NTT だけを使う場合は `components/fps_light_ntt.hpp` を読み込みます。複数を include しても定義は一度だけで、通常版 FPS の実装は読み込みません。

FPS の型と基本演算は [基本演算の共通部品](fps_light_core.md) に置き、このヘッダーで積木と追加演算を組み合わせます。基本演算だけが必要な内部部品は `components/fps_light_core.hpp` を include できます。
