# Shortest Path 追加測定 5

一般的な入力での改善を確認できる候補が得られなかったため、高速化の追加は見送りました。`DirectedCSR`・Dijkstra・入出力の実装は `c075fb733c2954ce303eb6a7fa1ef78977a611f9` と同じです。辺の順序・頂点番号・問題名による分岐は追加しません。測定資料と、アラインメントの大きい重み・例外処理のテストを追加します。

## 前回の約 3.8% の調査

[前回](shortest_path_round4.md)は `max_dense_random_01` の実提出の全体時間が `bd29f70` 7.492 → `c075fb7` 7.774 ms でした。同じ run の区間ドライバは入力 4.289 → 4.148 ms、構築 1.734 → 1.718 ms、探索 0.226 → 0.219 ms、全体 7.754 → 7.473 ms と逆の傾向でした。

この入力は 707 頂点・499142 辺で、全頂点の出次数が 706 です。先読みの条件である active 行数 8192 に達せず、先読み本体は実行されません。追加された行数の集計は 707 回です。

Git revision ごとに依存先を凍結した [再現比較](results/shortest_path_round5_reproduce/README.md)では、native 31 回の実提出は 6.281 → 6.202 ms、generic 21 回は 6.135 → 6.331 ms でした。同じ試行を対とした bootstrap の変化率の 95% 区間は、それぞれ -4.10%～+3.65%、-1.79%～+5.32% です。native の構築区間は 1.330 → 1.360 ms で、この約 0.030 ms だけで前回の約 0.282 ms の差を説明できません。

[生成コードの比較](results/shortest_path_round5_variants/codegen/README.md)では、実提出の GCC 15.2 native の関数本体をローカルラベルの名前を揃えて比較しました。変更のある `DirectedCSR<uint32_t>::finish` を除く 29 関数は一致し、parser・探索・出力の命令やレジスタ割り当ての退行は見つかりませんでした。コード配置やホストの状態による影響までは排除していません。

したがって、固定的な 3.8% の退行は再現せず、原因を一つに特定できていません。別の密グラフでは native の実提出に約 3.3% の増加もあり、全ての差が誤差であるとも断定しません。全体時間と区間時間を分け、中央値と全試行を保存します。

## 検討して見送った初期化省略

従来の `std::vector<std::pair<int,T>>::resize` は、各ペアを値初期化してから、全要素を辺の値で上書きしていました。候補では、整数・浮動小数点などの自明な重み型に対して、内部の `OverwriteArray` で配列の領域だけを確保し、各格納先に `std::construct_at` で辺を一度ずつ構築しました。全辺を必ず一度ずつ配置する CSR の構築手順を利用します。

重みが自明な型で、辺の構築とコピーが例外を送出せず、辺の破棄も自明な場合に使います。それ以外は既存の `std::vector` を使います。入力の辺順、`std::pair<int,T>` の公開型、連続する `std::span`、`size_t` の辺数、コピー・移動・容量の再利用を維持します。

`std::allocator::allocate` は配列の寿命だけを開始し、要素の寿命は `std::construct_at` で明示的に開始します。`std::pair` の表現を仮定した `memcpy` や `vector` の内部書き換えは使いません。根拠は [WG21 P0593R6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0593r6.html)、[P0784R7](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0784r7.html) と [規格レビューの記録](results/shortest_path_round5_storage_review.json) に残します。

## 見送った候補と参照実装

[codegen の候補](results/shortest_path_round5_variants/codegen/README.md)では active 集計の省略、ループ分離、ポインタの配置位置、代入順序を比較しました。密グラフで一部が速くても疎グラフなどで増加し、安定した一般的な改善が得られず見送りました。

