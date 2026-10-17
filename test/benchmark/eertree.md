# Eertree

## 参考

2026-10-17 に Library Checker Eertree の実行時間順の上位 AC を取得しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [406240](https://judge.yosupo.jp/submission/406240) | 68 ms | ハッシュ値順の子の二分探索木を頂点と一体化。文字・親・suffix link・長さ・suffix の個数・末尾での出現回数を保持 |
| [394036](https://judge.yosupo.jp/submission/394036) | 68 ms | 406240 と改行を正規化したソースが一致 |
| [400577](https://judge.yosupo.jp/submission/400577) | 69 ms | 子を連結リストで管理し、2 個の根の遷移を配列に保持。遷移先と文字を整数にまとめ、blazingio を使用 |

順位一覧・取得時刻・原文の SHA256 は `results/eertree_reference.json`、取得スクリプトは `eertree_reference.py` にあります。406240 の原文は CRLF です。測定スクリプト内の SHA256 は LF に正規化しています。judge の記録は下の手元の時間と比較できません。

## 実装

`Eertree` は末尾への追加、相異なる回文の数、総出現回数、各頂点の出現回数、初出区間、最長回文、各接頭辞の最長回文接尾辞を扱います。各頂点は長さ・suffix link・親・初出の右端・回文接尾辞の数・末尾での出現回数・子の先頭・兄弟の 8 個の `int`、計 32 byte です。

- 根 2 個の遷移だけを 256 要素の配列に保持し、頻繁な根への参照を直接処理。
- その他の遷移は、回文の頂点と一体化した連結リスト。辺ごとの確保やハッシュを使用しない。
- 辺の文字は初出の右端から取得し、頂点内の文字フィールドとパディングを削除。
- 文字は番兵 256 付きの `uint16_t` 配列。0〜255 の全バイトを扱い、suffix link の探索中の範囲判定を省略。
- 構築時には `reserve` だけを行い、未使用の頂点を初期化しない。
- 各頂点が最長回文接尾辞となった回数を保存し、出現回数は取得時に suffix link を逆順に伝播。取得後も追加できる。
- 同じ長さの最長回文は、先に発見したものを保持。

遷移先の検索は根で O(1)、その他で O(σ) です。σ は登場するバイトの種類数で、最大 256 です。構築全体は O(nσ)、空間は O(n) です。単一の追加では suffix link を多数たどることがあり、追加は償却 O(σ) です。先頭への追加や削除は扱いません。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の実測ではありません。参考提出の既存の最適化指定は保持しており、400577 は `O3,unroll-loops` と AVX2 を使います。

全実装に同じ通常ファイルを標準入力として渡し、Library Checker と同じ頂点数・親・suffix link・各接頭辞の最長回文接尾辞を出力させました。全出力を空白で正規化して一致させています。プロセス起動・入力・構築・出力・終了を含む壁時計時間です。ウォームアップ 1 回の後、順序を混ぜた 7 回の中央値を採用しました。計測中に他のコンパイル・テストは実行していません。

`dense26` は 26 要素の配列を各頂点に持つ比較実装で、同じ Fast I/O を使用します。文字列の番兵には `int` 配列を使い、頂点は 128 byte です。実装は `eertree_baseline.hpp` にあります。参考提出は原文のままビルドし、394036 は調査のみです。

入力は全て百万文字です。

| 入力 | 内容 |
| --- | --- |
| uniform | `a` の繰り返し |
| alternating | `ab` の繰り返し |
| random2 | 独立に生成した `a`・`b` |
| random26 | 独立に生成した英小文字 |
| zimin | `a`、`aba`、`abacaba`、… と作った文字列の接頭辞 |
| long_fallback | `a` を 999,999 個並べた後に `b` |

`/usr/bin/time` の最大 RSS は時間の計測とは別に 1 回ずつ取得しました。入力・出力や処理系の領域も含む値です。

## 結果

単位は ms です。

| 入力 | 今回 generic | dense26 | 406240 | 400577 |
| --- | ---: | ---: | ---: | ---: |
| uniform | 41.860 | 67.321 | 43.955 | 43.301 |
| alternating | 40.219 | 63.221 | 42.915 | 42.916 |
| random2 | 24.383 | 22.673 | 26.667 | 27.088 |
| random26 | 20.944 | 20.428 | 45.842 | 23.236 |
| zimin | 40.167 | 67.647 | 42.984 | 41.443 |
| long_fallback | 43.384 | 73.521 | 48.268 | 44.786 |

| 入力 | 今回 native | dense26 | 406240 | 400577 |
| --- | ---: | ---: | ---: | ---: |
| uniform | 44.140 | 66.297 | 45.716 | 46.217 |
| alternating | 43.626 | 67.946 | 45.110 | 43.257 |
| random2 | 25.845 | 23.556 | 27.399 | 27.564 |
| random26 | 20.990 | 19.972 | 43.257 | 22.616 |
| zimin | 43.509 | 69.337 | 43.686 | 42.367 |
| long_fallback | 46.691 | 77.308 | 48.015 | 45.519 |

上位 AC と近い値になりました。generic では全測定入力で参考提出より速く、native では 400577 が交互列・Zimin 文字列・長い suffix link の探索を含む入力で少し速い結果です。ランダム列では dense26 が速い場合もあります。全入力で最速とはしていません。

generic の最大 RSS は、uniform で今回 42,388 KiB、dense26 138,180 KiB、406240 48,000 KiB、400577 53,808 KiB です。random26 は順に 11,228 / 13,572 / 12,932 / 36,096 KiB です。

生データ・全オプション・入力と出力の SHA256・ソースの SHA256・各入力の最大 RSS は `results/eertree_generic/results.json` と `results/eertree_native/results.json` にあります。`results/eertree_initial/results.json` は同じ版による 3 回の予備測定です。

## 検証

- 長さ 0〜9 の二値列を全列挙し、各接頭辞で全ての回文を直接列挙して頂点番号・親・suffix link・出現回数・最長回文・総回数を照合。
- 小さい文字集合と全バイトから生成した 400 本を同じ方法で照合。
- 全 256 バイトを並べた列とその反転をつないだ列を確認。各頂点から全 256 バイトの遷移を照合。
- 構築、一括追加、1 文字ずつの追加、コピー、初期化後の再利用、`reserve` の有無を比較。
- 出現回数の繰り返し取得と、その後の追加を確認。
- `a` が 200,000 個並ぶ列で全回文の個数と回数を確認し、末尾への `b` の追加で長い suffix link の探索を確認。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、リリース時の `-Werror` ビルド。
- Library Checker の公式 24 ケースを GCC・Clang で検証。
- ヘッダー単独 include、API 表と公開用 Bundle を検証。

## 再実行

```sh
python3 test/benchmark/eertree_reference.py --ids 406240 394036 400577 \
  --cache .verify-helper/cache/eertree-reference \
  --output test/benchmark/results/eertree_reference.json
python3 test/benchmark/eertree.py \
  --references .verify-helper/cache/eertree-reference \
  --output test/benchmark/results/eertree_generic
python3 test/benchmark/eertree.py --native \
  --references .verify-helper/cache/eertree-reference \
  --output test/benchmark/results/eertree_native
python3 test/verify.py --unit-only --cxx g++ clang++ \
  --selection test/benchmark/results/eertree_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/eertree_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/eertree.test.cpp
```
