---
title: "提出コードの生成とサイズ上限"
---

通常の `oj-bundle` で依存ヘッダーを展開します。専用の提出用スクリプトや、展開後の追加圧縮は不要です。

```sh
oj-bundle -I . a.cpp > submit.cpp
wc -c submit.cpp
```

高速 FPS は短縮済みの配布ヘッダー、軽量 FPS は通常の読みやすい C++ のヘッダーを保存しています。通常版の全 API 使用例と基本の `FastInput`・`FastOutput` は **256 KiB = 262,144 bytes 未満**、軽量版の全 FPS 演算と同じ入出力は **64 KiB = 65,536 bytes 未満**になることを、後処理のない `oj-bundle` の出力で検査します。公開 API・CPU 判定・他環境の分岐・精度上限は維持しています。生成後のファイルは UTF-8 のまま保存してください。

Web サイトの **Bundle** も依存ヘッダーの展開だけを行います。サイト生成時には参照 URL を付けた Copy のサイズも検査します。上限を超えたコードを、自動圧縮して表示することはありません。

64 KiB 制限では [軽量版 FPS](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) を使います。解答部分や他のライブラリを追加した最終ソースの大きさは `wc -c` で確認してください。