[grouped scatter の候補](results/shortest_path_round5_grouped/README.md)では、連続する同じ始点をまとめる方法、整列確認、入力先頭のサンプリングを比較しました。公式の密グラフと既存の合成 dense は始点順に整列しているため、これらだけで採用すると評価が偏ります。ユーザーの一般性の条件に従い、整列した入力だけの fast path や先頭のサンプリングは採用しません。`OverwriteArray` の [予備測定](results/shortest_path_round5_variants/storage/README.md)も最終版とは分けて保存しました。

参照した AC 提出は [407097](https://judge.yosupo.jp/submission/407097) と [404592](https://judge.yosupo.jp/submission/404592) です。[公式 API の調査記録](results/shortest_path_round5_reference.json)に取得日時・順位・ソース hash・手法を保存しました。CSR、radix queue、単一目的地への上界打ち切りなどを比較しています。32 bit の辺位置や行内の逆順配置をそのまま移すと API の条件が変わるため、今回は配列の二重の書き込みを減らす実装を独立に作りました。API の過去の実行環境は判明しておらず、本番順位や本番での速度は断定しません。

## 最終候補の測定条件

Linux x86_64 / WSL2、GCC 15.2、CPU 2 固定、共有 lock `/tmp/cp-wm-opt.lock` の下で測定します。フラグは `-std=gnu++23 -O2 -DNDEBUG -DATCODER -DONLINE_JUDGE -ftrivial-auto-var-init=zero`、native は `-march=native` を追加します。入力・出力・実行ファイルは Linux `/tmp` に置きます。各試行で実行順を変え、各ケース・各バイナリは warmup 1 回と fresh process 21 回です。実提出の全体時間にはプロセス起動・終了、確保・破棄、経路出力も含みます。

公式 29 ケースと既存の合成 7 ケースに、seed `2026101905` で作る合成 5 ケースを追加します。dense・sparse・次数の偏ったグラフ・一本道・双方向の星で、全て辺順と頂点番号をランダム化し、始点・終点にも同じ置換を適用します。新しい dense は公式や既存 dense と重みも異なる独立したグラフです。全 41 ケースの入力 hash と全試行を保存します。

以下は候補をライブのヘッダーに置いた時点の測定コマンドです。現在は候補を戻しているため、保存結果の `current` / `own_path` は本採用したライブラリとは異なります。

```sh
python3 test/benchmark/shortest_path_round5.py \
  --output test/benchmark/results/shortest_path_round5_native \
  --trials 21 --native --submissions
python3 test/benchmark/shortest_path_round5.py \
  --output test/benchmark/results/shortest_path_round5_generic \
  --trials 21 --submissions
```

測定ドライバの依存先と実提出は実行前に凍結します。warmup の経路を公式 checker または合成入力の辺で検証し、全試行で距離を照合します。終了時にライブソース・凍結ソース・runner の hash が変わっていないことを確認します。

保存済みの候補を実提出の全体時間で再測定する例です。候補のヘッダーをライブに戻す必要はありません。

```sh
python3 test/benchmark/shortest_path_round5.py --reference-only --native --trials 21 \
  --output .verify-helper/cache/shortest-path/round5/replay \
  --reference own_baseline=test/benchmark/results/shortest_path_round5_native/sources/own_baseline/0.cpp \
  --reference own_path=test/benchmark/results/shortest_path_round5_native/sources/own_path/0.cpp \
  --reference own_topo=test/benchmark/results/shortest_path_round5_native/sources/own_topo/0.cpp
python3 test/benchmark/shortest_path_round5_summary.py --allow-archived-source \
  --output .verify-helper/cache/shortest-path/round5/archived_summary.json
```

`--allow-archived-source` は保存済みの全結果を再集計するための指定です。保存ソースと結果の hash は検査し、現在のライブソースとの一致は要求しません。初回の集計は、この指定なしでライブソースの一致も確認しています。[再集計の検証](results/shortest_path_round5_reanalysis.json)では、全ケースの統計と bootstrap 区間が初回の結果と一致しました。

## 候補の結果と不採用の理由

[native 全試行](results/shortest_path_round5_native/results.json)、[generic 全試行](results/shortest_path_round5_generic/results.json)、[集計・分布・bootstrap 区間](results/shortest_path_round5_summary.json)を保存しました。単位は ms、前は `c075fb7`、後は初期化省略の候補です。実提出の全体時間を比較します。

| ケース | native 前 | 後 | 変化 | generic 前 | 後 | 変化 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `max_dense_random_01` | 6.449 | 6.271 | -2.75% | 6.599 | 6.525 | -1.12% |
| `max_dense_random_00` | 6.561 | 6.268 | -4.47% | 6.492 | 6.424 | -1.04% |
| `max_sparse_random_01` | 13.103 | 13.063 | -0.31% | 13.327 | 13.371 | +0.33% |
| `line_00` | 20.945 | 20.774 | -0.82% | 20.760 | 20.411 | -1.68% |
| `synthetic_dense_shuffled` | 8.059 | 7.935 | -1.54% | 7.128 | 6.861 | -3.74% |
| `synthetic_sparse_shuffled` | 12.397 | 12.528 | +1.05% | 9.786 | 9.701 | -0.87% |
| `synthetic_skew_shuffled` | 25.782 | 26.483 | +2.72% | 19.073 | 19.288 | +1.13% |
| `synthetic_line_permuted` | 21.579 | 21.574 | -0.02% | 21.023 | 20.971 | -0.25% |
| `synthetic_star_shuffled` | 19.443 | 20.445 | +5.16% | 12.356 | 12.780 | +3.43% |

`max_dense_random_01` の native の変化率の bootstrap 95% 区間は -6.73%～-1.24%、generic は -5.55%～+0.05% でした。一方、ランダム化した星の native は +0.23%～+12.51% で、全体時間の増加が残ります。

区間ドライバの構築だけを見ると、`max_dense_random_01` は native 1.431→1.265、generic 1.537→1.247 ms です。しかし、ランダムな密グラフは native 2.189→2.343、generic 1.932→2.228 ms と逆転しました。ゼロ初期化を省くことで最初のページ書き込みが散布に移る影響は考えられますが、ページフォールトやメモリアクセスを十分に観測しておらず、原因は確定していません。

全 41 ケースで最大の時間増加と増加率は、native が `max_star_01` の 13.190→14.937 ms（+1.747 ms、+13.25%）、generic が `grid_random_00` の 9.002→9.476 ms（+0.474 ms、+5.26%）でした。変更前が 5ms 以上のケースに限っても同じです。native の星の区間測定は入力 7.100→9.198 ms が主な増加で、構築は 2.625→2.666 ms、探索は約 0.73 ms とほぼ同程度でした。全ての増加を散布の変更そのものに帰せるとは限りません。

一部の公式密グラフが速くなるだけでは、専用の配列管理と寿命管理を追加する根拠として不十分と判断しました。一般の辺順・次数分布での一貫した利益を確認できず、候補を取り下げました。元の実装を維持するので、前回の疎グラフ向けの先読みもそのままです。

## 検証

候補の両設定・全 41 ケース・各 21 回で距離照合と warmup の経路検証が成功しました。[候補の検証](results/shortest_path_round5_variants/storage/formal_validation.json)は GCC・Clang の unit 14 件、ASan / UBSan 8 件、公式 checker 174 件です。[候補の文書検証](results/shortest_path_round5_variants/storage/formal_docs.log)も保存しました。これらは不採用の候補に対する記録です。

本実装には、128 byte alignment の重み、辺順・コピー・移動・再構築、非自明な重みのコピー例外と生存数のテストを追加します。`python3 test/benchmark/shortest_path_round5_check.py` は、現行の `DirectedCSR` に対して追加した unit だけを GCC・Clang と各 ASan / UBSan で検証します。[現行版の検証記録](results/shortest_path_round5_validation.json)に保存します。変更のない Dijkstra のテスト・公式 checker は再実行しません。
