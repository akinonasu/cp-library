# Monotone Minima

## 参考

2026-10-17 に、対応する用途である Library Checker Min Plus Convolution (Convex and Arbitrary) の上位 AC を調査しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [409564](https://judge.yosupo.jp/submission/409564) | 9 ms | 幅優先で標本行を探索し、候補の列挙・ブロック下界・AVX2 で凸畳み込みを計算 |
| [409229](https://judge.yosupo.jp/submission/409229) | 11 ms | 幅優先で標本行を探索し、行のグループを AVX2 で処理 |
| [409218](https://judge.yosupo.jp/submission/409218) | 13 ms | 同系統の標本行の探索・AVX2、異なる出力処理 |
| [375997](https://judge.yosupo.jp/submission/375997) | 45 ms | 汎用 monotone minima と凸畳み込みの専用版を含む、再帰を使わない幅優先の探索 |
| [296643](https://judge.yosupo.jp/submission/296643) | 50 ms | 幅優先の monotone minima を凸畳み込みの式へ直接適用 |
| [202204](https://judge.yosupo.jp/submission/202204) | 62 ms | SMAWK による完全単調行列の探索 |

上位 3 件と、汎用の monotone minima・SMAWK の系統を確認しました。原文の取得時刻・順位一覧・SHA256 は `results/monotone_minima_reference.json`、取得スクリプトは `monotone_minima_reference.py` にあります。judge の記録は手元の測定値と比較できません。

## 実装

最小値を取る列の位置が行番号について広義単調増加するとき、各行の最良の列番号を返します。SMAWK より適用条件が弱く、部分行列まで完全単調であることは要求しません。既存の SMAWK は変更せず、用途に応じて選べるようにしています。

- 行・旧候補列・新候補列を取る比較関数。既存の SMAWK と同じ形。
- 列数を渡す版と、列番号の配列を渡す版。
- 0 行と 1 列を先に処理。これらでは比較関数を呼ばない。
- 最初の行を走査し、残りの行を幅優先で処理。各行を、確定済みの両隣の行の最良列の範囲内で走査。
- 行番号を符号なし整数で処理し、`bit_ceil` と倍幅のステップによるオーバーフローを避ける。
- 列番号を指定する版では、探索中は配列内の位置を保存し、終了時に列番号へ変換。
- 結果の配列だけを確保し、再帰・候補列配列の複製・`std::function` を使用しない。

375997 の汎用部分と 409229 の探索順を参考にしました。最上位の AVX2 版は凸畳み込みの式、32 bit の値域、Monge 性を使う問題専用の実装です。今回の API は値の型や式を持たず、任意の比較関数と Mac・Clang に対応します。

行数 n・列数 m について、n >= 2 なら O(n+m log n) 回以内の比較と結果の書き込みで処理し、返り値を含め O(n) 空間です。探索の作業空間は O(1) です。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の実測ではありません。

全実装に同じ通常ファイルを標準入力として渡し、凸 min-plus 畳み込みの全ての値を出力させました。プロセス起動・入力・計算・出力・終了を含む壁時計時間です。ウォームアップ 1 回、順序を混ぜた 7 回の中央値です。測定中に他のコンパイル・テストは実行していません。全出力の一致と、各入力の最大 35 行について全候補を調べた最小値を確認しています。

`recursive` は再帰による通常の分割統治で、候補が 1 列に絞れた区間は一括で埋める比較版です。`smawk` は既存の実装です。これらと今回の版は同じ Fast I/O・値型・比較関数を使います。

参考提出は原文のまま、409564・375997・202204 をビルドしました。409564 は AVX2 のため `-mavx2` を追加しています。参考提出には独自の入出力・値型・メモリ確保・出力方式があり、時間差は探索関数だけの差ではありません。

| 入力 | 凸列の長さ | 任意列の長さ | 内容 |
| --- | ---: | ---: | --- |
| random | 200,000 | 200,000 | 傾き -32〜32 を整列して凸列を作り、任意列は 0〜10^9 の一様乱数 |
| moving | 200,000 | 200,000 | 凸列は 7i、任意列は全て 0 |
| ties | 200,000 | 200,000 | 両方とも全て 0 |
| thin | 64 | 400,000 | 凸列は絶対値、任意列は 0〜10^9 の一様乱数 |
| single_column | 400,000 | 1 | 凸列は絶対値、候補列は 1 列だけ |

## 結果

単位は ms です。

| 入力 | 今回 generic | recursive | smawk | 409564 | 375997 | 202204 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| random | 25.615 | 25.459 | 22.535 | 16.204 | 21.495 | 21.830 |
| moving | 16.392 | 16.654 | 14.675 | 9.720 | 19.089 | 15.472 |
| ties | 12.311 | 12.555 | 11.228 | 7.488 | 17.243 | 11.965 |
| thin | 26.656 | 27.638 | 28.430 | 20.360 | 25.177 | 29.780 |
| single_column | 17.077 | 17.201 | 17.144 | 11.987 | 18.781 | 16.567 |

| 入力 | 今回 native | recursive | smawk | 409564 | 375997 | 202204 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| random | 25.293 | 25.801 | 21.538 | 18.149 | 21.701 | 22.362 |
| moving | 16.351 | 16.433 | 14.383 | 10.324 | 19.531 | 15.663 |
| ties | 12.232 | 12.629 | 11.133 | 7.198 | 15.745 | 11.928 |
| thin | 26.316 | 27.304 | 29.335 | 21.233 | 25.362 | 29.716 |
| single_column | 16.895 | 16.840 | 16.938 | 11.472 | 18.457 | 16.406 |

再帰版と近い値で、一部の入力では幅優先の方が短い結果でした。最速の問題専用 AVX2 版を上回ってはいません。SMAWK は完全単調行列で比較回数が線形になるため、こちらを使う方が速い入力もあります。

全測定値・フラグ・入力/出力/ソースの SHA256 は `results/monotone_minima_generic/results.json` と `results/monotone_minima_native/results.json` にあります。ソースの SHA256 は LF に正規化しています。

## 検証

- 4×4 以下の二値行列を全列挙し、左端の最小列が単調になる全行列を全探索に照合。
- 1〜75 行・列の全サイズで、最小列が単調なランダム行列を生成。その他の値は独立に選び、完全単調性を仮定せずに検証。
- 同値、最大値、並べ替えた列番号、重複・負の列番号、0 行、1 行、1 列を検証。
- 2 の冪の前後の行数と 100 万行・100 万列の片方が小さい行列を検証。
- 比較関数の添字範囲・比較回数の上界、0 行・1 列での呼び出し回数、move-only な比較関数を検証。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、リリース時の `-Werror`、ヘッダー単独 include を検証。
- Library Checker の凸 min-plus 畳み込み 41 ケースを GCC・Clang で検証。

## 再実行

```sh
python3 test/benchmark/monotone_minima_reference.py --ids 409564 409229 409218 375997 296643 202204 \
  --cache .verify-helper/cache/monotone-minima-reference \
  --output test/benchmark/results/monotone_minima_reference.json
python3 test/benchmark/monotone_minima.py --references .verify-helper/cache/monotone-minima-reference \
  --output test/benchmark/results/monotone_minima_generic
python3 test/benchmark/monotone_minima.py --references .verify-helper/cache/monotone-minima-reference \
  --output test/benchmark/results/monotone_minima_native --native
python3 test/verify.py --unit-only --selection test/benchmark/results/monotone_minima_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/monotone_minima_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 test/monotone_minima.test.cpp
```
