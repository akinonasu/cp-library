# ACL と比較したデータ構造・グラフの追加

2026-10-21。通常の Union-Find、Fenwick Tree、固定長の遅延セグメント木、SCC、2-SAT を追加しました。新規のヘッダーと通常のテストに ACL の依存はありません。関数名・テンプレート引数は ACL に近い設計ですが、クラス名はそれぞれ `UnionFind`・`FenwickTree`・`LazySegtree`・`SCCGraph`・`TwoSAT` です。

## 対象の選択

| ACL の機能 | 今回の扱い |
| --- | --- |
| dsu | 未追加だった通常の構造を追加。成分列挙を効率化 |
| fenwick_tree | 公開ライブラリを追加。共通祖先の省略と選択可能なパディング |
| lazy_segtree | 固定長版を追加。作用の有無を記録し、不要な伝搬・合成を省略 |
| scc / two_sat | 公開ライブラリを追加。共通の反復 SCC を使用 |
| segtree | 今回は追加なし。汎用の固定長版について ACL より速い根拠をまだ得ていない |
| maxflow / mincostflow | 既存の `graph/flow/max_flow.hpp`・`min_cost_flow.hpp` があるため追加なし |
| convolution / convolution_ll | 既存の `convolution/` のライブラリを使用 |
| modint | 既存の `math/mint.hpp` があるため追加なし。任意法の ACL modint も新規 Fenwick・Lazy の型として使用可能 |
| suffix_array / lcp_array | 既存の `components/suffix_array.hpp`・`string/suffix_tree.hpp` に構築機能があるため今回の追加対象から除外 |
| z_algorithm | 今回は追加なし。ACL より速い根拠をまだ得ていない |
| pow_mod / inv_mod / floor_sum | 既存の数論・floor sum の実装を使用 |
| crt | 今回は追加なし。ACL より速い根拠をまだ得ていない |

## 参考にした公開 AC 提出

取得日時、当時のランキング上位 2 件の一覧、ソースの SHA-256 は [reference.json](results/acl_structures/reference.json) に保存しています。表の時間は公開 judge の表示値で、今回のローカル測定ではありません。

