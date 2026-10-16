# Hungarian Algorithm のベンチマーク

2026-10-16 に [Assignment Problem](https://judge.yosupo.jp/problem/assignment) の実行時間順の AC 提出を調査しました。

- [410724](https://judge.yosupo.jp/submission/410724): 列最小値による初期割当、割当行の余裕の移動、回数を制限した行簡約、最短増加路、Monge・逆 Monge の判定を確認しました。
- [410633](https://judge.yosupo.jp/submission/410633): 長方形行列への対応、同距離での空き列の優先、双対変数の一括更新を確認しました。
- [384069](https://judge.yosupo.jp/submission/384069): 正方行列と長方形行列の処理の分離を確認しました。

参照提出 410724 は `hungarian_reference.py` が一時取得します。BSD の著作権表示を保持し、入出力と全体に作用する `O3` 指定を取り除き、入力を複製せずに渡すため `vector<int>` の参照をポインターに変更します。アルゴリズム部分は同じです。元ソースの SHA-256 は `29be0049ade38e1236ff572ae5e0e614132d9d4317e4ad300d169cc05dc78fd6` です。

## 再現方法

GitHub Actions の `hungarian benchmark` は手動実行です。`verify` を無効にすると、変更のない本体の公式・単体・sanitizer テストを省き、比較計測だけを行います。

```sh
python3 test/benchmark/hungarian_variants.py /tmp/hungarian-variants
python3 test/benchmark/hungarian_reference.py /tmp/hungarian-variants
g++ -std=c++23 -O2 -I . -I /tmp/hungarian-variants test/benchmark/hungarian.cpp -o /tmp/hungarian-bench
/tmp/hungarian-bench 512 512 5 > /tmp/hungarian.csv
```

引数は行数・列数・試行数です。`-DNDEBUG` を付ける測定も行います。同じ行列を使い、各試行で実装の実行順をシャッフルします。入力生成・入出力・正解照合を測定から除き、結果の構築・作業配列の確保を含めます。全実装の費用が一致することと、各割当の有効性・費用の和を測定後に確認します。

| 行列 | 内容 |
| --- | --- |
| `random` | $$[0,10^9]$$ の乱数 |
| `signed` | 上記から $$5\cdot10^8$$ を引いた行列 |
| `ties` | $$[0,7]$$ の乱数 |
| `constant` | 全要素が $$-17$$ |
| `monge` | $$(i-j)^2$$ |
| `anti_monge` | $$-(i-j)^2$$ |
| `correlated` | 行ごとの定数＋列ごとの定数＋小さい乱数 |
| `blocks` | 同じ 32 要素ブロック内は 0 または 1、その他は 1000 |

## 比較した工夫

| 名前 | 内容 |
| --- | --- |
| `classic` | 各探索段階で双対変数と全距離を更新する通常の実装 |
| `dense` | 探索後に双対変数をまとめて更新し、同距離の列をキューで処理 |
| `greedy` | 行最小値で空き列に初期割当 |
| `column` | 正方行列で列最小値の初期割当と、唯一の割当行の余裕の移動 |
| `no_transfer` | 余裕の移動を省略し、行簡約を 2 巡する |
| `reduced` | `column` に 2 巡の行簡約を追加 |
| `baseline` | `reduced` に Monge・逆 Monge の専用処理を追加した初期版 |
| `ascending` | 初期割当の列順を逆転 |
| `queue` | 行簡約で追い出された行を再処理し、全体を最大 $$2n$$ 回に制限 |
| `queue_half` | 上記を最大 $$n$$ 回に制限 |
| `narrow` | 列最小値の走査を入力と同じ型で行い、双対変数へ最後に変換 |
| `narrow_order` | 上記に加え、32 bit 入力の Monge 判定を 64 bit で行う |
| `free_only` | 同じ最小費用で割当候補を変更するのは、空き列が見つかった場合だけ |
| `rect_scan` | 長方形行列の行最小値と、空き列の検索を別ループにする |
| `current` | 現在の `graph/hungarian.hpp` |

初期版は `hungarian_baseline.hpp` に保存しています。費用の計算型は `long long`、入力は `int` です。作業領域は $$O(n+m)$$ で、入力行列は複製しません。

## Linux/GCC での実測

[初回比較](https://github.com/qandori/cp-library/actions/runs/38848344617)、[追加比較](https://github.com/qandori/cp-library/actions/runs/38848823291)、[採用版の検証・計測](https://github.com/qandori/cp-library/actions/runs/38849111181) を行いました。環境・全試行の CSV・参照コードのハッシュ・公式テストログは artifact `hungarian-linux-gcc-benchmark` にあります。

以下は採用版の計測です。Ubuntu 24.04 / AMD EPYC 7763 / GCC 13.3.0、`-std=c++23 -O2`、5 試行の中央値、単位は ms です。各表の値を同じジョブ内で比較しています。採用版のソースは `9512b86827dc78b6c2adbbc9bda18aa67c13dd41` の `graph/hungarian.hpp` です。

### 512×512、assert 有効

| 実装 | random | ties | constant | monge | anti_monge | correlated | blocks |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `classic` | 10.956 | 146.308 | 137.104 | 0.567 | 75.883 | 171.577 | 8.450 |
| `baseline` | 1.858 | 0.728 | 0.455 | 0.472 | 0.412 | 40.841 | 0.557 |
| **採用版** | 1.803 | 0.716 | 0.244 | 0.239 | 0.209 | 39.163 | 0.551 |
| `reference_410724` | 1.781 | 0.495 | 0.246 | 0.296 | 0.252 | 42.821 | 0.450 |

### 1024×1024、NDEBUG

| 実装 | random | ties | constant | monge | anti_monge | correlated | blocks |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `classic` | 66.050 | 962.969 | 920.868 | 2.305 | 592.671 | 1238.510 | 32.931 |
| `baseline` | 11.233 | 2.821 | 1.789 | 1.884 | 1.619 | 281.460 | 1.995 |
| **採用版** | 11.213 | 2.912 | 0.987 | 0.953 | 0.843 | 281.462 | 2.132 |
| `reference_410724` | 10.111 | 1.939 | 0.991 | 0.993 | 1.004 | 283.129 | 1.904 |

### 256×2048、NDEBUG

| 実装 | random | ties | constant | monge | anti_monge | correlated | blocks |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `classic` | 1.551 | 134.866 | 126.640 | 1.186 | 127.738 | 166.988 | 16.271 |
| `baseline` | 0.812 | 0.971 | 1.306 | 0.478 | 60.909 | 85.428 | 0.614 |
| **採用版** | 0.569 | 0.836 | 0.515 | 0.252 | 60.622 | 84.701 | 0.420 |
| `rect_scan` | 0.710 | 0.380 | 0.357 | 0.343 | 60.866 | 84.484 | 0.405 |


採用した変更は、入力型での列最小値走査、入力幅に応じた Monge 判定、同額の候補で空き列だけを優先する処理です。1024×1024 の random は通常実装の約 5.9 倍、Monge は初期版の約 2.0 倍でした。256×2048 の constant は初期版の約 2.5 倍です。

行簡約のキュー化は random を改善する場合がある一方、ties・blocks で悪化しました。長方形の最小値走査を 2 ループへ分ける案は constant・ties で速く、random では採用版が速い結果でした。全体のバランスを優先して採用していません。列順の変更にも安定した改善はありませんでした。

参照提出より速い条件も遅い条件もあります。参照提出は正方・32 bit 入力・最小化に特化しており、採用版は長方形・最大化・広い整数型も扱います。この測定から judge 上の最速や全入力での優位性は主張していません。

公式 Assignment Problem の 14 ケースを GCC・Clang の両方で通過しました。単体テストは部分集合 DP との照合、負の費用、同値が多い入力、空行列、長方形、Monge・逆 Monge、平坦な配列の stride、最小化・最大化、異なる計算型を確認し、ASan/UBSan も通過しています。成功済みで変更のない本体のテストは、追加比較では省略しています。
