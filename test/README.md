# テスト

- `unit/`: 愚直解・全探索・不変条件との比較による単体テスト。
- `*.test.cpp`: Library Checker・AOJ・yukicoder の公開ケースを使うジャッジ用テスト。
- `fixtures/`: テストが共有する型・比較用の正解実装。
- `verify.py`: ヘッダーの単独 include、単体テスト、ジャッジの実行。
- `verify_edit_distance_simd.py`: SIMDe による edit distance・LCS の SIMD 経路の検証。

```sh
python3 -m pip install -r test/requirements.txt
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2
python3 test/verify.py --unit-only --cxx clang++ --sanitize --jobs 2
python3 test/verify.py
```

AtCoder のジャッジ用テストは追加しません。一時的な計測コード・結果・ログはチェックアウト外の一時ディレクトリへ置き、リポジトリには保存しません。

macOS の移植性検証は ARM64 の `macos-15` で行います。Ubuntu の事前ジョブで変更ファイルを確認し、`docs/`・`submissions/`・ルートの `README.md`／`AGENTS.md` だけの変更では macOS ジョブを省略します。ライブラリ・単体テスト・CI 設定などの変更、差分を確定できない場合、明示的な no-cache 検証では実行します。手動の `workflow_dispatch` でも実行できます。

`macos-14` は [GitHub の廃止告知](https://github.com/actions/runner-images/issues/13518)により計画停止の対象となり、2026-11-02 にサポートが終了するため使用しません。
