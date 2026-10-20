# Grouped scatter 候補

全候補を見送りました。ユーザーから「Yosupo の入力に特化した fastest ではなく、一般に有効な高速化を行う」という追加の条件を受けたため、始点が整列した入力だけの fast path や入力先頭のサンプリングによる分岐は採用していません。これらは本実装の成功した高速化として数えません。

比較の基準は `c075fb7`、GCC 15.2 / Linux x86_64 / CPU 2、共通フラグは `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero`、native は `-march=native` を追加しました。各候補は Git から依存込みで凍結し、共有 `/tmp/cp-wm-opt.lock` の下で fresh process、Linux `/tmp` の入力・バイナリ・出力を使います。各試行で順序を seed `20261019` により無作為化し、warmup の経路を公式 checker または合成入力の辺で検証しました。

`active >= 8192` の先読み経路は変更せず、それ以外で比較しました。

| 候補 | 処理 |
| --- | --- |
| `all` | 同じ始点が連続する区間ごとに配置位置をレジスタに保ち、区間末尾で offsets へ一回保存 |
| `sorted` | 全辺の始点が非減少なら `all`、それ以外は従来の配置 |
| `sample64` / `sample128` | 先頭 64 / 128 辺の隣接する始点の 75% 以上が同じなら `all` |
| `emplace` | 頂点数 8192 未満かつ始点が非減少なら exclusive offsets を作り、Arc の初期化と散布を `reserve` / `emplace_back` に変更 |

各候補は入力の順序にかかわらず正しく動作する O(N+M) の配置で、辺順も保ちます。性能上の採用条件が今回のユーザーの希望に合わないため、ライブのヘッダーには反映していません。再構築・例外・汎用型を含めた本採用向けの unit / sanitizer 検証も実施していません。

## 予備測定

`grouped_native.json` は 9 ケース・各 17 回、`grouped_generic.json` は同じ 9 ケース・各 17 回、`shuffled_native.json` はランダムに並べ替えた密グラフ・各 31 回です。`emplace_native.json` は 9 ケース・各 17 回です。区間ドライバと実提出を分けています。全サンプルと区間中央値は JSON、集計は `summary.json` に保存します。

native 実提出の中央値で、`max_dense_random_01` は current 6.207ms、all 6.137ms、sorted 6.318ms、sample64 6.122ms、sample128 6.065ms でした。構築区間は current 1.409ms、sample64 1.205ms でした。generic でも current 6.145ms、sample64 6.026ms となりました。

既存の `synthetic_dense` も始点順に整列した生成器です。そこで、同じ重み・辺集合を seed `2026101905` で無作為に並べ替えた `synthetic_dense_shuffled` を追加しました。native 実提出は current 6.625ms、all 6.793ms、sample64 6.595ms、sample128 6.755ms、emplace 6.772ms でした。全体へ一律に run 配置を使うとランダムな密グラフでは追加判定の負担が出ており、整列した公式密グラフの改善だけでは一般的な採用理由として不十分です。

emplace の別 run は、密グラフの構築区間で約 1.3–1.4ms のままで、明確な短縮を得られませんでした。全辺の整列確認と `emplace_back` のコストが関係する可能性がありますが、命令単位の原因は未検証です。

最初の native run は公式密グラフ 4 ケースの検証・採時を完了した後、合成ケース検証の関数引数漏れで中断しました。引数を修正して残り 5 ケースを再開し、凍結ソース・フラグ・試行数が同じことを確認して `grouped_native.json` に統合しています。中断理由は JSON の `segments` にも残しています。

## 再実行

リポジトリ内から次を実行します。保存済みの全結果と Git revision / patch / source hash により、重複ヘッダーを保存せず再生成できます。

```sh
python3 test/benchmark/results/shortest_path_round5_grouped/prepare.py
python3 test/benchmark/results/shortest_path_round5_grouped/prepare_shuffled.py
python3 test/benchmark/results/shortest_path_round5_grouped/run.py --output grouped_native.json
python3 test/benchmark/results/shortest_path_round5_grouped/run.py --variants current all sample64 --generic --output grouped_generic.json
python3 test/benchmark/results/shortest_path_round5_grouped/run.py --variants current all sample64 sample128 emplace --cases synthetic_dense_shuffled --trials 31 --output shuffled_native.json
python3 test/benchmark/results/shortest_path_round5_grouped/run.py --variants current emplace --output emplace_native.json
```

診断後に runner のリポジトリ検出・出力先を再実行しやすい形へ調整しました。コンパイル・採時・候補の処理は同じです。結果の `source_hashes` と各 `.patch` が測定対象のソースを示します。入力の並べ替えは `shuffled_input.json` に元と並べ替え後の hash を残しました。
