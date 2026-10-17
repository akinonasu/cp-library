# Interval Set

整数の半開区間を `std::map` に圧縮します。隣接区間を結合し、挿入ヒント、包含時の早期終了、ノード抽出による左端変更を使います。右端の伸縮は既存ノードを直接変更します。番兵や座標に比例する配列は使いません。

## 参考

2026-10-17 UTC に [Predecessor Problem の AC 実行時間順](https://judge.yosupo.jp/submissions?problem=predecessor_problem&status=AC&order=%2Btime) の先頭 3 件を取得しました。取得時の一覧、実行時間、ソースの SHA-256 は `results/interval_set_reference.json` にあります。

| 提出 | 取得時の実行時間 | 手法 |
| --- | ---: | --- |
| [403023 / nandhagk](https://judge.yosupo.jp/submission/403023) | 16 ms | 64 分木の階層ビット集合。16 要素以下は SIMD 検索を併用。AVX2 入出力 |
| [278227 / chaihf](https://judge.yosupo.jp/submission/278227) | 20 ms | 固定段数の階層ビット集合。AVX2 で初期文字列をビット化 |
| [269275 / mukundan314](https://judge.yosupo.jp/submission/269275) | 20 ms | 固定段数の階層ビット集合。AVX2 による初期化、mmap 入力 |

[公式問題](https://judge.yosupo.jp/problem/predecessor_problem) は座標範囲が既知で、更新は単点です。上記の実装はこの条件を利用しており、区間追加・削除を直接扱いません。Interval Set は広い座標上の連続区間を扱う用途を優先します。ビット集合方式をそのまま採用すると、64 bit の広い座標や長い区間の更新に向きません。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、各組を 7 回、実装の順序を固定 seed でシャッフルして逐次実行した中央値です。他のテストやベンチマークとは並行実行していません。

- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra`。内部処理の比較には `-Werror` も指定。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- 参考提出 403023 は明示的な AVX2 を要求するため、両設定で追加の `-mavx2` を指定。ソースは変更せずコンパイル。
- 完全なコンパイラ・CPU 情報、フラグ、入力・ソースの SHA-256、生データは各結果ディレクトリに保存。

AtCoder の実機では未測定です。native が常に速いという結果ではなく、最速性も主張しません。

## 区間更新

seed は 731。初期構築・データ生成・破棄を除き、操作とチェックサム更新を測定します。比較対象の `baseline` は区間を削除して作り直す素朴な `std::map` 実装です。包含する区間の追加でもノードを再作成します。各試行で返り値・最終要素数・全区間のチェックサムが一致することを確認します。

| 操作列 | 内容 | generic 素朴 / 本実装 | native 素朴 / 本実装 |
| --- | --- | ---: | ---: |
| grow | 範囲 100,000 内に単点を 400,000 回追加 | 72.05 / 38.29 ms | 71.48 / 36.87 ms |
| range | 長さ 0～256 の区間を 300,000 回追加・削除 | 46.57 / 28.73 ms | 46.47 / 28.85 ms |
| fragmented | 100,000 区間から開始し、単点更新・検索を 300,000 回 | 67.31 / 60.90 ms | 64.91 / 59.28 ms |
| split | 長さ 10^12 の区間内で削除・復元を 400,000 組 | 28.12 / 11.41 ms | 27.14 / 11.93 ms |
| sliding | 長さ 10^6 の区間を 400,000 回右へ移動 | 20.63 / 7.59 ms | 19.44 / 7.28 ms |
| query | 100,000 区間に所属・前後要素・mex を 500,000 回問い合わせ | 90.83 / 91.33 ms | 87.17 / 88.69 ms |

この素朴実装との比較では更新が改善し、検索のみではほぼ同等です。測定結果は `results/interval_set_generic/`・`results/interval_set_native/` にあります。

## Predecessor Problem

公式ケースの入力読み込み、初期構築、操作、出力、プロセス起動・終了を含む実時間です。標準入力は通常ファイルを使い、出力の全トークンを公式解答と比較します。出力を受け取る親プロセスも CPU 2 に固定されるため、ジャッジ上の実行時間とは直接比較できません。

| 公式ケース | generic 本実装 / 参考提出 | native 本実装 / 参考提出 |
| --- | ---: | ---: |
| max_random_00 | 1,041.80 / 59.16 ms | 1,056.36 / 65.27 ms |
| max_all1_00 | 253.06 / 60.17 ms | 345.92 / 84.94 ms |
| max_sparse_00 | 86.95 / 56.30 ms | 86.61 / 58.35 ms |

単点操作に限定したこれらのケースでは、参考提出の階層ビット集合が速い結果です。結果は `results/interval_set_judge_generic/`・`results/interval_set_judge_native/` にあります。

## 検証

- GCC 15.2 / Clang 21.1.8 でヘッダー単体・既存ライブラリとの同時 include・単体テスト。
- 25,000 回のランダム区間更新を小さな配列と比較。所属、包含、交差、要素数、前後要素、mex、全区間を確認。
- 符号付き・符号なし 8 / 32 / 64 bit の端点、長さ 2^64−1 の区間、コピー・移動・交換を確認。
- ASan / UBSan、GCC 13.4、AArch64 GCC + QEMU で単体テスト。
- Predecessor Problem の公式 22 ケースが GCC・Clang の双方で AC。区間操作そのものは単体テストで検証。
- API 表・ドキュメント・Bundle を検証。Mac の動作は macOS CI で確認。

## 再現

```sh
python3 test/benchmark/interval_set_reference.py --cache .verify-helper/cache/interval-set-references --output test/benchmark/results/interval_set_reference.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/interval_set_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/interval_set_selection.json
oj-verify run --timeout 1800 --tle 10 -j 2 test/interval_set_predecessor.test.cpp
python3 test/benchmark/interval_set.py --output test/benchmark/results/interval_set_generic
python3 test/benchmark/interval_set.py --native --output test/benchmark/results/interval_set_native
python3 test/benchmark/interval_set_judge.py --cache .verify-helper/cache/interval-set-references --cases-dir .verify-helper/cache/857afc16bc9bf8d936851d86478706ec/test --output test/benchmark/results/interval_set_judge_generic
python3 test/benchmark/interval_set_judge.py --cache .verify-helper/cache/interval-set-references --cases-dir .verify-helper/cache/857afc16bc9bf8d936851d86478706ec/test --native --output test/benchmark/results/interval_set_judge_native
```

`--cases-dir` には `oj-verify` が取得した Predecessor Problem の `test/` を指定してください。
