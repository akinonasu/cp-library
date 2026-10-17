# Library Checker の fastest 候補

2026-10-18 に公開 API の AC 提出を時間順に確認した。既存の測定から候補を選び、ライブラリと FastInput / FastOutput を使う提出全体を比較した。全ライブラリの網羅的な順位調査ではない。

## 結論

最有力は [Stern–Brocot Tree](https://judge.yosupo.jp/problem/stern_brocot_tree)。[提出用コード](../../submissions/stern_brocot_tree.cpp) を C++23 に貼り付けて使える。外部ヘッダーは不要で、32 bit 型は問題の制約に合わせている。GCC 用の追加 pragma は使用していない。

[Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) は同率最速の候補として [コード](../../submissions/general_matching.cpp) を用意した。現在の 2 ms からの単独更新を期待できる測定結果ではない。

各公式ケースを 7 回測った中央値について、その最大値を比較する。最大になるケースは実装間で異なる。単位は ms。

| 問題 | 今回 | 参照提出 | ローカルの最大中央値 | 公開ジャッジの表示 | 判定 |
| --- | ---: | --- | ---: | ---: | --- |
| Stern–Brocot Tree | 18.049 | [381187 / Rust](https://judge.yosupo.jp/submission/381187) | 22.090 | 31 | 有望 |
| 同上 | 18.049 | [405753 / C++](https://judge.yosupo.jp/submission/405753) | 31.819 | 35 | C++ の上位より短い |
| Matching on General Graph | 1.998 | [404900](https://judge.yosupo.jp/submission/404900) | 2.096 | 2 | 同率の候補 |
| 同上 | 1.998 | [386231](https://judge.yosupo.jp/submission/386231) | 1.995 | 2 | 同程度 |
| 同上 | 1.998 | [384089](https://judge.yosupo.jp/submission/384089) | 1.802 | 2 | 参照の方が短い |
| Range Chmin Chmax Add Range Sum | 202.498 | [204482](https://judge.yosupo.jp/submission/204482) | 187.993 | 225 | 今回は見送り |

公開ジャッジへの提出はしていない。ローカル時間と公開ジャッジの表示時間は直接比較できず、実際の fastest 獲得を保証するものではない。特に 2 ms 付近の順位は計測の粒度と起動時間の影響が大きい。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、CPU 2 に固定。C++ は GCC 15.2.0、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native`。assert は有効。[公式言語設定](https://github.com/yosupo06/library-checker-judge/blob/master/langs/langs.toml) の C++23 の指定に合わせた。CPU と OS は公開ジャッジとは異なる。

Rust 参照は元ソースのまま、`rustc --edition 2024 -C opt-level=3 -C target-cpu=native`。手元は rustc 1.97.1、公開 API の表示は 1.94.1 であり、Rust のコンパイラは一致していない。

公式データのリビジョンは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。問題ごとに公開一覧の上位 3 提出を取得した。Stern–Brocot の 405753 と 327748 は元ソースのハッシュが一致する。

各ケース・各実装を 1 回ウォームアップしてから、固定 seed 20261018 で実行順をシャッフルして 7 回計測した。実行ファイル・入力・出力は Linux の `/tmp` に置く。通常ファイルを stdin / stdout に渡し、新しいプロセスの起動、入力、データ構造の構築、計算、出力、終了までの wall time を測る。コンパイル、入力のコピー、公式 checker、ハッシュ計算は時間外。測定中は他のコンパイル・テストを実行していない。子プロセスの CPU time も生データに残したが、上表は wall time を使う。

取得日時、ランキング、ソース SHA-256、全入力・公式出力の SHA-256、全試行の出力 SHA-256 と時間は以下に保存した。

- [Segtree Beats・Matching の条件](results/fastest_candidates/environment.json)・[全測定値](results/fastest_candidates/results.json)
- [Stern–Brocot の条件](results/fastest_candidates_stern/environment.json)・[全測定値](results/fastest_candidates_stern/results.json)

## ケース別の比較

Stern–Brocot はユークリッド互除法で経路を圧縮し、祖先と LCA は経路配列を作らず途中で打ち切る。参照 C++ は外積による探索と経路の生成を使う。Rust は 32 bit の分数の境界を使い、decode が特に速い。全ケースで今回の方が速いわけではない。

| Stern–Brocot のケース | 今回 | Rust 381187 | C++ 405753 |
| --- | ---: | ---: | ---: |
| random_decode_00 | 18.049 | 8.445 | 21.186 |
| random_encode_00 | 15.253 | 15.137 | 31.819 |
| random_lca_00 | 5.688 | 22.090 | 8.823 |
| random_ancestor_yes_00 | 8.615 | 19.540 | 21.290 |
| random_range_00 | 12.978 | 12.552 | 24.274 |

一般グラフのマッチングは Blossom の縮約に加えて、初期マッチングと短い増加路の前処理を使う。404900 / 386231 は DSU で縮約する BFS、384089 も初期マッチングと短い増加路を使う。計算部分の改善だけでは、入出力を含む 2 ms 程度の提出での順位更新を断言できない。

Segtree Beats はランダムな大区間での差が大きい一方、`random3` のように最大・最小を少しずつ詰める公式入力では参照の反復処理が速かった。今回のドライバーを FastInput / FastOutput に変更し、比較用の [単一ファイル](../../submissions/range_chmin_chmax_add_range_sum.cpp) も残したが、fastest の推奨には含めない。

| Segtree Beats のケース | 今回 | 204482 |
| --- | ---: | ---: |
| max_random_00 | 84.578 | 187.993 |
| random2_00 | 176.643 | 151.104 |
| random3_02 | 202.498 | 178.255 |
| issue_1369_log2_03 | 111.324 | 77.057 |

追加で GCC の O3、ループ展開、NDEBUG を重い 5 ケースで比較した。各 1 回ウォームアップ・5 回測定、CPU 2、通常ファイルでの入出力と公式 checker を使う。O3 だけでの短縮は小さく、全体で最速提出を上回る根拠にならなかったため採用しない。[条件と全試行](results/fastest_candidates_tuning.json)。

## 検証・再生成

提出する単一ファイルそのものをコンパイルし、Segtree Beats 33 ケース、Matching 12 ケース、Stern–Brocot 18 ケースを公式 checker で検証した。GCC 15.2 の計測時に全試行を検査し、Clang 21.1.8 と GCC 13.4 では各ケースを 1 回ずつ `-std=c++2b -O2 -Wall -Wextra -Werror` で検証した。ヘッダーの実装は変更していない。Clang / GCC 13 の時間は性能比較に使わない。

- [Clang の条件](results/fastest_candidates_clang/environment.json)・[検証結果](results/fastest_candidates_clang/results.json)
- [GCC 13 の条件](results/fastest_candidates_gcc13/environment.json)・[検証結果](results/fastest_candidates_gcc13/results.json)

公式データは `library-checker-problems/generate.py` で各問題の `info.toml` から生成する。参照提出の取得と単一ファイルの再生成、検証、測定は次のコマンドで行う。再取得時はランキングと測定対象が変わる場合がある。

```sh
python3 test/benchmark/fastest_candidates.py \
  --official /path/to/library-checker-problems \
  --cache .verify-helper/cache/fastest-candidates \
  --output .verify-helper/cache/fastest-candidates/new-results --refresh
python3 test/benchmark/fastest_candidates.py \
  --official /path/to/library-checker-problems \
  --cache .verify-helper/cache/fastest-candidates \
  --output .verify-helper/cache/fastest-candidates/clang-check \
  --cxx clang++ --check-only
```

`--problems stern_brocot_tree` などで対象を絞れる。`--check-only` は保存済みの単一ファイルとドライバーの展開結果が一致することも検査する。
