名前・ファイル名・関数名で検索できます。各ページの **Bundle** で依存ヘッダーを展開し、**Copy** でコピーできます。

Bit Vector やアロケータなど、主にライブラリ内部で使う部品は [補助部品]({{ '/' | append: page.library_prefix | append: 'components/' | relative_url }}) にまとめています。

ライブラリをコピーするボタンを押すと 1 行目に参照 URL を含むコメント行が自動で添付されるようになっています。

全体サイトと[軽量版サイト]({{ '/lightweight/' | relative_url }})は、同じ分類・項目順・機能ごとのURLで構成しています。FPSなど実装が異なる項目は「高速実装」「軽量実装」として相互に案内します。軽量実装がない項目は全体サイトへリンクします。サイズはUTF-8のバイト数で、参照URLのコメントと依存ヘッダーを含みます。詳細は[サイズ制限と提出方法]({{ '/docs/source_size.html' | relative_url }})を参照してください。

応用的なデータ構造・木のアルゴリズムは、一覧末尾の [応用ライブラリ]({{ '/' | append: page.library_prefix | append: 'data_structure/advanced/' | relative_url }}) にまとめています。
