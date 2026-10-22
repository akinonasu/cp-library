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
