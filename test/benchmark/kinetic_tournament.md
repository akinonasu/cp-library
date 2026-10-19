# Kinetic Tournament

2026-10-20。全体の絶対時刻を進める旧実装・説明・テスト・測定資料を削除し、[JSC2024 決勝 D の公式解説](https://atcoder.jp/contests/jsc2024-final/editorial/10766)に対応する区間更新版へ置き換えた。同じヘッダー名を使うが API は異なる。

## 実装

固定した昇順の係数列 `V` に対して、区間内の有効な値に `a*V[i]+b` を加える。`a >= 0`、`b` は符号付き。最大値版が既定で、最小値版も用意する。点での値の設定・無効化・再有効化に対応する。係数は変更しない。

`GenericSegtreeBeats` を共有し、集約値・勝者・部分木の証明が失効するまでの残りステップ数を保持する。更新が証明を破る場合だけ子へ降りる。浮動小数点や優先度付きキューは使わない。最大値の同値時は右、最小値は左を選び、交差時刻で将来有利になる側を選ぶ。

証明には最初の交差時刻でなく、安全に進められる最大の整数ステップ数を保存する。正の値の差 `gap` と係数の差 `d` に対して `(gap-1)/d` を使う。係数・値が符号付き 64 bit 全域でも正の差は `uint64_t` に収まるため、符号なし減算と 64 bit 除算で正確に求められる。「交差なし」の番兵 `UINT64_MAX` と有限の証明が衝突しない。値の評価と遅延更新の累積には符号付き 128 bit を使う。

係数の昇順と非負の更新係数により、全体更新だけでは勝者が将来不利な側へ戻らない。部分区間更新や点更新が切る節点を数えると、`q` 操作の合計は `O(n log n + q log² n)`。更新 1 回の最悪時間は `O(n)`。任意の符号の `a` を受け付けるものではない。

## 参考資料・提出

- [公式解説](https://atcoder.jp/contests/jsc2024-final/editorial/10766)の区間 add・重み付き加算・区間 max を基にした。[解説の回答例](https://atcoder.jp/contests/jsc2024-final/submissions/56754335)の HTML 取得は HTTP 403 となったため、回答例のソースは調査・速度比較に使っていない。
- 関連する Library Checker の [Range Linear Add Range Min](https://judge.yosupo.jp/problem/range_linear_add_range_min) の AC を公式 API の実行時間順で取得した。[393894](https://judge.yosupo.jp/submission/393894)（102 ms）、[382268](https://judge.yosupo.jp/submission/382268)（120 ms）は正負両方向の証明を持つ Kinetic Segment Tree。[371003](https://judge.yosupo.jp/submission/371003)（212 ms）は凸包情報を使う方式。最速の `KineticRangeLinearAddMinTree` 本体を抽出し、同じ非負の操作列で比較した。この問題の負の更新係数を含む全仕様に、新実装が対応するわけではない。
- 一様加算の検証に使う [Range Add Range Min](https://judge.yosupo.jp/problem/range_add_range_min) も調査した。[409052](https://judge.yosupo.jp/submission/409052)（50 ms）、[402318](https://judge.yosupo.jp/submission/402318)（51 ms）、[408981](https://judge.yosupo.jp/submission/408981)（55 ms）。幅の広い木や正規化した最小値の表現を使う。一様加算だけの問題であり、重み付き加算まで扱う今回のライブラリとの直接の速度比較は行わない。

取得日時・一覧・URL・ソースの SHA-256 は `results/kinetic_tournament/reference.json`。外部ソースは `.verify-helper/cache/kinetic_range_references` に保存し、リポジトリには含めない。最速性は主張しない。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。他の測定・テストとは共通のロックで直列実行。
- generic は `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。native はさらに `-march=native -ftrivial-auto-var-init=zero`。
- 5 試行の中央値。実装の順序は固定 seed でシャッフル。木の比較では構築・入力生成・入出力を除外し、操作と checksum を計測する。
- 本体の比較は長さ 100,000、係数 `V[i]=i`、100,000 反復。参考実装もこの固定係数を使う。返り値は最小値のみ比較し、同値時の添字は比較しない。全試行の checksum が一致。
- `random` は初期値を ±10¹² の乱数にし、部分区間の非負の重み付き加算・符号付き一様加算・区間最小値を交互に実行する。`partial` はさらに左半分への一様加算を挟む。
- `envelope` の初期値は `(n-1-i)*(n-i)/2`。全体に係数分を 1 回ずつ加算し、全体最小値を毎回取得する。`jump` は同じ初期値から一度だけ `n*V[i]` を加える。
- JSC の測定は長さ 500,000。`kinetic_tournament_jsc.cpp` の `solve` 全体を測り、係数列のソート・重複除去・木の構築も含む。入出力と入力生成は除外する。
- AtCoder・Library Checker の実機での速度、Mac の速度は未測定。

## 結果

単位は ms。

| 操作列 | generic 新実装 / 参考 | native 新実装 / 参考 |
| --- | ---: | ---: |
| random | 58.200 / 41.860 | 61.866 / 44.897 |
| partial | 64.859 / 43.833 | 68.329 / 46.207 |
| envelope | 24.689 / 9.758 | 24.251 / 11.154 |
| jump | 1.004 / 0.508 | 0.922 / 0.604 |

今回の全ケースで参考実装の方が速い。新実装は、任意の昇順係数・点設定・無効化・最大値・添字を扱い、64 bit 全域の差と広い累積更新に対応する。一方、参考実装は添字を係数として固定し、64 bit 演算と専用の再帰木を使う。汎用の Beats を使う新実装との差は残っている。

| JSC2024 D、500,000 要素 | generic | native |
| --- | ---: | ---: |
| 全て 10⁹ | 19.521 | 22.966 |
| 昇順 `1..n` | 554.908 | 559.033 |
| 降順 `n..1` | 171.658 | 169.985 |
| 小さい値と大きい値の交互列 | 544.743 | 558.629 |
| `1..10⁹` の乱数列 | 899.769 | 955.716 |

## 試行した改善

最初は全体への更新・取得も通常の区間操作で処理していた。`GenericSegtreeBeats::apply(f)` を追加し、全体の操作を根へ直接適用するようにした。大きさが 2 の冪でない場合も、末尾の無効な葉を含む根を利用できる。

| 版 | envelope generic / native | JSC 降順 generic / native |
| --- | ---: | ---: |
| 初期版 | 40.831 / 44.410 | 279.857 / 260.265 |
| 全体操作を根で処理 | 24.885 / 25.039 | 193.342 / 176.736 |
| 証明の除算も 64 bit 化 | 24.689 / 24.251 | 171.658 / 169.985 |

全体操作の経路の短縮が envelope に有効だった。除算の変更は、差が非負かつ `uint64_t` に収まることを使い、対象の値域を狭めずに行った。測定のばらつきもあるため、各差を全て変更の効果とは断定しない。

初期版と全体操作を改善した版のソースは `initial_sources.tar.gz`・`root_sources.tar.gz`、結果は `initial_generic`・`initial_native`・`root_generic`・`root_native`。最終版は `generic`・`native`。各ディレクトリにソースの SHA-256 と条件、全試行、中央値を保存した。

## 検証

- GCC・Clang でヘッダーの単独 include、既存ヘッダーとの併用、コピー・ムーブ・全体更新の後の部分区間更新を検証。
- 32 bit / 64 bit の最大値・最小値版で合計 384,000 回のランダム操作を素朴な配列と照合。負の係数・同一係数・空区間・全位置無効・再有効化を含む。
- `INT64_MIN`・`INT64_MAX` の係数と値、64 bit 符号付きの範囲を超える交差までの増分、遅延更新の広い累積、積と加算の相殺を検証。
- JSC2024 D の値 `1..4`、長さ `0..7` の全列に対し、部分列への全分配を列挙した解と一致。公開 API だけで解くドライバーを用意した。
- JSC2024 D の全 4 サンプルと、最大長 500,000 の同一値列を検証。
- Library Checker Range Add Range Min の公式 23 ケースを GCC・Clang ともに通過。
- 全体更新を追加した汎用 Beats の非可換作用・伝播中の失敗・非冪長・空配列を検証し、Range Chmin Chmax Add Range Sum の公式 33 ケースも両コンパイラで通過。
- Clang ASan/UBSan、ヘッダー・両ドライバーの `oj-bundle`、説明内コードを検証。

汎用 Beats の変更を検証した後、証明の除算だけを変更した際は、ソース・コンパイラ・入力が同じ汎用 Beats の結果を再利用し、Kinetic Tournament に影響する範囲を再実行した。対象は `selection.json`・`policy_selection.json`、結果は `generic_check.log`・`generic_sanitize.log`・`check.log`・`sanitize.log`・`validation.json` に保存した。JSC2024 D のオンライン提出や全公式入力の実行は行っていない。Apple Clang は既存の macOS CI の対象。

## 再実行

```sh
python3 test/benchmark/kinetic_tournament_reference.py --cache .verify-helper/cache/kinetic_range_references --output test/benchmark/results/kinetic_tournament/reference.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament.py --output test/benchmark/results/kinetic_tournament/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament.py --native --output test/benchmark/results/kinetic_tournament/reproduced_native
```

公式入力は `.verify-helper/cache` 内の各問題 URL の MD5 に対応する `test` ディレクトリを使う。未取得なら先に `oj download --system` で保存する。
