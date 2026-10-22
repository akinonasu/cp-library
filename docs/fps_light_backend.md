---
title: "軽量 FPS の共通実装"
documentation_of: //components/fps_light_backend.hpp
---

軽量 FPS・NTT・追加演算の共通実装です。積木は FPS の型で共通化した[多項式の共通積木](polynomial_product_tree.md)を使い、合成・射影は[汎用実装](fps_composition_generic.md)を共有します。通常の C++ と ASCII の識別子で記述し、コード長を縮めるための特殊文字マクロは使いません。AVX2 の実行時判定、スカラー経路、Newton 更新での中間結果の再利用は保持しています。

公開ヘッダーは従来どおり `fps/fps_light.hpp`・`fps/fps_light_algorithms.hpp`・`components/fps_light_ntt.hpp`・`components/fps_light_product_tree.hpp` です。各ヘッダーから同じ実装を読み込み、複数を include しても定義は一度だけです。通常版 FPS の実装は読み込みません。

利用者は通常の `#include`・`oj-bundle` だけで使えます。メンテナーが修正する場合も、このヘッダーを直接編集します。高速実装と同様に、配布コード生成の手順はありません。
