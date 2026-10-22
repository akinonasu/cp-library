# Top K Sum

2026-10-22 JST。`data_structure/top_k_sum.hpp` は、選ぶ側と残す側を2本の `std::multiset` で保持します。境界の移動と1個の置換には C++17 のノードハンドルを使い、既存ノードを再利用します。不存在の削除・置換を検出でき、格納するノード数は現在の要素数です。

## 参照と比較

- [ABC306 E 公式解説](https://atcoder.jp/contests/abc306/editorial/6607)：大きい方から K 個を2本の多重集合で管理し、和を保持する手法と実装例。
- [ABC281 E 公式解説](https://atcoder.jp/contests/abc281/editorial/5368)：小さい方から K 個を管理し、スライド区間を更新する手法。
- [ei1333/library の PrioritySumStructure](https://github.com/ei1333/library/blob/088b4c8b49f85194099305d998e7113dc3d5dd2e/structure/others/priority-sum-structure.hpp)：4本の優先度付きキューを使う遅延削除。比較には `MaximumSum<long long>` を使いました。原本の SHA-256 は測定原票に保存しています。

確認した Library Checker の問題一覧に、動的な Top K Sum を直接扱う専用問題はありません。対応する AtCoder の提出用ドライバを追加し、公式サンプルをローカルで確認しています。judge への提出や fastest の取得を示す測定ではありません。

## 正答検証

- 独立したソートによる愚直計算と、大小両方向・状態を持つ比較関数の4通りを各30,000操作比較。追加、重複の1個削除、不存在の削除・置換、k の変更、一括構築、コピー・移動・交換を含みます。
- 空集合、k=0、k が要素数より大きい場合、符号付き・符号なし、負数、128 bit の和、浮動小数点を確認。
- GCC 14.2・Clang 19 の単独 include と単体テスト、両コンパイラの ASan/UBSan が成功。
- ABC306 E は公式1例と乱択100例、ABC281 E は公式2例と乱択100例。両コンパイラ・通常 include と `oj-bundle` の4構成で合計812ケースが成功。乱択入力の正解は各更新・各窓をソートして生成しました。オンラインでの AC 判定ではありません。

## 速度

Linux x86_64、AMD EPYC 9V74、GCC 14.2、`-std=c++23 -O2 -march=haswell -Wall -Wextra -Werror`、CPU 2 固定。各条件5回の中央値、単位は ms。入力生成は計時外、初期値の逐次挿入と200,000回の置換・和の取得を計時します。各試行は新しいプロセスで、実行順を乱択しました。全60試行で各条件の checksum が一致しています。

| 条件 | 今回の実装 | 単純な2本の multiset | 遅延削除ヒープ |
| --- | ---: | ---: | ---: |
| 200,000要素・ランダム位置の点更新 | 145.560 | 163.044 | 42.426 |
| 200,000要素・循環順の窓更新 | 133.539 | 154.907 | 39.669 |
| 200,000要素・値が0〜15の更新 | 91.146 | 120.407 | 41.214 |
| 4,096要素・単調増加値への点更新 | 31.888 | 43.881 | 32.202 |

k は要素数の半分、通常の値は0以上10^9未満です。単調増加の条件だけ新しい値を `10^9+i` とします。窓更新は出る位置を循環順に選ぶため、処理中の窓の長さは常に200,000です。

単純な多重集合実装との比較では、所要時間が約11〜27%減りました。遅延削除ヒープは、今回の大規模な3条件では今回の実装より約2.2〜3.4倍速くなりました。今回の実装を最速とは主張しません。任意の比較関数、正確な出現個数、不存在の更新検出、現在の要素数に比例する保持領域を備えた構成です。

一括構築のソートや、k を大きく変更する操作の速度は、この表には含めていません。[全サンプル・ソースハッシュ](results/top_k_sum.json)を参照してください。

```sh
curl -L https://raw.githubusercontent.com/ei1333/library/088b4c8b49f85194099305d998e7113dc3d5dd2e/structure/others/priority-sum-structure.hpp -o /tmp/priority-sum-structure.hpp
taskset -c 2 python3 test/benchmark/top_k_sum.py --reference /tmp/priority-sum-structure.hpp --output /tmp/top-k-sum.json
```
