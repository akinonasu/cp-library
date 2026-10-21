---
title: "提出コードの生成とサイズ上限"
---

ライブラリを使う C++ ソースから、依存ヘッダーを展開した提出用ファイルを生成します。

```sh
python3 tools/bundle.py a.cpp -I . -o submit.cpp
```

`oj-bundle` が PATH 上にあることが必要です。別の場所にある場合は `--bundle /path/to/oj-bundle` を指定します。`-I` は複数指定でき、省略時はカレントディレクトリです。

既定の上限は **256 KiB = 262,144 bytes 未満**です。通常の結合結果が上限未満なら、そのまま出力します。上限以上なら共通するトークン列を内部マクロへまとめ、コンパイル時に元の記述へ展開するコードを生成します。演算・公開 API・未使用関数・CPU 判定・他環境の分岐は削除しません。元のライブラリのヘッダーは編集しやすい形のまま保持しています。

Web サイトの **Bundle** ボタンも同じ上限を適用し、必要な場合だけ圧縮します。表示されるサイズは UTF-8 のバイト数です。生成結果は UTF-8 のまま保存してください。CLI は UTF-8・LF で書き出し、バイト数を上限検査と一致させます。

```sh
python3 tools/bundle.py a.cpp -I . --limit 65536 -o submit.cpp
wc -c submit.cpp
```

`--limit` で別の上限も指定できます。その値以上にしかできない場合はエラーとなり、出力先を更新しません。64 KiB 制限で FPS を使う場合は [軽量版](https://qandori.github.io/cp-library/fps/fps_light.hpp.html) を選びます。

通常の `oj-bundle` 自体の動作は変更していないため、大型 FPS などはそれだけで結合すると 256 KiB を超えます。既に結合したコードには、従来どおり次のコマンドも使えます。

```sh
python3 tools/compact_fps.py bundled.cpp --limit 262144 -o submit.cpp
```

公開ヘッダー・内部部品・Fast I/O を加えたコード・既存の提出例を検査しています。[測定範囲・サイズ・生成コードの検証](https://github.com/qandori/cp-library/blob/main/test/benchmark/source_size_256k.md)を参照してください。複数ライブラリや解法を加えた最終ソースの総サイズは、生成時の上限検査で確認します。
