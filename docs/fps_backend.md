---
title: "FPS の互換ヘッダー"
documentation_of: //components/fps_backend.hpp
---

[FPS 本体](https://qandori.github.io/cp-library/fps/fps.hpp.html)を読み込む互換ヘッダーです。新しいコードでは `fps/fps.hpp` を直接 include してください。合成・補間・GCD などの追加演算は、それぞれの公開ヘッダーを include します。

高速実装と軽量実装は、どちらも通常の C++ の `.hpp` ファイルを直接編集します。共通の型・演算は型エイリアス、関数、テンプレートにまとめ、必要な部品だけを読み込みます。短縮用の辞書マクロや、配布コードを生成する手順はありません。

利用時は通常の `#include`・`oj-bundle` を使います。[提出コードの生成とサイズ上限](bundle.md)も参照してください。
