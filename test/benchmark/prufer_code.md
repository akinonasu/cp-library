# Prufer Code

2026-10-21。`tree/prufer_code.hpp` に、無向木の隣接リストと Prufer 符号の相互変換を追加しました。

## 手法

[Library Checker の公式問題一覧](https://github.com/yosupo06/library-checker-problems/blob/master/categories.toml)には、Prufer 符号化・復号に対応する問題がないため、該当する fastest 提出との比較は行っていません。

[cp-algorithms の Prufer Code](https://cp-algorithms.com/graph/pruefer_code.html)で説明されている、最小の葉をヒープで管理する方法と、探索位置を単調に進める線形時間の方法を比較しました。本実装は後者です。符号化では DFS で親を求める代わりに、残っている隣接頂点の XOR を更新します。復号では次数から各隣接リストの容量を先に確保します。どちらも再帰を使いません。

比較用のヒープ版も XOR による隣接頂点の取得と同じ容量予約を使い、葉を選ぶ方法の差を比較しています。単体テストでは、これとは別に隣接リストを走査するヒープ版とも照合しています。

## 検証

- 0・1・2 頂点の境界ケース。
- 3〜7 頂点の全 Prufer 符号 18,247 個で、符号の往復と独立したヒープ版による復号結果・符号化結果を比較。
- ラベルと隣接リストの順序を入れ替えたランダムな木 2,000 個で、入力が変化しないことと、符号・辺集合の一致を確認。
- 20 万頂点のパス 2 種・中心の異なるスター 3 種。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、二重 include、テンプレートマクロの非混入、`NDEBUG` でのヘッダー・ドキュメント例のコンパイル。

空の符号は頂点数を一意に決めません。既定の復号は 2 頂点、0・1 頂点は頂点数を明示して復号します。符号化の入力には、両方向の辺が各 1 回ずつ記録された単純な無向木が必要です。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、`-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror`。CPU 2 に固定し、7 回の中央値を記録しました。各回の実装順は seed `20261021` で入れ替え、入力生成と checksum は測定外、作業領域・返り値の確保は測定内です。返り値の解放は測定外です。他のコンパイル・検証と同時に測定していません。

`random` は各符号要素を一様に選び、`path` は `[1,2,...,n-2]`、`star` は全符号要素を `n/2` としています。毎回 checksum の一致を確認します。単位は ms、順にヒープ版／本実装です。

| 頂点数・形 | 符号化 | 復号 |
| --- | ---: | ---: |
| 200,000・random | 12.300 / 3.978 | 17.710 / 6.881 |
| 200,000・path | 1.080 / 1.288 | 3.262 / 2.869 |
| 200,000・star | 9.385 / 0.801 | 11.465 / 2.815 |
| 1,000,000・random | 75.962 / 23.566 | 183.176 / 55.410 |
| 1,000,000・path | 6.659 / 6.342 | 17.098 / 15.455 |
| 1,000,000・star | 48.735 / 4.289 | 60.039 / 15.365 |

昇順パスではヒープの要素数が小さく、20 万頂点の符号化は比較用ヒープ版の方が速い結果でした。AtCoder・Library Checker 上の実測値ではありません。

測定値・入力条件・ソースのハッシュは `results/prufer_code/` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/prufer_code_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/prufer_code_selection.json
g++ -std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror -I . test/benchmark/prufer_code.cpp -o /tmp/prufer-code-benchmark
flock /tmp/cp-fps-tuning.lock taskset -c 2 /tmp/prufer-code-benchmark
```
