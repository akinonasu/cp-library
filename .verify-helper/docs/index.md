名前・ファイル名・関数名で検索できます。各ページの **Bundle** で依存ヘッダーを展開し、**Copy** でコピーできます。

Bit Vector やアロケータなど、主にライブラリ内部で使う部品は [補助部品]({{ '/' | append: page.library_prefix | append: 'components/' | relative_url }}) にまとめています。

ライブラリをコピーするボタンを押すと 1 行目に参照 URL を含むコメント行が自動で添付されるようになっています。

従来実装を全て掲載する全体サイトと、64 KiB以内の実装だけを掲載する[軽量版サイト]({{ '/lightweight/' | relative_url }})を用意しています。両方にあるライブラリは同じ実装・APIです。サイズはUTF-8のバイト数で、参照URLのコメントと依存ヘッダーを含みます。詳細は[サイズ制限と提出方法]({{ '/docs/source_size.html' | relative_url }})を参照してください。

応用的なデータ構造・木のアルゴリズムは、一覧末尾の [応用ライブラリ]({{ '/' | append: page.library_prefix | append: 'data_structure/advanced/' | relative_url }}) にまとめています。
