# Project Selection

2026-10-21。`graph/flow/project_selection.hpp` に、最小カットによる二値変数の費用最小化を追加しました。

## 参考と構成

[Library Checker の公式問題一覧](https://github.com/yosupo06/library-checker-problems/blob/master/categories.toml)には、Project Selection に直接対応する問題がないため、該当する fastest 提出との比較は行っていません。最大流は既存の `MaxFlow<uint64_t>` を再利用し、その参考提出・測定記録は [最大流・最小費用流](flow.md) を参照してください。今回、最大流本体は変更していません。

[ABC326 G - Unlock Achievement](https://atcoder.jp/contests/abc326/tasks/abc326_g)と[公式解説](https://atcoder.jp/contests/abc326/editorial/7475)の、段階的なスキル・報酬・依存関係を最小カットへ変換する方法を確認しました。検証コードでは、各レベルの費用に `add_cost`、上位レベルから下位レベルへの依存に `imply`、実績の報酬に `add_all_profit` を使います。

単項コストは頂点ごとに集約し、零容量辺と自己辺を省きます。二変数の劣モジュラコストは定数項・単項項・有向辺 1 本に分解します。全員を同じ側に置く利益は補助頂点 1 個と有限容量辺で表し、利益の大きさを容量として使います。

必須・禁止・依存関係の辺は有限コストと分離し、最大流を実行する前に、制約辺だけの到達可能性で実行不能を判定します。有限容量の総和より 1 大きい値を制約辺の容量に使います。実行可能なら最小カットは有限容量の総和以下なので、制約辺が切られることはありません。矛盾がある場合に多数の「無限大」を流量として合算することも避けます。

入力は符号付き 64 bit 以下、単項項の集約・定数項・合計費用は符号付き 128 bit、辺の容量は符号なし 64 bit です。最小の符号付き 64 bit 整数の否定も、128 bit へ拡張してから行います。

## 検証

- `int`・`int64_t` のそれぞれで 1,200 個のランダムな問題。変数数は 0〜8、追加操作は 24 回、seed は `20261021`。全選択の列挙と最適値・復元した選択の費用・実行可能性を比較しました。
- 単項費用・利益、二変数項、全員の利益、空の頂点列、重複頂点、自己辺、必須・禁止・依存・同一選択を混在させ、求解後の追加と再求解、コピーの独立性も確認しました。
- 各値を -2〜2 とする劣モジュラな二変数コスト表 355 種について、選択を全 4 通りに固定し、それぞれの費用を確認しました。
- `INT64_MIN` の費用・利益、64 bit を超える定数項、`UINT64_MAX-1` の有限容量、「無限大」の合計が容量型を超える矛盾した制約。
- 20 万頂点の依存関係の鎖で、最適値、禁止による選択の変更、矛盾を確認しました。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入。
- `NDEBUG`・警告をエラーにした設定で、GCC・Clang の利用例と検証コードをコンパイルしました。
- ABC326 G の公式 3 サンプル。さらにスキル数 1〜5・実績数 1〜8 の独立な 150 ケースで、全レベル組合せを列挙した答えと、両コンパイラの検証コードの出力を比較しました。全て一致しました。公開ジャッジへの提出は行っていません。

検証結果・ソースのハッシュは `results/project_selection/validation.json` に保存しています。今回、実行時間の比較測定は行っていません。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/project_selection_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/project_selection_selection.json
oj-verify run test/project_selection.test.cpp
```
