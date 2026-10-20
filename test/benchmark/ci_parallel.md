# CI の並列実行

## 変更

- Linux の独立ヘッダー・単体テスト・ASan/UBSan・展開済みコードのコンパイルを 2 並列にする。
- 各コンパイルの一時ディレクトリを分ける。コンパイル・実行の失敗は CI を失敗させる。
- 検証結果キャッシュは同じ runner image の結果を優先する。内容・依存先・コンパイラ・環境の fingerprint の照合は維持する。
- GCC・Clang、サニタイザ、ジャッジのケース数は削減しない。Mac とローカルの既定値は 1 並列。

## GitHub Actions の調査

[9806574 の実行](https://github.com/qandori/cp-library/actions/runs/39454321626)では、GCC/Clang が約 24 分、ASan/UBSan が約 21 分かかり、順番に実行されていた。対象はヘッダー 181 件、単体テスト 162 件、サニタイザ 162 件。

この実行の runner image は `20261018.339.1`、[9b892ae の実行](https://github.com/qandori/cp-library/actions/runs/39461950769)は `20261012.334.1`。従来は直近の異なる image のキャッシュを復元していた。fingerprint が一致しないため、正しく全件再検証になっていた。同じ image のキャッシュを先に探すことで、この不要な復元先の選択を避ける。

[c8c0688 の実行](https://github.com/qandori/cp-library/actions/runs/39471541041)は既存の C++ 検証結果を再利用し、全 144 ジャッジドライバを再検証した。ジャッジは 25 分 53 秒、ドキュメント生成は 66 秒、展開コードの検証は 106 秒、サイト生成は 155 秒。CI 全体は約 34 分。選択対象が異なる実行の総時間を、そのまま速度比較には使わない。

## ローカル測定

`ci_parallel.py` で同じヘッダー 4 件と単体テスト 4 件を GCC/Clang で検証する。1 並列・2 並列を交互に各 2 回実行し、全プロセスが成功した時間の中央値を比較した。コンパイルフラグは通常の `test/verify.py` と同じ。

| 環境 | 1 並列 | 2 並列 | 時間比 |
| --- | ---: | ---: | ---: |
| Linux x86_64 / WSL2、GCC 15.2、Clang 21.1.8 | 37.541 秒 | 20.292 秒 | 0.541 |

結果と対象一覧は [ci_parallel.json](results/ci_parallel.json)。GitHub runner と異なる環境であり、CI 全体の短縮率を保証する測定ではない。テストを並列化したこと自体の局所測定。

```sh
python3 test/benchmark/ci_parallel.py --output test/benchmark/results/ci_parallel.json
```
