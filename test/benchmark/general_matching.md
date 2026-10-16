# General Matching のベンチマーク

2026-10-16 に [Matching on General Graph](https://judge.yosupo.jp/problem/general_matching) の実行時間順の AC 提出を確認しました。最速は複数の提出が 2 ms で並んでいます。

- [404900](https://judge.yosupo.jp/submission/404900)・[386231](https://judge.yosupo.jp/submission/386231): DSU で Blossom を縮約する BFS 型の厳密解を確認しました。
- [88850](https://judge.yosupo.jp/submission/88850)・[71785](https://judge.yosupo.jp/submission/71785): 配列による隣接リストと、縮約後に base をまとめて更新する方式を確認しました。
- [384089](https://judge.yosupo.jp/submission/384089): 初期マッチングと長さ 3 の増加路による前処理を確認しました。

比較用の 404900 は `general_matching_reference.py` が API から一時取得します。コア比較ではテンプレート・入出力を除き、マクロを展開し、相手の頂点を `-1` に揃える薄いアダプターを付けます。入出力込みの比較では元の提出コードをそのままコンパイルします。元ソースの SHA-256 は `a78f1bdaa77cf8582600e100797a1f837e934dc03f422bb0ddd5b01d508afad5` です。実行 artifact にも保存します。

## 再現方法

GitHub Actions の `general matching benchmark` は手動実行です。`verify` を無効にすると、変更のない本体の単体・sanitizer・公式テストを省き、合成グラフでの比較だけを行います。

```sh
python3 test/benchmark/general_matching_variants.py /tmp/matching-variants
python3 test/benchmark/general_matching_reference.py /tmp/matching-variants
g++ -std=c++23 -O2 -I . -I /tmp/matching-variants test/benchmark/general_matching.cpp -o /tmp/matching-bench
/tmp/matching-bench 500 5 20 > /tmp/matching.csv
```

引数は頂点数・試行数・試行内の反復数です。頂点ラベルと辺順を固定シードでシャッフルし、各試行で実装の実行順もシャッフルします。`exposed_path` は内部のマッチングを先に作り、長い増加路が残る順序にします。

構築・辺追加と `build()` を分けて測定し、両者の合計を比較します。入力生成・入出力・正解照合はコア計測から除きます。各反復で相手の対称性、実際の辺の存在、頂点の重複がないこと、組数を確認し、全実装の組数を照合します。

| グラフ | 内容 |
| --- | --- |
| `sparse` | 平均次数がおよそ 6 の乱数グラフ |
| `dense` | 各辺を確率 1/2 で追加 |
| `complete` | 完全グラフ |
| `odd_cliques` | 9 頂点ずつの完全グラフ |
| `star`・`path`・`tree` | 星形・パス・ランダム木 |
| `bipartite` | 左右同数の疎な二部グラフ |
| `bipartite_skew` | 片側が全体の 1/8 の二部グラフ |
| `blossom_chain` | 三角形を辺で連結 |
| `exposed_path` | 長い増加路を残すパス |

## 比較した変更

| 名前 | 内容 |
| --- | --- |
| `plain` | CSR と DSU を使う Blossom 法。初期マッチングなし |
| `greedy` | 辺追加中に初期マッチングを作り、全体の上限に達したら探索を省略 |
| `reverse` | 探索する根の順序を逆転 |
| `short` | 長さ 3 の増加路を 1 巡処理してから Blossom 法 |
| `short2` | 前処理を 2 巡 |
| `free_first` | CSR の走査で空き頂点を先に探す |
| `bitset` | 1024 頂点以下をビット集合で保持し、空き頂点との積を先に調べる |
| `bitset_short`・`bitset_short2` | ビット集合と短い増加路の前処理を組み合わせる |
| `components` | 連結成分ごとの上限を求め、二部成分は小さい側からだけ探索 |
| `components_short`・`components_short2` | 上記と短い増加路の前処理を組み合わせる |
| `current` | 現在の採用版 |

採用版は `components_short` を基に、不要な作業配列の確保も遅延させています。初期マッチングだけで全体の上限に達した場合は隣接リストを構築せず終了します。それ以外は短い増加路を処理し、CSR でグラフを保持します。二部成分では左右の小さい方、それ以外は成分の頂点数の半分を上限とします。上限が埋まらない場合は Blossom 法で厳密に解きます。

ビット集合は密なグラフを改善する場合がありましたが、星形・偏った二部グラフ・奇数クリークで遅くなりました。前処理 2 巡も安定した改善がなく、採用していません。

## 検証

6 頂点以下の全グラフと 18 頂点以下の乱数グラフを部分集合 DP と照合します。辺の追加後の再構築、コピー、自己ループ、多重辺、未マッチ頂点も確認します。4096 頂点までの長い増加路と連続する Blossom も検証します。

## Linux/GCC での実測

[初回比較](https://github.com/qandori/cp-library/actions/runs/38851726795)、[成分の上限を追加した比較](https://github.com/qandori/cp-library/actions/runs/38852208572)、[最終版](https://github.com/qandori/cp-library/actions/runs/38852504276) です。CSV・環境情報・公式テストのログは artifact `general-matching-linux-gcc-benchmark` にあります。

以下は最終版の測定です。Ubuntu 24.04 / Intel Xeon Platinum 8573C / GCC 13.3.0、`-std=c++23 -O2`。採用版は `6aef348a111e408f9b391ecc54b45c90206680b5` の `graph/general_matching.hpp` と一致します。

### 合成グラフ

500 頂点、assert 有効、各試行で 20 回解いた平均の 5 試行中央値です。辺追加と探索の合計、単位は µs です。

| グラフ | 初期マッチングのみ | 採用版 | 最速 AC 404900 |
| --- | ---: | ---: | ---: |
| `sparse` | 30.32 | 28.29 | 126.07 |
| `dense` | 501.80 | 150.63 | 450.73 |
| `complete` | 189.07 | 187.88 | 672.73 |
| `odd_cliques` | 30.49 | 16.14 | 130.20 |
| `star` | 86.61 | 6.55 | 95.12 |
| `tree` | 23.94 | 14.98 | 77.55 |
| `bipartite` | 17.10 | 19.08 | 110.39 |
| `bipartite_skew` | 528.83 | 14.38 | 507.53 |
| `blossom_chain` | 16.95 | 16.31 | 85.23 |
| `exposed_path` | 5.71 | 10.19 | 65.78 |

2000 頂点でも比較しました。偏った二部グラフは参照実装 9262.99 µs に対して採用版 107.99 µs、星形は 1278.94 µs に対して 25.00 µs でした。一方、長い増加路だけが残るケースや 2000 頂点の sparse では、前処理を省いた案が速いこともあります。前処理の走査量と、その後に削減できる探索量に差があります。

### 公式 12 ケースのコア部分

公式入力をそのまま使用し、5 試行・各 20 回の中央値です。`-O2 -DNDEBUG`、辺追加・初期化・探索を含み、入出力と結果照合を除きます。単位は µs です。

| ケース | 採用版 | 最速 AC 404900 | 倍率 |
| --- | ---: | ---: | ---: |
| `example_00` | 0.094 | 0.482 | 5.16 |
| `example_01` | 0.205 | 0.241 | 1.18 |
| `issue610_00` | 0.511 | 0.814 | 1.59 |
| `max_random_00` | 42.324 | 265.505 | 6.27 |
| `max_random_01` | 157.298 | 521.321 | 3.31 |
| `random_00` | 3.999 | 53.749 | 13.44 |
| `random_01` | 23.317 | 143.247 | 6.14 |
| `sparse_00` | 25.130 | 89.544 | 3.56 |
| `sparse_01` | 5.423 | 82.074 | 15.13 |
| `sparse_02` | 3.527 | 80.219 | 22.75 |
| `sparse_03` | 36.683 | 126.055 | 3.44 |
| `sparse_04` | 19.944 | 80.706 | 4.05 |

小さいケースは時計の計測費用の影響を受けます。最大ケースの改善は約 3.3〜6.3 倍でした。

### 入出力込み

現在の検証ドライバーと元の最速 AC 提出全体を、それぞれ `g++ -std=c++23 -O2` でコンパイルします。assert は有効です。入力ファイルを stdin に渡し、新しいプロセスを 9 回ずつ交互に実行した中央値です。プロセス起動・入出力を含む壁時計時間、単位は ms です。

| ケース | 採用版 | 最速 AC 404900 |
| --- | ---: | ---: |
| `max_random_00` | 1.238 | 1.538 |
| `max_random_01` | 2.498 | 3.017 |
| `random_01` | 1.319 | 1.462 |
| `sparse_00` | 0.887 | 0.957 |
| `sparse_03` | 0.929 | 1.005 |

これは GitHub Actions の同じジョブ内での比較です。Library Checker 本体での提出時間や順位を計測したものではありません。プロセス起動の影響もあるため、コア部分と区別してください。

採用版は公式 12 ケースを GCC・Clang の両方で通過し、ASan/UBSan と 4096 頂点の追加テストも通過しています。マッチングの正しさは乱択に依存しません。
