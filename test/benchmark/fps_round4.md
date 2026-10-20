# FPS の追加高速化 4

今回の対象は convolution。公開 API、入力の不変性、Montgomery 表現の不変条件を維持して、Linux x86_64 / GCC 向けの経路を改善した。

## 採用した変更

- 短辺が 32 より大きく、長辺が 32768 以上かつ短辺の 32 倍以上の場合、短辺の変換を一度だけ行い、長辺をブロックに分ける。結果の全長零初期化・加算を省き、重複部分だけ加算する。[非対称畳み込み](fps_convolution_unbalanced_round4.md)
- 両入力が 4096 項以上で各列の全係数がそれぞれ一定なら、NTT を省略し、重なりの項数から線形時間で積を作る。先頭 8 項での早期棄却後も末尾まで厳密に確認し、入力を変更しない。[定数係数の経路](fps_convolution_constant_round4.md)
- 専用 `Product` に加え、汎用 `ntt::Convolution` でも返す `vector<Mint>` を作業配列 A として利用する。生存中の要素を使い、alignment と末尾 16 係数の零領域を保証する。既存の二引数コンストラクタと `run_borrowed` は維持する。[汎用 NTT の借用](fps_convolution_borrow_round4.md)

最終比較の基準は `bd29f70c70e24e27770b9e9a3777b6cf85664b63`。調査開始時の `7521ea8703a362aaf3ccd370188fccab86050848` から別作業の Dijkstra 更新を取り込み、共有 `fast_io_bulk.hpp` の更新を基準・変更後の双方へ揃えた。途中の候補測定は `7521ea8` 基準の別セッションとして保存し、最終結果に加算しない。

