# ライブラリの分類変更

46 ヘッダーを用途ごとのフォルダへ移動しました。[分類一覧](../../docs/library_layout.md)に新しい配置をまとめています。木上のアルゴリズムと動的森は `tree/` に集約し、グラフ・データ構造・数学も細分化しました。

アルゴリズム・型名・関数名の変更はありません。[manifest.json](results/library_layout/manifest.json)には、移動前後のパスと、include と既知のライブラリパス定数を除いた C++ ソースの hash を保存しました。移動前のソースと同一であることを照合しています。今回、速度測定は行っていません。

検証は、移動・include の変更箇所とその依存先だけを対象にしています。

- GCC・Clang: ヘッダー単独コンパイル各 52 件、単体テスト各 63 件。
- GCC: 判定用コード 66 本のコンパイル。
- CI の依存解析・差分検証などの Python テスト 142 件。
- 対象ヘッダーの Bundle 52 件の生成・コンパイル。
- 全ライブラリの README 掲載、説明ページ、階層別の検索カテゴリ、旧 URL 46 件の転送。

[C++ の検証記録](results/library_layout/validation.json)、[CI の検証記録](results/library_layout/python.log)、[説明ページの検証記録](results/library_layout/docs.json)を保存しました。検証用の一時コピーで補助ヘッダーが不足した初回の確認は失敗し、コピー対象を修正して未成功分だけを実行しました。成功済みの確認は、依存するソースとコンパイラ・フラグが同じ場合に再利用します。

```sh
python3 test/benchmark/library_layout_check.py
python3 test/benchmark/library_layout_docs.py
```

過去の測定結果・凍結したコード・単一ファイルの提出コードは変更していません。過去の比較実験は、測定資料に記録された Git revision で再現してください。旧説明ページからは移動後の URL へ転送します。
