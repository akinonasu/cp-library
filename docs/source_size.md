---
title: "サイズ制限と提出方法"
---

| サイト | 掲載するライブラリ |
| --- | --- |
| [全体サイト（256 KiB向け）](https://qandori.github.io/cp-library/) | 高速実装と軽量実装を掲載。64 KiBを超えるものも含む |
| [軽量版サイト（64 KiB）](https://qandori.github.io/cp-library/lightweight/) | 同じ分類・項目順で、65,536 bytes以内の実装を案内。未提供の項目は高速実装へリンク |

両サイトの分類・項目順を揃えています。実装が同じライブラリは通常側のURLを共通ページとし、両方の一覧・検索・依存リンクから同じページへ案内します。実装が異なる場合だけページを分けます。例えば形式的冪級数は、全体サイトの `fps/fps.hpp.html` と軽量版サイトの `lightweight/fps/fps.hpp.html` から参照できます。多項式 GCD・合成・合成逆元・射影・Taylor shift・Bostan–Mori・線形漸化式・多点評価・補間も、それぞれ同じ位置にあります。

実装が異なる場合は「高速実装」「軽量実装」と表示し、「軽量実装はこちら」「高速実装はこちら」のリンクで相互に移動できます。**APIやinclude先が異なる場合があるため、移動先の使用例を確認してください。** FPSの軽量実装は `light_fps` と `light_fps_algorithms`、FPSを使う畳み込みは `light_convolution` と `LightOnlineConvolution`、二項係数は `light_combinatorics` を使います。

軽量実装がない項目は「軽量実装なし」と表示し、通常版へリンクします。

サイズはUTF-8のバイト数です。1 KiBは1,024 bytesなので、64 KiBは65,536 bytes、256 KiBは262,144 bytesです。
サイズ制限には、依存を全て展開した `Bundle` とコピー時に付く参照 URL のコメント行も含まれます。
64 KiB以内の共通ページと軽量専用ページでは最初から展開コードを表示し、「展開済みをコピー」は常に依存込みのコードをコピーします。
64 KiB を超えるコードは軽量版サイトからコピーできません。

**制限を確認する対象は、最終的に提出するソース全体です。**
ライブラリ単体が収まっていても、解答本体・入出力・他のライブラリを追加すると上限を超える場合があります。
展開時に共通の依存は一度だけ出力されるので、単体サイズの単純な合計とは一致しません。

```sh
oj-bundle -I . a.cpp > submission.cpp
python3 tools/check_source_size.py submission.cpp --limit 64
```

依存の展開は通常の `oj-bundle` を使います。高速 FPS・軽量 FPS ともに通常の C++ のヘッダーを同梱し、必要な部品だけを展開します。[提出コードの生成](bundle.md)も参照してください。

```sh
oj-bundle -I . a.cpp > submission.cpp
python3 tools/check_source_size.py submission.cpp --limit 256
```

`check_source_size.py` は提出ファイルそのもののサイズを確認します。`--limit 64` または `--limit 256` を指定し、上限以下なら残りのバイト数を表示し、超過した場合は終了コード1を返します。改行コードやBOMもバイト数に含まれます。

サイトでも通常の依存展開だけを行います。FPSには通常版のほかに [軽量FPS](fps_light.md) と [軽量FPSの各種演算](fps_light_algorithms.md) があります。用途に合うAPIを選んでください。
最終的な提出コードでコンパイル・テストを行ってください。

64 KiBを超えた実装は軽量版のコピー対象から外れ、一覧には高速実装へのリンクを残します。
