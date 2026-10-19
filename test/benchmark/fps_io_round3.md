# FPS 入力の中間コピー削減

2026-10-19 UTC。基準は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。公開 FPS API と `FastInput`・`FastOutput` の契約を保ち、AVX2 の係数入力を変更した。

## 変更

従来は 32768 係数ごとに parser の作業領域を確保し、4 ストリームを一つの整数配列へ再結合してから Mint 配列へ変換していた。新経路は作業領域を一回の係数入力で所有し、4 ストリームをすべて検証した後、それぞれの区間を順に Montgomery 変換して追加する。再結合用の配列とコピー、ブロックごとの作業領域の確保を取り除く。

末尾は 64 整数まで局所配列へまとめ、既存の 64 個の生存中の Mint を使う出力 helper へ渡す。入力の上限・桁・区切りを確認してから採用し、不規則な空白・長い先行ゼロ・入力末尾では従来の整数読み込みへ戻る。`out` の既存要素と、指定個数より後のトークンを保持する。非 AVX2・Clang・Mac の経路は変更していない。

共通の `chunk_blocks` は callback を参照として受け取り、状態を持つ callback を分割ごとにコピーしない。既存の raw `chunk` は最初の区間を直接 `dst` へ読み、従来どおり `3*kCapacity` の追加作業領域で呼べる。raw `read` と公開係数入力は同じ parser を共有する。

## 条件と比較

AMD EPYC 9V74、Linux x86_64、GCC 14.2、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。コンパイルと検証は `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロックで分離した。各版 1 回の予備実行後、順序をシャッフルする。通常ファイルの入出力、確保・計算・結果生成、起動・終了を含む。全出力は `checker input actual answer` で検証した。

最速原本は未編集の [畳み込み 409465](https://judge.yosupo.jp/submission/409465) と [対数 409452](https://judge.yosupo.jp/submission/409452)。公式問題のコミットは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。

次の 21 試行は末尾の 64 個バッチを比較したセッションで、単位は ms。列は各版の全体時間の中央値である。

| 処理・入力 | 基準 | 中間コピー削除 | 末尾バッチ付き | 最速原本 |
| --- | ---: | ---: | ---: | ---: |
| 積 max_random_00 | 14.059 | 13.636 | 13.578 | 10.747 |
| 積 max_random_01 | 14.576 | 14.165 | 14.085 | 11.366 |
| 対数 max_random_00 | 14.576 | 14.262 | 14.397 | 13.150 |
| 対数 max_random_01 | 14.701 | 14.312 | 14.334 | 13.274 |

末尾のバッチによる全体時間の差は小さく、対数ではこのセッションの中央値が悪化した。バッチは短い残部も既存 SIMD 変換へ渡すために採用し、大きな入力の短縮量をバッチだけへ帰属しない。

先行する 21 回と独立した 41 回でも、中間コピー削除版は乱数の最大 2 入力ずつで改善した。41 回の同じ反復の短縮中央値は、積が 0.490 / 0.434 ms（33 / 32 勝）、対数が 0.321 / 0.311 ms（32 / 28 勝）。同一値の積は `all_same_00` で -0.042 ms（15 勝）、`all_same_01` で +0.247 ms（26 勝）と混在した。改善率を全入力へ広げない。

採用版は末尾バッチに callback の参照受け渡しを加えたもの。上の個別比較は修正前の候補であり、最終の統合版は [全関数の比較](fps_round3.md) に別途記録する。異なるセッションの中央値をつないで短縮率を作らない。

## 不採用候補

単なる parser 作業領域の再利用は、積で約 0.08〜0.12 ms 短縮しても対数で退行した。小さい分割幅は入力間で優劣が逆転したため、どちらも採用しなかった。

出力の Montgomery 解除と 16 個の整形を融合する候補も比較した。係数の中間配列と一走査は減るが、21 回の対応差は積の 2 入力で -0.039 / +0.167 ms、対数で -0.093 / +0.147 ms と混在した。レジスタ圧やコード配置の影響は分離していない。共通の出力経路は変更しない。

## 保存物と検証

[results/fps_io_round3/](results/fps_io_round3/) に、`first.json`・`repeat.json`・`refactored.json`・`tail.json`・`output.json` の全試行、各ヘッダー・バイナリ・入力・正解・checker の SHA-256、測定コマンドを保存した。`sources/` は実測時のヘッダー、`adopted.hpp`・`adopted.patch` は採用版である。`harness_v1.py` は最初の測定に使ったスクリプトそのもの。`fps_io_round3.py` は外部候補も指定できる現行ハーネスである。

`make_input.py`・`make_output.py` は保存した基準から候補を再構築する。採用版は次で復元できる。

```sh
mkdir -p /tmp/cp-fps-round3-base
git archive bd951dce6fdcb3ea9db4fcef342147012a8ab0c0 | tar -x -C /tmp/cp-fps-round3-base
python3 test/benchmark/results/fps_io_round3/make_input.py \
  /tmp/cp-fps-round3-base/components/fps_io.hpp /tmp/fps-io-reconstructed.hpp \
  --tail-batch --forward-emit
python3 test/benchmark/fps_io_round3.py \
  --baseline-root /tmp/cp-fps-round3-base --variants baseline \
  --source adopted=/tmp/fps-io-reconstructed.hpp \
  --work /tmp/fps-io-reproduction --output /tmp/fps-io-reproduction.json --trials 21
```

通常の GCC Werror と ASan/UBSan で拡張した単体テストが成功した。空・非空の出力への追加、容量の有無、32/64/1024/32768/65536 周辺、uint32 全域、不規則な空白、読み取り専用入力と直後の guard page、FILE・string_view・並行した別ストリーム、出力エラーを確認した。状態を持つ callback の分割順序と、各ストリーム近傍の不正な桁・uint32 超過で callback を呼ばず入力位置を保持することも検査した。独立の読み取りレビューと記録は `checks.json`、最終統合の依存検査・Clang・公式検証は全関数の比較資料を参照。