| 問題 | 提出 | 表示時間 | 比較した手法 |
| --- | --- | ---: | --- |
| Unionfind | [400979](https://judge.yosupo.jp/submission/400979) / [393688](https://judge.yosupo.jp/submission/393688) | 5 / 5 ms | サイズ併合と経路半減。今回も反復的な経路半減を採用 |
| Point Add Range Sum | [403014](https://judge.yosupo.jp/submission/403014) / [258491](https://judge.yosupo.jp/submission/258491) | 23 / 23 ms | 多分木の累積和と AVX2 更新。今回は型と演算への条件を増やさない Fenwick を追加 |
| Range Affine Range Sum | [402323](https://judge.yosupo.jp/submission/402323) / [408770](https://judge.yosupo.jp/submission/408770) | 82 / 88 ms | 多分木・作用の保留状態・modular sum の専用カーネル。汎用版に適用できる伝搬省略を採用 |
| SCC | [404707](https://judge.yosupo.jp/submission/404707) / [403026](https://judge.yosupo.jp/submission/403026) | 22 / 29 ms | 反復 Tarjan、入次数 0 の除去、functional graph の閉路処理。頂点数に固有のビット詰めは使わない |
| 2 Sat | [405839](https://judge.yosupo.jp/submission/405839) / [403031](https://judge.yosupo.jp/submission/403031) | 38 / 38 ms | 含意の閉包探索と SCC フォールバック。今回は共有 SCC による線形時間の解法と、恒真節・固定節の辺削減を採用 |

新しい各実装は一般の入力に対応します。公開提出で使われている入力の先読み・専用 I/O・将来クエリの prefetch は今回の時間に含めません。公開 judge への提出は行っておらず、fastest を取れたという測定ではありません。

## 測定条件

Linux x86_64 / AMD EPYC 9V74 の仮想環境、GCC 14.2、CPU 2 固定。同じ実際の ACL と新規実装を、同じソース・乱数 seed・操作列で比較しました。`-std=c++23 -O2 -DNDEBUG`、追加の命令セット指定なしと `-march=native` ありの 2 条件です。警告フラグ・全コマンド・ACL のコミット・ソースハッシュは [benchmark.json](results/acl_structures/benchmark.json) にあります。

各条件 9 回の中央値です。実行順を毎回シャッフルし、各実行は新しいプロセスです。ほかのテスト・測定とは排他実行しました。各シナリオは次のとおりです。

- Union-Find / Fenwick / Lazy は `N=200000, Q=500000`。操作列と初期化は測定前に用意します。成分列挙は 20 万頂点を 1 成分に併合した後の `groups()` 1 回です。判定のみの入力も、同じ併合の後に検索を始めます。
- Fenwick は加算と和が半々です。prefix は `sum(0,r)` を呼びます。`ours` は既定のパディングあり、`unpadded` はパディングなしです。
- Lazy は両者とも同一の **ACL `modint998244353`** と同一の集約・アフィン作用を使います。自作 modint の高速性は含みません。`lazy_point` は区間作用がなく、不要な恒等作用の伝搬を省ける条件です。
- SCC は `N=200000`。random と DAG は辺を 60 万回生成し、DAG では自己ループを除きます。cycle は 20 万辺、path は 199999 辺です。`scc()` の成分配列生成までを含みます。
- 2-SAT は 10 万変数・30 万節で、節の登録後の `satisfiable()` を測ります。SAT の代入検証と `answer()` のコピーは時間に含めません。
- 入出力・プロセス起動・正解検証は測定から除きます。ACL の再帰 SCC が大きな閉路でスタック不足にならないよう、**両実装ともスタック上限を同じ環境の hard limit まで拡張**しています。新規 SCC 自体は DFS 再帰を使いません。
- Union-Find / Fenwick / Lazy は返り値の checksum、SCC は各頂点の所属成分の最小頂点、2-SAT は充足可能性を比較します。充足可能なら返った代入を全節で別途検証します。

## 9 回の中央値

比率は `ACL / 追加版` です。1 より大きければ追加版が速いことを示します。

### 命令セット指定なし

| 条件 | ACL ms | 追加版 ms | 比率 |
| --- | ---: | ---: | ---: |
| Union-Find：併合・判定半々 | 7.799 | 7.628 | 1.02 倍 |
| Union-Find：判定のみ | 3.903 | 4.562 | 0.86 倍 |
| Union-Find：1 成分の列挙 | 2.379 | 0.788 | 3.02 倍 |
| Fenwick：ランダム区間 | 11.343 | 11.880 | 0.95 倍 |
| Fenwick：長さ 128 以下 | 11.200 | 9.605 | 1.17 倍 |
| Fenwick：prefix | 8.927 | 8.904 | 1.00 倍 |
| Lazy：区間更新・取得半々 | 447.178 | 332.205 | 1.35 倍 |
| Lazy：区間更新 10% | 401.155 | 268.795 | 1.49 倍 |
| Lazy：一点代入・区間取得半々 | 299.229 | 49.555 | 6.04 倍 |
| SCC：ランダム有向グラフ | 23.664 | 16.901 | 1.40 倍 |
| SCC：頂点番号をシャッフルした DAG | 17.961 | 13.920 | 1.29 倍 |
| SCC：1 個の閉路 | 14.792 | 2.555 | 5.79 倍 |
| SCC：頂点番号順のパス | 19.273 | 5.902 | 3.27 倍 |
| 2-SAT：解のあるランダム式 | 18.718 | 15.589 | 1.20 倍 |
| 2-SAT：ランダム式（今回の入力は充足不能） | 22.872 | 15.715 | 1.46 倍 |
| 2-SAT：矛盾しない固定節 | 6.546 | 3.594 | 1.82 倍 |

### -march=native

| 条件 | ACL ms | 追加版 ms | 比率 |
| --- | ---: | ---: | ---: |
| Union-Find：併合・判定半々 | 7.558 | 7.549 | 1.00 倍 |
| Union-Find：判定のみ | 3.648 | 4.637 | 0.79 倍 |
| Union-Find：1 成分の列挙 | 2.335 | 0.841 | 2.78 倍 |
| Fenwick：ランダム区間 | 10.986 | 10.339 | 1.06 倍 |
| Fenwick：長さ 128 以下 | 10.667 | 8.943 | 1.19 倍 |
| Fenwick：prefix | 8.448 | 7.925 | 1.07 倍 |
| Lazy：区間更新・取得半々 | 435.332 | 359.896 | 1.21 倍 |
| Lazy：区間更新 10% | 396.381 | 291.710 | 1.36 倍 |
| Lazy：一点代入・区間取得半々 | 274.046 | 47.517 | 5.77 倍 |
| SCC：ランダム有向グラフ | 25.175 | 17.255 | 1.46 倍 |
| SCC：頂点番号をシャッフルした DAG | 19.190 | 14.331 | 1.34 倍 |
| SCC：1 個の閉路 | 20.428 | 2.727 | 7.49 倍 |
| SCC：頂点番号順のパス | 21.855 | 6.148 | 3.55 倍 |
| 2-SAT：解のあるランダム式 | 20.120 | 16.742 | 1.20 倍 |
| 2-SAT：ランダム式（今回の入力は充足不能） | 23.525 | 16.107 | 1.46 倍 |
| 2-SAT：矛盾しない固定節 | 6.862 | 3.665 | 1.87 倍 |

Union-Find は成分列挙が約 2.8〜3.0 倍になりました。一方、判定のみでは追加版の方が約 17〜27% 遅い結果です。併合と判定が半々ではほぼ同速でした。通常の検索を目的に ACL から置き換えれば速くなる、という結果ではありません。

Fenwick は短い区間で約 1.17〜1.19 倍でした。命令セット指定なしのランダム区間では約 5% 遅く、prefix はほぼ同速です。次のパディングなしも比較してください。

| 条件 | パディングあり ms | なし ms | ACL ms |
| --- | ---: | ---: | ---: |
| generic / Fenwick：ランダム区間 | 11.880 | 11.242 | 11.343 |
| generic / Fenwick：長さ 128 以下 | 9.605 | 10.242 | 11.200 |
| generic / Fenwick：prefix | 8.904 | 8.965 | 8.927 |
| native / Fenwick：ランダム区間 | 10.339 | 10.744 | 10.986 |
| native / Fenwick：長さ 128 以下 | 8.943 | 8.340 | 10.667 |
| native / Fenwick：prefix | 7.925 | 9.180 | 8.448 |

パディングの有無で最良の条件は変わります。遅延セグメント木・SCC・2-SAT は今回の全シナリオで改善しましたが、対象 CPU・入力によって比率は変わります。AtCoder / Library Checker 実機での時間を保証する測定ではありません。

生データは [CSV](results/acl_structures/benchmark.csv) に保存しています。

## 検証

- GCC 14.2 / Clang 19 で新規 6 ヘッダーの単独・二重 include と、利用者マクロへの非依存を確認しました。
- Union-Find はラベル配列、Fenwick は配列の加減算、Lazy は愚直配列と照合。負数・整数の wrapping・128 bit 整数・空列・境界を含みます。
- Lazy は等値比較を持たないアフィン作用と、非可換な文字列連結モノイドの両方を検証しました。境界検索の左右の演算順も独立した文字列の prefix/suffix と照合しました。
- SCC は 4 頂点以下の全 66067 有向グラフを到達可能性の推移閉包と照合し、多重辺と 50 万頂点の閉路も検証しました。
- 2-SAT は変数数 0〜8 の計 4500 式を全割り当て探索と照合し、節追加後の再判定・恒真節・固定節・コピーの独立性を確認しました。
- 上記単体テストは GCC / Clang、Clang ASan/UBSan で成功しました。新規 6 ドキュメントの例も両コンパイラでコンパイル・実行し、実際の ACL `modint1000000007` を使った Fenwick の 10000 操作も愚直配列と一致しました。
- Library Checker の公式生成器の全ケースを GCC / Clang で検証しました。Unionfind 18、Point Add Range Sum 21、Range Affine Range Sum 19、SCC 12、2 Sat 18 の計 88 入力です。SCC と 2-SAT は非一意な答えを公式 checker で検証しました。生成器・公式正解はスタック上限を拡張して実行しています。

公式入力・正解のハッシュ、検証コードのハッシュ、使用した公式生成器のコミットとケース数は [validation.json](results/acl_structures/validation.json) にあります。

```sh
python3 test/verify.py --unit-only --selection test/benchmark/acl_structures_selection.json --cxx g++ clang++
python3 test/verify.py --unit-only --sanitize --selection test/benchmark/acl_structures_selection.json --cxx clang++
python3 test/benchmark/acl_structures_check.py --problems /path/to/library-checker-problems
python3 test/benchmark/acl_structures.py --acl /path/to/ac-library --cpu 2 --trials 9
```
