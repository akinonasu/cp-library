# Dynamic Li Chao Tree

`data_structure/dynamic_li_chao_tree.hpp` を測定した。クエリ座標を先読みせず、指定した整数座標範囲で直線・線分の追加と点クエリを処理する。最小値・最大値の両方を扱う。

## 参考提出・資料

2026-10-17 に、対応する二つの Library Checker 問題の AC を実行時間順に取得した。

| 問題 | 提出 | 取得時のジャッジ時間 |
| --- | --- | ---: |
| Line Add Get Min | [362191](https://judge.yosupo.jp/submission/362191) | 0.057 秒 |
| Line Add Get Min | [357336](https://judge.yosupo.jp/submission/357336) | 0.058 秒 |
| Line Add Get Min | [357321](https://judge.yosupo.jp/submission/357321) | 0.058 秒 |
| Segment Add Get Min | [357121](https://judge.yosupo.jp/submission/357121) | 0.062 秒 |
| Segment Add Get Min | [361262](https://judge.yosupo.jp/submission/361262) | 0.063 秒 |
| Segment Add Get Min | [361261](https://judge.yosupo.jp/submission/361261) | 0.063 秒 |

取得した上位提出は全て座標を先読みする Li Chao Tree で、配列配置、葉の値の分離、radix sort による座標圧縮を利用している。線分版は区間を分解し、反復で直線を挿入する。今回のオンライン API では座標の先読み・圧縮を使わず、配列配置と不要な挿入の打ち切りを参考にした。取得結果とソースのハッシュは `results/dynamic_li_chao_tree_reference.json` に保存した。これらの提出とのローカル時間比較はしていない。

[Luzhiled: Dynamic Li Chao Tree](https://ei1333.github.io/library/structure/convex-hull-trick/dynamic-li-chao-tree.hpp.html) の動的ノードと端点比較による手法も確認した。比較用には、個別に確保するポインタのノード・端点比較・再帰的な挿入とクエリを持つ実装を用意した。中点には安全な `std::midpoint` を使い、空の結果は `optional` で表す。

```sh
python3 test/benchmark/dynamic_li_chao_tree_reference.py --cache .verify-helper/cache/dynamic-li-chao-references --output test/benchmark/results/dynamic_li_chao_tree_reference.json
```

## 実装

ノードを連続した `vector` に格納し、子を 32 bit の添字で保持する。左の子の添字の上位 bit を直線の有無に使い、線分追加用の中間ノードと直線を持つノードを区別する。直線を持つノードは中点で最適な直線を残し、残りが改善し得る側を傾きから選ぶ。直線追加・クエリは反復で処理する。線分追加の再帰の深さは座標型の bit 数以下。

既定の評価型は 128 bit。直線・線分の有効範囲の両端を調べ、64 bit に収まる間は軽い比較を使う。中間の積が 64 bit を超えても、128 bit で評価してから狭めるため、和が収まる場合の符号付きオーバーフローを避けられる。全ての積と和が 64 bit に収まることが分かる場合は第 3 テンプレート引数に `int64_t` を指定できる。

座標範囲は半開区間。端点型を座標型より広くし、INT64_MIN から INT64_MAX の全範囲と、最大値一点への線分追加を扱う。線分の範囲外は切り詰め、値のない場所は `nullopt` を返す。

## 測定条件

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。各入力は追加 200000 回とクエリ 200000 回を交互に行う。seed 731、各条件 7 回、実装の実行順は毎回シャッフルした。入力生成とオブジェクト破棄は時間に含めず、構築・`reserve(200000)`・追加・クエリ・共通のチェックサム処理を測定した。比較側の `reserve` は何もしない。測定中は他のテストを実行していない。

| 入力 | 内容 |
| --- | --- |
| random_line | 座標・傾きが ±10^9、切片が ±10^18 のランダムな直線 |
| hull | 直線 -2ix+i^2 を順に追加。全ての直線がある座標で最小になる |
| same_slope | 傾き 7 を固定した直線 |
| segments | ランダムな座標区間に直線を追加 |
| points | 一点だけ有効な線分を追加。クエリの半分は追加した点 |
| dominated | 最初に定数 -8×10^18 を追加し、それより大きい線分を追加 |
| wide | 符号付き 64 bit 全域の座標・傾き・切片による直線 |

`baseline` は `wide` 以外で 64 bit、`wide` で 128 bit の比較を使う。`library` は既定型、`library64` は第 3 引数に `int64_t` を指定したもの。後者は値が収まる入力だけを対象にする。全実装・全試行でクエリ結果のチェックサムが一致した。

generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native はさらに `-march=native -ftrivial-auto-var-init=zero` を指定した。以下は中央値、単位はミリ秒。ノード数は当該入力で使用した数。AtCoder 実機での速度は未測定。

### generic

| 入力 | 実装 | 時間 ms | ノード数 |
| --- | --- | ---: | ---: |
| random_line | baseline | 4.901 | 30 |
| random_line | library | 5.652 | 30 |
| random_line | library64 | 5.174 | 30 |
| hull | baseline | 27.353 | 200000 |
| hull | library | 24.313 | 200000 |
| hull | library64 | 24.150 | 200000 |
| same_slope | baseline | 1.417 | 1 |
| same_slope | library | 1.846 | 1 |
| same_slope | library64 | 1.483 | 1 |
| segments | baseline | 14.471 | 6356 |
| segments | library | 16.556 | 6356 |
| segments | library64 | 15.165 | 6356 |
| points | baseline | 168.844 | 2829419 |
| points | library | 149.660 | 2829419 |
| points | library64 | 140.193 | 2829419 |
| dominated | baseline | 1.363 | 1 |
| dominated | library | 2.213 | 1 |
| dominated | library64 | 1.779 | 1 |
| wide | baseline | 5.858 | 31 |
| wide | library | 5.967 | 31 |

### native

| 入力 | 実装 | 時間 ms | ノード数 |
| --- | --- | ---: | ---: |
| random_line | baseline | 4.899 | 30 |
| random_line | library | 5.573 | 30 |
| random_line | library64 | 5.248 | 30 |
| hull | baseline | 31.368 | 200000 |
| hull | library | 24.498 | 200000 |
| hull | library64 | 24.855 | 200000 |
| same_slope | baseline | 1.461 | 1 |
| same_slope | library | 1.731 | 1 |
| same_slope | library64 | 1.417 | 1 |
| segments | baseline | 15.029 | 6356 |
| segments | library | 17.583 | 6356 |
| segments | library64 | 15.403 | 6356 |
| points | baseline | 181.739 | 2829419 |
| points | library | 154.561 | 2829419 |
| points | library64 | 151.220 | 2829419 |
| dominated | baseline | 1.400 | 1 |
| dominated | library | 2.226 | 1 |
| dominated | library64 | 1.778 | 1 |
| wide | baseline | 6.056 | 31 |
| wide | library | 6.405 | 31 |

ノード数の多い `hull`・`points` では配列プールの実装が短縮した。一方、少数のノードで打ち切れる入力では、既定型の評価範囲チェックやノード状態の分岐の費用があり、比較側が速い場合があった。全条件での高速化や Library Checker fastest との優劣は主張しない。生の試行値、CPU・コンパイラ・フラグ・入力条件・ソースのハッシュは `results/dynamic_li_chao_tree_{generic,native}/` に保存した。

```sh
python3 test/benchmark/dynamic_li_chao_tree.py --output test/benchmark/results/dynamic_li_chao_tree_generic
python3 test/benchmark/dynamic_li_chao_tree.py --native --output test/benchmark/results/dynamic_li_chao_tree_native
```

## 検証

全探索との比較で最小・最大の両方を検証。8 / 16 / 32 / 64 bit の座標、等傾き、空区間、範囲外の切り詰め、線分の右端、値のない座標、最小・最大の 64 bit 座標と係数、一点の領域、64 bit から 128 bit 比較への切り替え、中間積の桁あふれと加算の相殺、再構築・clear・コピー・ムーブ、配列の再確保、30000 本の全てが最小値を取る入力を確認した。

GCC 15 / Clang 21 の単体・単独 include・共存検証、Clang の ASan / UBSan、GCC 13、ARM64 / QEMU を通過。公式ケースは Line Add Get Min が 15 個、Segment Add Get Min が 13 個で、GCC・Clang の両方で AC。公式線分テストのローカル実行では最大約 310 MB を使用した（検証ツールの表示値）。これは上の 200000 回の追加ベンチマークとは別の条件。全 API 表と公開用の結合ヘッダーも検証した。ログと検証対象のハッシュは `results/dynamic_li_chao_tree_*` に保存した。
