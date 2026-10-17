# Mod of Linear の Prefix Minimum

2026-10-16 に作成・測定した。公開ヘッダーは
[`math/prefix_min_of_mod_of_linear.hpp`](../../math/prefix_min_of_mod_of_linear.hpp)、
内部の整数幅別の走査は
[`components/mod_linear_prefix_min.hpp`](../../components/mod_linear_prefix_min.hpp)。
使い方は [`docs/prefix_min_of_mod_of_linear.md`](../../docs/prefix_min_of_mod_of_linear.md) にある。

## 出典と比較した手法

指定された [maspy の解説記事](https://maspypy.com/library-checker-min-of-mod-of-linear) と、
[記事の AC 提出 100952](https://judge.yosupo.jp/submission/100952)、
[現行の圧縮実装](https://raw.githubusercontent.com/maspypy/library/9aaac53044867f5a22ca09e58d98a5db065ebb33/mod/min_of_linear_segments.hpp)
を調べた。下側・上側の近似分数を進め、同じ添字差と値の減少幅を持つ更新点をまとめる方針を採用した。

公式 API の `problem=min_of_mod_of_linear&status=AC&order=%2Btime&limit=4` でも上位を調べた。

| 提出 | ユーザー | judge 表示時間 | 確認した特徴 |
| --- | --- | ---: | --- |
| [405749](https://judge.yosupo.jp/submission/405749) | Rohan_Kapri | 0.013 s | 32 bit の交互のユークリッド縮小、最小値1個を返す |
| [389955](https://judge.yosupo.jp/submission/389955) | API に名前なし | 0.013 s | 405749 と同じソースの SHA256 |
| [217811](https://judge.yosupo.jp/submission/217811) | sortA0329 | 0.013 s | 405749 と同じソースの SHA256 |
| [335228](https://judge.yosupo.jp/submission/335228) | mizar | 0.015 s | Rust、線形式の係数を更新する floor sum 型の縮小 |
| [100952](https://judge.yosupo.jp/submission/100952) | maspy | 0.049 s | prefix minimum の全周期分の圧縮から区間の答えを得る |

judge の表示時間と今回のローカル測定は別のものとして扱う。
上位の最小値1個だけを返す実装は、prefix minimum の全更新点を返す実装と出力が異なる。
ベンチマークでは、その数論部分を最小値1個の参考値として載せた。

取得した順位一覧、提出情報、ソースの SHA256、固定した maspypy 側の参照先は
[`prefix_min_of_mod_of_linear_reference.json`](results/prefix_min_of_mod_of_linear_reference.json) に記録した。
第三者のソース全文はローカルの一時キャッシュに置き、リポジトリには追加していない。

## 実装

返り値の各区間は `(x, y, dx, dy, count)`。
`0 <= j < count` に対する更新点 `(x + dx*j, y - dy*j)` を表す。
更新は厳密な減少とし、`x=0` を一度含める。隣の区間との共有端点を出力しない。
空の prefix も扱い、更新がない場合は初期値だけの1区間を返す。

主な実装上の選択は以下のとおり。

- 位置の刻みと値の減少幅をユークリッド互除法と同様に更新し、探索状態を使い回す。
- 各区間で値は半分未満へ減る。区間数は、初期値が正なら高々その bit 幅、初期値が0なら1区間。
- `n` を `m` 以下へ切り詰める。周期は `m / gcd(a, m)` 以下なので、それ以降には新しい更新点がない。
- GCD の前計算、再帰、各更新位置での `(a*x+b)%m` の再評価を行わない。
- `m <= UINT32_MAX` では32 bit 除算を使い、それ以外は64 bit 除算を使う。幅の広い型は上限判定の積に使い、128 bit 除算を使わない。
- 第5引数にコールバックを指定できる。区間を消費する処理では出力用の動的確保が不要。
- vector 版も提供する。複数回の呼び出しでバッファを再利用する場合はコールバックで既存の vector に追加できる。

`uint64_t` の全域に対応する。`n`・`m` が `UINT64_MAX` でも扱え、
`a=b=m-1` のように更新点が `UINT64_MAX` 個ある場合も1区間に収まる。
符号なしの減算と積の上限を保ち、余りが0になったときは後続の除算を行わない。

## 測定条件

- Intel Core i7-13700F、WSL2 Linux x86_64、GCC 15.2。CPU 2 に固定。
- `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native 設定では `-march=native -ftrivial-auto-var-init=zero` も付ける。
- 入力30万件、seed 713。実行順をシャッフルし、各7回の中央値。
- 入力生成・入出力・比較用の検証は計測外。vector の確保と解放、および結果を消費する処理は計測内。
- 独立した専用コードを生成し、記事側の `vc` / `eb` / `ceil` だけを標準 C++ で補った。数論部分は取得した関数を使用する。
- 最速提出の計算関数 `LinearModMin` も取得したソースから取り出した。提出全体の入出力、`Ofast` / ループ展開の pragma は比較へ持ち込んでいない。

`random` は法10億以下で区間長を `[1,m]` から選ぶ。
`full` は同じ範囲で `n=m`。
`short` は区間長64以下。
`fibonacci` は `m=701408733, a=433494437, n=m`。
`wide` は64 bit の法、`wide_short` はその短い区間。
`fibonacci64` は `m=12200160415121876738, a=7540113804746346429, n=m`。
`descending64` は64 bit の法で `a=b=m-1, n=m`。
`degenerate` は `a=0`、`b=0`、`n=0`、`n=1` を混ぜる。

記事の実装は常に全周期の更新点を求め、`X` と `DX` の2配列を返す。
本実装は指定した区間で打ち切り、値も含む区間を返す。
特に `short` では仕事量の違いを含むため、全周期の比較には `full` 行を使う。
最小値1個の列も、同じ出力を求めた場合の比較には使わない。

## 結果

単位は ms、30万件の合計。`baseline` は最初の64 bit 版を
[`prefix_min_of_mod_of_linear_baseline.hpp`](prefix_min_of_mod_of_linear_baseline.hpp) に保存したもの。

| 入力 | baseline | vector 版 | callback 版 | 記事の圧縮実装 | 参考: 最小値1個 |
| --- | ---: | ---: | ---: | ---: | ---: |
| random | 50.020 | 45.446 | 37.384 | 86.998 | 31.063 |
| full | 50.425 | 45.777 | 37.737 | 86.109 | 30.776 |
| short | 15.176 | 13.271 | 11.015 | 86.624 | 12.890 |
| fibonacci | 87.484 | 77.500 | 67.291 | 115.311 | 58.049 |
| wide | 106.337 | 106.366 | 88.342 | — | — |
| wide_short | 15.504 | 14.132 | 11.400 | — | — |
| fibonacci64 | 187.364 | 188.995 | 164.790 | — | — |
| descending64 | 6.436 | 4.606 | 4.007 | — | — |
| degenerate | 5.062 | 3.602 | 3.318 | 35.082 | 11.246 |

`full` で記事の実装に対し vector 版が約1.88倍、callback 版が約2.28倍だった。
本実装の全周期の callback 版は1入力あたり約126 ns。
この環境での測定値であり、AtCoder・Yosupo の実行時間を保証する数値ではない。

native 設定の結果:

| 入力 | vector 版 | callback 版 | 記事の圧縮実装 |
| --- | ---: | ---: | ---: |
| random | 46.181 | 37.278 | 88.400 |
| full | 47.252 | 38.438 | 88.861 |
| short | 13.613 | 11.080 | 87.772 |
| fibonacci | 78.380 | 66.055 | 116.743 |
| wide | 105.484 | 89.008 | — |
| wide_short | 13.980 | 11.817 | — |
| fibonacci64 | 191.014 | 168.768 | — |
| descending64 | 4.696 | 4.136 | — |
| degenerate | 3.597 | 3.386 | 37.050 |

GCC の強制インライン化、vector の `reserve(8)`、バッファの再利用も比較した。
強制インライン化は測定した全種類で遅くなったため採用しなかった。
固定の reserve は通常の入力では改善したが短い入力・退化ケースで遅くなるため、vector 版には追加しなかった。
バッファ再利用はコールバックで利用者が選べる形にした。

探索は各10万入力・3回測定。
全標本・中央値・フラグ・コンパイラと CPU 情報・ソース SHA256 は以下に保存した。

- [初期の比較](results/prefix_min_of_mod_of_linear_exploration/summary.csv)
- [インライン化の比較](results/prefix_min_of_mod_of_linear_inline_exploration/summary.csv)
- [汎用設定](results/prefix_min_of_mod_of_linear_generic/summary.csv)
- [native 設定](results/prefix_min_of_mod_of_linear_native/summary.csv)

各ディレクトリの `samples.csv` に全標本、`environment.json` に環境情報がある。

## 正当性・移植性

- 小さな法の全探索で、全 prefix と、圧縮した全更新点を素朴解と比較した。
- 64 bit の短い区間のランダム2万件、型の境界、GCD が1でない場合、フィボナッチ数、大量の更新点も検査した。
- 広い入力の各区間の始点・中間・終点と、その直前の最小値を既存の `min_of_mod_of_linear` と比較した。最終値・更新直前の値も照合した。
- 各入力種類2万件で64 bit baseline・採用版・強制インライン候補の圧縮出力全体を比較した。32 bit 範囲では記事の実装と、最速提出の最終値とも比較した。
- 本測定では baseline と各採用候補の checksum と区間数を比較した。記事側と最小値1個の実装は出力形式が異なるので、この checksum 比較には含めない。
- GCC 15.2 / Clang 21.1.8 で新規ヘッダーの単独コンパイル、当該単体テスト、依存先の `standalone.cpp` が成功した。
- Clang の AddressSanitizer / UBSan、および ARM64 GCC 15.2 + QEMU aarch64 で単体テストが成功した。
- 圧縮結果の最後から最小値を作るドライバを、Library Checker の Min of Mod of Linear 公式17ケースで検証し、GCC / Clang の両方で AC、計34回成功した。
- `oj-bundle` で展開したヘッダーの単独コンパイルも成功した。
- Mac 実機での追加実行はしていない。Apple Clang の確認には既存の macOS portability CI を使う。GCC / Clang で共通の整数演算を使い、CPU 命令セットの追加要求はない。

対象選択は [selection](results/prefix_min_of_mod_of_linear_selection.json)、
ログは [unit](results/prefix_min_of_mod_of_linear_unit_check.log)、
[sanitize](results/prefix_min_of_mod_of_linear_sanitize_check.log)、
[judge](results/prefix_min_of_mod_of_linear_judge_check.log)、
[validation](results/prefix_min_of_mod_of_linear_validation.json) にある。

## 再現

Linux のリポジトリルートで実行する。GCC / Clang / `oj-verify` は既存の検証環境を使う。
CPU 番号は `--cpu` で指定できる。
初回は出典取得にネットワークが必要。第三者のソースは指定した一時ディレクトリへ保存する。

```sh
python3 test/benchmark/prefix_min_of_mod_of_linear_reference.py --cache /tmp/prefix-min-sources --output /tmp/prefix-min-references.json
python3 test/benchmark/prefix_min_of_mod_of_linear.py --reference-cache /tmp/prefix-min-sources --output /tmp/prefix-min-generic --count 300000 --trials 7
python3 test/benchmark/prefix_min_of_mod_of_linear.py --reference-cache /tmp/prefix-min-sources --output /tmp/prefix-min-native --count 300000 --trials 7 --native --skip-check
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/prefix_min_of_mod_of_linear_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/prefix_min_of_mod_of_linear_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/prefix_min_of_mod_of_linear.test.cpp
```

無関係なライブラリのテストは再実行していない。測定用ブランチは作成しておらず、commit・push も行っていない。
