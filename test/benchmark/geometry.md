# 幾何ライブラリ

## 調査

2026-10-21 に Library Checker の live API を `status=AC&order=+time` で確認した。
時間は入出力込みの提出時間であり、今回のライブラリのローカル測定とは比較しない。
全言語の先頭が Rust の問題では、C++ の先頭も別に調べた。

| 問題 | 参照した C++ AC | 当日の時間 | 主な手法 |
| --- | --- | ---: | --- |
| Static Convex Hull | [#262822 / sortA0329](https://judge.yosupo.jp/submission/262822) | 39 ms | 32 bit 座標を64 bitキーに詰めて整数ソートし、重複を除いて monotone chain |
| Closest Pair of Points | [#410745](https://judge.yosupo.jp/submission/410745) | 23 ms | ランダムな点対から距離の上界を得て、整数 radix sort と隣接する帯の走査 |
| Closest Pair of Points | [#410741](https://judge.yosupo.jp/submission/410741) | 55 ms | ランダムな距離上界から格子を作り、セルを整数ソートして近傍セルを列挙 |
| Furthest Pair of Points | [#262824 / sortA0329](https://judge.yosupo.jp/submission/262824) | 36 ms | 凸包と回転キャリパー |
| Sort Points by Argument | [#262820 / sortA0329](https://judge.yosupo.jp/submission/262820) | 17 ms | 整数の象限・比をソートキーに変換 |
| Minimum Enclosing Circle | [#365901](https://judge.yosupo.jp/submission/365901) | 5 ms | 点順を変えて処理する Welzl 系の包含円更新 |

凸包の全言語先頭は [Rust #390244](https://judge.yosupo.jp/submission/390244)、34 ms。
偏角ソートの全言語先頭は [Rust #385808](https://judge.yosupo.jp/submission/385808)、16 ms。
提出の順位・時間・AC 状態は変わり得る。

最速提出の32 bit 座標パッキングや座標幅に依存する格子は、64 bit 整数・実数を同じ API で扱う今回の実装には導入していない。
凸包は monotone chain、最近点対は決定的な分割統治、最遠点対は凸包と回転キャリパーを使う。
整数の外積・距離二乗は128 bitへ拡張し、ソートには EPS を使わない。
乱択の最近点対は調査したが、今回は座標幅・実数・最悪計算量の前提が明確な分割統治を採用した。

凸包・最近点対・最遠点対は、公式の [凸包](https://github.com/yosupo06/library-checker-problems/blob/master/geo/static_convex_hull/sol/correct.cpp)・[最近点対](https://github.com/yosupo06/library-checker-problems/blob/master/geo/closest_pair/sol/correct.cpp)・[最遠点対](https://github.com/yosupo06/library-checker-problems/blob/master/geo/furthest_pair/sol/correct.cpp) の解法とも比較した。
円は CC0 の KACTL [共通接線](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/CircleTangents.h)・[円の交点](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/CircleIntersection.h)・[円と多角形の面積](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/CirclePolygonIntersection.h)・[最小包含円](https://github.com/kth-competitive-programming/kactl/blob/main/content/geometry/MinimumEnclosingCircle.h) を参照した。
三角形と扇形への分割、接線の法線、乱択増分法を使い、点円・一致円・空入力の扱いと、半径比の大きい円の計算を補った。

Library Checker の [Minimum Enclosing Circle](https://judge.yosupo.jp/problem/minimum_enclosing_circle) は半径ではなく、各入力点が境界にあるかを求める。
[公式解答](https://github.com/yosupo06/library-checker-problems/blob/master/geo/minimum_enclosing_circle/sol/correct.cpp) は直径円・外接円への包含を整数で判定する。
実数の中心・半径を返す API から任意の EPS で境界の真偽を決める検証には置き換えず、最小包含円はユニットテストで全候補の円と比較する。

参照ソースは実行していない。API の metadata と参照ソースを再取得するには以下を使う。

```sh
python3 test/benchmark/geometry_reference.py \
  --cache .verify-helper/cache/geometry/references \
  --output test/benchmark/results/geometry/references.json
```

## 検証対象

[公式問題リポジトリ](https://github.com/yosupo06/library-checker-problems/tree/master/geo) の凸包・最近点対・最遠点対・偏角ソートを judge driver に追加した。
偏角の区間はライブラリでは `[0,2pi)`、Library Checker は `(-pi,pi]` なので、driver で負の y 座標を持つ区間を先頭へ回転する。
原点は角度0として扱う。

実数の基本操作は、AOJ の [射影](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_1_A)、[線分間距離](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_2_D)、[多角形の包含](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_3_C)、[円と直線の交点](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_D) の driver を追加した。
さらに [凸切断](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_4_C) と [円と多角形の共通面積](https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_7_H) でも検証する。
接点を2回出力する AOJ の仕様は、1個の接点を返すライブラリの結果から driver で変換する。

2026-10-21 のローカル検証は GCC 15.2.0・Clang 21.1.8、Linux x86_64/WSL Ubuntu で実行した。
6ヘッダーの単独コンパイル、5ユニットを両コンパイラで確認し、同じ5ユニットを Clang の AddressSanitizer・UndefinedBehaviorSanitizer でも確認した。
10 judge driver は取得した253入力を両コンパイラで通過し、合計506ケース成功。Library Checker は公式 checker、AOJ は誤差付き数値比較を使い、リモート提出はしていない。
CI の対象選択・依存追跡は `ci_test.py` 46件・`affected_test.py` 28件を確認した。
新規6ページの説明・依存展開を生成し、6個の Bundle を GCC で、使用例6個を GCC・Clang でコンパイルした。
一時コピーで `Real` を `double` にした5ユニットも通過した。これは精度差の確認であり、macOS 上での実行結果ではない。
問題別のケース数は [validation.json](results/geometry/validation.json) に保存した。

ユニットでは交差判定の対称性、最近・最遠点対と小入力の総当たり、Minkowski 和と全点対和の凸包、最小包含円と全支持候補、円の共通面積と解析値を比較する。
重複・空集合・同一直線・点線分・点円・一致円・接する外側の多角形辺・小さい凸多角形・大きな整数座標で面積が小さい三角形も含めた。

## ローカル測定

`geometry.cpp` と `geometry.py` は整数の凸包・最近点対・最遠点対を測る。
入力生成・シャッフルは計測外で、ライブラリ呼び出し、内部確保、戻り値の走査・チェック、戻り値の破棄を含む。
入力は random・collinear・grid・duplicates・parabola の5種類。seed は `731 + trial`。
parabola は全頂点が凸包へ残る入力、duplicates は重複のある入力も含める目的で使う。

```sh
python3 test/benchmark/geometry.py --output test/benchmark/results/geometry/generic
python3 test/benchmark/geometry.py --native --output test/benchmark/results/geometry/native
```

既定では `n = 200000`、5標本、CPU 0 固定。
共通フラグは `-std=c++23 -O2 -Wall -Wextra -Werror`、native は `-march=native -ftrivial-auto-var-init=zero` を追加する。
コンパイラ・CPU・入力条件・ソースの SHA-256 は `environment.json`、全標本は `samples.json`、中央値と最小・最大値は `summary.json` に保存する。
現在の実装を最速提出と同じ環境で測定していないため、最速・同速とは主張しない。

2026-10-21、Core i7-13700F、WSL Ubuntu、GCC 15.2.0、CPU 0 固定で実行した。
以下は20万点・5 seed の中央値（ms）。同時に他のテスト・コンパイルは実行していない。

| 操作 | 入力 | generic | native |
| --- | --- | ---: | ---: |
| 凸包 | random | 18.938 | 19.480 |
| 凸包 | collinear | 15.731 | 15.207 |
| 凸包 | grid | 19.797 | 20.057 |
| 凸包 | duplicates | 10.307 | 10.424 |
| 凸包 | parabola | 16.261 | 16.644 |
| 最近点対 | random | 28.786 | 30.648 |
| 最近点対 | collinear | 19.397 | 21.334 |
| 最近点対 | grid | 23.568 | 25.318 |
| 最近点対 | duplicates | 9.476 | 11.293 |
| 最近点対 | parabola | 20.402 | 22.840 |
| 最遠点対 | random | 19.013 | 19.825 |
| 最遠点対 | collinear | 14.793 | 15.290 |
| 最遠点対 | grid | 19.811 | 20.463 |
| 最遠点対 | duplicates | 10.217 | 10.277 |
| 最遠点対 | parabola | 19.382 | 19.976 |

この環境・入力では native フラグによる一貫した改善は見られず、ヘッダーへの target pragma は追加していない。
測定値は [generic](results/geometry/generic/summary.json)・[native](results/geometry/native/summary.json)、条件とソースハッシュは同じディレクトリの `environment.json` に保存した。
