---
title: "FPS の配布実装"
documentation_of: //components/fps_backend.hpp
---

公開ヘッダーから読み込む共通の配布実装です。高速実装と[軽量実装](https://qandori.github.io/cp-library/components/fps_light_backend.hpp.html)は別ファイルで、通常の `#include`・`oj-bundle` だけで使えます。利用時にコードを圧縮する処理はありません。

高速実装の編集用原本は `fps/*.inc` と `components/*.inc` にあります。軽量実装は通常の C++ のヘッダーを直接編集します。メンテナーが原本を変更した場合だけ、次のコマンドで配布実装を更新します。利用者には不要です。

```sh
python3 tools/update_fps_headers.py
python3 tools/update_fps_headers.py --check
```

共通する記述を一時的な内部マクロにまとめ、通常版の内部変数名を短くしています。内部マクロはヘッダー末尾で解除します。演算、入力の const 性、境界検査、CPU の実行時判定、他環境の分岐は原本と同じです。短縮されたレイアウトに由来する `misleading-indentation` 診断だけを配布実装の内部で抑制し、ヘッダーの末尾で診断設定を復元します。