調査時点の [fastest 409465](https://judge.yosupo.jp/submission/409465) は judge 表示 12 ms、近傍の [409433](https://judge.yosupo.jp/submission/409433) と [409432](https://judge.yosupo.jp/submission/409432) は 13 ms。[取得した一覧](results/fps_convolution_borrow_round4/reference_heads.json)と未編集の原本を保存した。原本 SHA-256 は `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。ローカルの時間を judge の時間へ換算しない。

## convolution の比較

[最終版の 13 ケース比較](fps_convolution_constant_final_round4.md)では、非対称な公式 4 ケースが中央値で 35〜39%、定数係数の公式 2 ケースが 29〜31% 短縮した。非対称の 4 ケースはいずれも未編集の最速原本も上回った。

最大ランダム入力では、この最終比較で基準に対し paired で 0.164 / 0.209 ms の退行があった。同一 binary を使った[独立した再比較](fps_convolution_branch_round4.md)では 0.193 / 0.082 ms の短縮となり、小さい差の方向は安定しなかった。改善したとは主張しない。最終比較の最大ランダムは約 12.7 ms、原本は約 9.8 ms で、明確な差が残る。

AMD EPYC 9V74、CPU 2 固定、GCC 14.2、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。各ケースで warmup 1 回と記録 21 回、実行順を乱択し、プロセス起動・入出力・終了を含めた。最終比較の 858 出力を公式 checker で確認した。前後の source、実際の compiler dependency、compiler、binary、input、answer の SHA-256 は一致した。原票・圧縮 source・差分・再現 script は [最終比較一式](results/fps_convolution_constant_final_round4/)にある。[定数経路追加前の比較](fps_convolution_round4.md)は別 snapshot として保持した。

## 原本との差と不採用の案

[工程別比較](fps_convolution_phases_round4.md)では、係数の読み込み、確保・初期化、結果準備、解放・flush に差があった。kernel 単独の時間は first touch の位置が異なるため、その大小だけで演算の優劣を断定しない。準備と kernel の合計には差が残る。この分析は採用前の別 snapshot に対するもので、最終版の残差の直接の内訳にはしない。

- [SIMD・演算 kernel](fps_convolution_kernel_round4.md)：部分木サイズ、静的再帰、AVX512VL の 32 レジスタ化、葉の register window、Karatsuba 分割、twiddle のレジスタ保持を比較した。演算数が減っても並べ替え・退避などの負担が増え、採用しなかった。
- [結果領域の構築・確保](fps_convolution_allocation_round4.md)：range / batch / iterator による構築、最終段の直接書き込み、正規化位置、追加容量など 12 案を比較した。追加容量 1 MiB は小さい改善の傾向があったが、配置依存とメモリ増加があり、今回は採用しなかった。
- [入出力](fps_io_round4.md)：Montgomery 変換の位置、転置、出力乗算の置換、入力の `resize`、追加容量を比較した。9 回で改善した候補も 21 回では再現せず、採用しなかった。

## 最終版の各 FPS 関数への波及

各問の最大入力を 21 回比較した別セッション。16 問 × 3 実装 × 22 実行の 1056 出力が checker を通過した。変更後 16 本を最終 source からビルドし、基準・参照原本の 32 本は初期比較の binary と source の SHA-256 一致を確認して再利用した。旧時点に記録していなかった compiler 実体・全 system dependency の hash を取得済みとは扱わない。新しい source と実依存・compiler・全 binary・fixture は前後一致した。単位は ms。

参照元は既存の `fps_speed_reference.json` に記録した C++ 原本であり、今回一覧を再取得したのは convolution。小さい差はばらつきを含む。[最終原票](results/fps_round4_constant_formal.json)、[再現と確認方法](fps_round4_constant_formal.md)、[定数経路追加前の原票](results/fps_round4_formal.json)。

| 問題 | 基準 | 最終版 | 参照原本 |
| --- | ---: | ---: | ---: |
| `convolution_mod` | 12.770 | 12.426 | 9.958 |
| `inv_of_formal_power_series` | 11.790 | 11.720 | 10.255 |
| `log_of_formal_power_series` | 13.369 | 13.458 | 12.286 |
| `exp_of_formal_power_series` | 16.499 | 16.434 | 14.736 |
| `pow_of_formal_power_series` | 24.336 | 24.530 | 22.768 |
| `sqrt_of_formal_power_series` | 12.556 | 12.447 | 10.226 |
| `division_of_polynomials` | 5.966 | 6.010 | 4.796 |
| `kth_term_of_linearly_recurrent_sequence` | 53.625 | 53.469 | 57.351 |
| `composition_of_formal_power_series` | 2.804 | 2.823 | 2.748 |
| `composition_of_formal_power_series_large` | 33.260 | 33.281 | 32.495 |
| `compositional_inverse_of_formal_power_series` | 2.587 | 2.621 | 2.553 |
| `compositional_inverse_of_formal_power_series_large` | 30.215 | 30.230 | 29.663 |
| `multipoint_evaluation` | 15.506 | 15.454 | 14.709 |
| `polynomial_interpolation` | 19.698 | 19.861 | 19.374 |
| `polynomial_taylor_shift` | 10.130 | 10.194 | 8.906 |
| `inv_of_polynomials` | 15.466 | 15.542 | 14.920 |

## 検証

定数経路追加前の統合版では GCC14 / Clang19 の各 21 standalone headers・27 units、Sanitizer 4 units、公式 22 問 596 ケースが成功した。Yukicoder 2 件は token がなく明示 skip とした。[初期統合版の検証](results/fps_round4_validation/README.md)

追加の定数経路は、新 header・公開 header・`fps.cpp`・専用 unit の GCC14 / Clang19・ASan/UBSan、GCC13 と非 Linux guard の standalone を確認した。完全一致する compiler / flags / source / 実依存 / binary / log を拘束できた専用 unit だけは成功証拠を再利用し、既存全体の検証を無条件には繰り返さなかった。最終版で公式 convolution 全 53 ケースも再確認した。[追加検証](results/fps_round4_constant_validation/README.md)、[公式全53ケース](results/fps_round4_constant_validation/convolution-audit.json)

最終 docs は 277 source 生成、152 library metadata、依存差分の 21 bundles を検証し、601 source の前後 SHA-256 が一致した。既知の Boost 未導入による Yuki BigInt metadata 警告は非 fatal として保存した。

検証後、別作業の上流 `c075fb733c2954ce303eb6a7fa1ef78977a611f9` を取り込んだ。CI cache・CSR・テストの `maybe_unused` 属性などの更新で、測定した 16 問の実際の project 依存 59 ファイルとは重ならず、最終作業ツリーの全依存内容が一致した。元の raw/gate は書き換えず [取り込み後の照合](results/fps_round4_upstream_audit.json)を別に保存した。
