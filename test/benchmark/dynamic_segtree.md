# Dynamic Segtree の比較

## 参考にした実装

- [ABC403 G / 65376624](https://atcoder.jp/contests/abc403/submissions/65376624): AC 87 ms。整列済みの小配列をブロックとして保持し、固定セグメント木で集約する実装。小配列による連続アクセスを参考にしました。
- [point_add_range_sum / 403014](https://judge.yosupo.jp/submission/403014): 調査時の fastest、23 ms。16 分木と AVX2 による接頭和。座標が既知の加法向けです。
- [point_set_range_composite / 402223](https://judge.yosupo.jp/submission/402223): 調査時の fastest、49 ms。逆元を用いた多分木。任意のモノイドや、直前の答えから座標が決まる問題にはそのまま適用できません。

## Linux/GCC での比較

2026-10-16、GitHub Actions `ubuntu-24.04`、AMD EPYC 7763、GCC 13.3、C++23、`-O2 -DNDEBUG`。同一入力を各候補に与え、順序を入れ替えて各5回測定した中央値です。assert 有効でも比較し、全候補のチェックサム一致を確認しています。

[最初の比較](https://github.com/qandori/cp-library/actions/runs/38860497751)、[多分木の比較](https://github.com/qandori/cp-library/actions/runs/38860920145)、[葉のバケット化の比較](https://github.com/qandori/cp-library/actions/runs/38872310524)。

`dynamic_segtree_base.hpp` は変更前の保存版です。`dynamic_segtree_variants.py` が細部を変更した候補を生成し、`dynamic_segtree_variants.cpp` で比較します。

```sh
python3 test/benchmark/dynamic_segtree_variants.py /tmp/dynamic-segtree-variants
g++ -std=c++23 -O2 -DNDEBUG -I . -I /tmp/dynamic-segtree-variants -I test/benchmark test/benchmark/dynamic_segtree_variants.cpp -o /tmp/dynamic-segtree-bench
/tmp/dynamic-segtree-bench 100000 100000 5 new > /tmp/dynamic-segtree.csv
```

62 bit の座標10万点、更新・区間積・一点取得を各10万回。和は64 bit整数、交代和は64 bit整数と長さの偶奇、一次関数合成は mod 998244353 です。以下は一様乱数の座標です。

| モノイド | 実装 | 構築 ms | 更新 ms | 区間積 ms | 一点取得 ms | 確保領域 MB |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| 和 | 従来 | 18.83 | 24.87 | 34.50 | 18.35 | 3.20 |
| 和 | 葉8要素 | 18.44 | 22.22 | 26.25 | 9.96 | 3.15 |
| 和 | 葉16要素 | 17.69 | 20.84 | 23.86 | 9.38 | 2.78 |
| 交代和 | 従来 | 23.88 | 29.89 | 39.60 | 20.38 | 4.80 |
| 交代和 | 葉8要素 | 21.18 | 25.13 | 28.83 | 10.49 | 4.65 |
| 交代和 | 葉16要素 | 20.75 | 24.80 | 26.37 | 10.18 | 4.13 |
| 一次関数合成 | 従来 | 35.23 | 42.24 | 50.94 | 18.73 | 3.20 |
| 一次関数合成 | 葉8要素 | 30.97 | 34.80 | 35.89 | 11.10 | 3.15 |
| 一次関数合成 | 葉16要素 | 33.49 | 36.31 | 35.71 | 10.50 | 2.78 |

昇順の一次関数合成では構築が従来10.91 ms、葉8要素11.76 ms、葉16要素13.65 msでした。更新・クエリと構築のバランスから8要素を採用しました。確保領域には予約容量を含み、入力・クエリ配列を含みません。座標が偏って葉の充填率が下がる場合、メモリ削減は保証されません。

### ABC403 G を模したオンライン処理

30万回の入力を乱数で作り、直前の答えから座標を復号して挿入し、奇数番目の要素の和を取得します。全体の和と交代和から答えを復元します。次の表は入出力を除く処理時間です。

| 実装 | 時間 ms | 確保領域 MB |
| --- | ---: | ---: |
| 従来 | 85.95 | 14.40 |
| 多分木・葉16要素・分岐4 | 98.78 | 22.50 |
| 多分木・葉32要素・分岐8 | 84.87 | 17.05 |
| 二分木・葉4要素 | 82.35 | 17.10 |
| 二分木・葉8要素 | 75.60 | 13.95 |
| 二分木・葉16要素 | 76.27 | 12.38 |

採用版は内部ノードに分岐と積だけを保持し、葉に最大8要素を整列して格納します。葉の積は順序を保った二分集約とし、重い演算の依存を短くします。分岐の高さは境界座標の末尾の0から復元します。

再帰更新、集約時の空の子の省略、ノード内のメンバー順変更、区間がノード全体を覆う場合の短絡も比較しましたが、再測定で安定した改善が出ませんでした。多分木はランダム挿入・一次関数合成・メモリで不利なため採用していません。

交代和と偶奇を64 bitにまとめる問題専用のモノイドも比較しました。従来84.40 ms・9.60 MB、葉8要素74.87 ms・9.45 MBで、主な効果はメモリ削減でした。汎用ライブラリのAPIには追加していません。

AtCoder の提出時間とは実行環境と入力が異なるため、この表は従来のライブラリとの比較です。

### 別実行ファイルでの比較

`dynamic_segtree_abc403g.cpp` は問題形式の入出力を含む実行コードです。`dynamic_segtree_process.py` は候補ごとに別々の実行ファイルを作り、公式サンプルと全出力の SHA-256 一致を確認して測定します。

```sh
python3 test/benchmark/dynamic_segtree_process.py /tmp/dynamic-segtree-variants /tmp/dynamic-segtree-results 5
```

入力は Python `random.Random(8652963)` による30万個の値です。時間にはプロセス起動と入出力を含み、メモリは GNU time による最大 RSS です。

[最終確認の実行](https://github.com/qandori/cp-library/actions/runs/38873228903)、測定スナップショット `8a25f91f7eefd203d327339ebf323c1318f2e610`。この一時ブランチは測定後に削除しています。採用版のヘッダー・単体テスト・公式ケース用コードと手元の内容の一致を確認しました。

| 実装 | 時間 ms | 最大 RSS KiB |
| --- | ---: | ---: |
| 従来 | 104.40 | 20,252 |
| 採用版 | 96.11 | 19,640 |
| 葉8要素・逐次集約 | 96.21 | 19,648 |
| 従来・64 bitモノイド | 103.26 | 15,532 |
| 採用版・64 bitモノイド | 97.25 | 15,336 |

入出力込みで約8%短縮しました。GCC・Clang の単体テスト、ASan・UBSan、Library Checker の `point_add_range_sum`・`point_set_range_composite` の公式ケースも成功しました。ABC403 G は公式サンプルと直接計算との比較を行い、AtCoder への提出は行っていません。

依存先の HLD 検証3問題（`vertex_set_path_composite`・`vertex_add_path_sum`・`vertex_add_subtree_sum`）も macOS arm64 の GCC・Clang で公式ケースを通しました。チェッカーが空白入りパスを扱えないため、ソースが一致する一時ディレクトリで実行しています。`standalone.cpp`・`rolling_hash_monoid.cpp` も GCC・Clang と ASan・UBSan で確認しました。
