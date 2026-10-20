# Segtree Beats のベンチマーク

2026-10-16 に [Range Chmin Chmax Add Range Sum](https://judge.yosupo.jp/problem/range_chmin_chmax_add_range_sum) の実行時間順の AC 提出を調査しました。[204482](https://judge.yosupo.jp/submission/204482) と [272331](https://judge.yosupo.jp/submission/272331) の、最大・最小の第 2 値と個数を保持する方式、および反復処理による区間分解を確認しています。

比較用の提出コードは `segtree_beats_reference.py` が API から一時取得します。204482 の入出力部分を除き、区間を半開区間へ変換するアダプターを付けています。代入は提出側の実装条件が異なるため、この比較から除外しました。取得した元ソースの SHA-256 は `35769e566e67a92e76c60870f11add94a9985a27796c2b109220d6aefde2b4c3` です。

## 再現方法

GitHub Actions の `segtree beats benchmark` は手動実行です。`verify` を無効にすると、変更のない本体の単体・sanitizer・公式テストを省き、比較実験だけを実行します。

```sh
python3 test/benchmark/segtree_beats_variants.py /tmp/beats-variants
python3 test/benchmark/segtree_beats_reference.py /tmp/beats-variants
g++ -std=c++23 -O2 -I . -I /tmp/beats-variants test/benchmark/segtree_beats.cpp -o /tmp/beats-bench
/tmp/beats-bench 200000 200000 5 > /tmp/beats.csv
```

引数は要素数・操作数・試行数です。入力生成と入出力を計測から除き、各試行で実装の実行順をシャッフルします。構築時間と操作時間を分けて記録し、全実装・全試行の checksum を照合します。`-DNDEBUG` を付けた測定も行います。

| 操作列 | 内容 |
| --- | --- |
| `mixed` | ランダム区間の chmin・chmax・加算・和 |
| `short` | 同じ更新・取得、区間長は最大 64 |
| `assign` | 代入・最小・最大の取得も含める |
| `clamp` | chmin・chmax・和 |
| `uniform` | 同じ値から始め、全区間更新とランダム区間の和 |

## 比較した工夫

| 名前 | 変更 |
| --- | --- |
| `compact` | 前順配置の再帰実装。ノードは $$2n-1$$ 個 |
| `heap` | 同じ処理を通常のヒープ配置にし、$$4n$$ 個確保 |
| `aligned` | ノードを 64 バイト境界に整列 |
| `plain_push` | 均一区間の伝播時に子の値を確認し、同値なら書き込みを省略 |
| `plain_query` | 均一区間へのクエリでも再帰する |
| `prefetch` | 更新先の子をプリフェッチ |
| `iterative_update` | 2 冪に切り上げ、更新の区間分解と再集約を反復処理 |
| `iterative` | クエリも反復処理 |
| `equal_first` | 再集約で最大値・最小値の一致を先に判定 |
| `branchless` | 再集約を条件式中心に変更 |
| `noinit` | `compact` の不要なゼロ初期化を省略。採用版 |
| `reference_204482` | 参考にした AC 提出のコア実装 |

`compact` は、採用版から allocator の変更と構築時の加算タグ初期化だけを戻して生成します。測定時の比較元は `f288aad7a2c18e477b41c9b7dafef65e6c4e7679` の `data_structure/segtree_beats.hpp` と同一です。

均一区間は代入として伝播し、クエリは再帰せず長さから計算します。chmin の上限が区間最小値以下、chmax の下限が区間最大値以上なら、区間全体を直接代入します。更新・取得の種類はテンプレート引数で分けています。

既定のノードは 56 バイトです。20 万要素のノード配列は 22,399,944 バイトで、`heap` の 44,800,000 バイトから約半分になります。2 冪に切り上げる実装のノード配列は同条件で約 29.36 MB です。入力やテスト用の配列は含みません。

## Linux/GCC での測定

[最初の比較](https://github.com/qandori/cp-library/actions/runs/38845092408) で配置・反復処理・整列・プリフェッチを比較し、[追加比較](https://github.com/qandori/cp-library/actions/runs/38845371738) で残りの調整と AC 提出を比較しました。環境・CSV・参照ソースのハッシュは artifact `segtree-beats-linux-gcc-benchmark` にあります。

追加比較の環境は Ubuntu 24.04 / AMD EPYC 7763 / GCC 13.3.0、`-std=c++23 -O2` です。5 試行の中央値、単位は ms、構築時間を除いた操作時間です。同じ列の値を同じジョブ内で比較しています。

### 20 万要素・20 万操作、assert 有効

| 実装 | mixed | short | assign | clamp | uniform |
| --- | ---: | ---: | ---: | ---: | ---: |
| `heap` | 92.74 | 92.15 | 79.48 | 67.40 | 2.53 |
| `compact` | 84.80 | 83.38 | 74.64 | 64.49 | 2.50 |
| `aligned` | 87.09 | 85.01 | 74.09 | 64.09 | 2.55 |
| `plain_push` | 87.45 | 84.23 | 73.53 | 64.86 | 2.62 |
| `plain_query` | 96.84 | 85.53 | 94.89 | 78.42 | 18.69 |
| `prefetch` | 86.18 | 85.57 | 73.18 | 64.44 | 2.53 |
| `iterative_update` | 118.53 | 94.73 | 94.86 | 104.61 | 24.61 |
| `iterative` | 126.97 | 93.42 | 111.00 | 116.42 | 37.52 |
| `equal_first` | 88.99 | 84.75 | 73.87 | 63.58 | 2.62 |
| `branchless` | 91.33 | 88.98 | 78.58 | 71.10 | 2.55 |
| **採用版 `noinit`** | **85.32** | **85.19** | **73.78** | **64.67** | **2.51** |
| `reference_204482` | 194.76 | 91.18 | — | 145.93 | 62.43 |

採用版はこの条件で参照実装に対し、mixed 約 2.28 倍、short 約 1.07 倍です。構築時間の全操作列・全試行をまとめた中央値は `compact` 3.66 ms、採用版 3.42 ms でした。

### 262144 要素・20 万操作、`-DNDEBUG`

| 実装 | mixed | short | assign | clamp | uniform |
| --- | ---: | ---: | ---: | ---: | ---: |
| `compact` | 94.98 | 93.82 | 80.21 | 70.41 | 2.43 |
| **採用版 `noinit`** | **95.28** | **94.01** | **80.42** | **67.96** | **2.51** |
| `reference_204482` | 211.49 | 102.11 | — | 154.44 | 70.20 |

構築時間の全操作列・全試行をまとめた中央値は `compact` 5.15 ms、採用版 4.42 ms でした。allocator の変更は構築を改善し、操作時間はほぼ同等です。整列・プリフェッチ・分岐順の変更には安定した改善がなく、反復処理はこの操作列で遅くなったため採用していません。

### 採用版の再測定

[最終実行](https://github.com/qandori/cp-library/actions/runs/38845695237) も AMD EPYC 7763 / GCC 13.3.0 でした。同じ設定・同じ checksum での中央値です。

| 条件 | 実装 | mixed | short | assign | clamp | uniform |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| 20 万、assert 有効 | 採用版 | 102.36 | 101.54 | 87.97 | 76.25 | 2.52 |
| 20 万、assert 有効 | 204482 | 225.72 | 107.57 | — | 167.12 | 72.69 |
| 262144、NDEBUG | 採用版 | 110.42 | 118.17 | 95.68 | 80.48 | 2.52 |
| 262144、NDEBUG | 204482 | 250.01 | 119.67 | — | 172.74 | 76.32 |

再測定でも mixed は参照実装の約 2.2〜2.3 倍でした。一方、262144 要素の short は `heap` 107.57 ms、`iterative` 109.83 ms と、採用版より速い結果です。通常のヒープ配置や反復処理が有利な条件もあります。採用版はメモリ量と混合更新の結果を優先しています。異なるジョブ間の絶対時間は比較せず、それぞれのジョブ内で比較してください。

これは同じ GCC 条件でのコア部分の比較です。全入力での優位性や judge 上での最速は保証しません。実際の提出時間には入出力と実行環境の差も含まれます。

単体テストは愚直な配列とのランダム照合、空区間、非 2 冪長、均一区間への更新の連続、単調な clamp、コピー、`__int128` の和を確認します。採用版の [最終検証](https://github.com/qandori/cp-library/actions/runs/38845695237) で、Range Chmin Chmax Add Range Sum の公式 33 ケースを GCC・Clang の両方で通過しました。ASan/UBSan も通過しています。測定対象の採用版は `712a932f7f3aa2302cd365bfbc8bdeb77b6b2fdc` のヘッダーです。
