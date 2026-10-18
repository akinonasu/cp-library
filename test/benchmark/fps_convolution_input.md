# 畳み込みの入力コピーと最上段 NTT の融合

2026-10-18 UTC に、公開 API の入力コピーと `Product` の radix-8 最上段をまとめる 2 候補を試した。どちらも全体時間が改善せず、不採用にした。ライブラリのコードは変更していない。

基準は `520e97212aa2fb7f277accbcd0b764213dce5d93`。FPS 関連ファイルは `4765ceb` と同じである。比較した最速原本は [convolution_mod / 409465](https://judge.yosupo.jp/submission/409465)。原本の SHA-256 は `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。

## 候補と結果

現行は Mint の内部表現を `[0,p)` に正規化しながら内部配列へコピーし、続けて最上段を処理する。候補は公開入力を `memcpy` で直接読み、正規化と最上段を同じ走査で行う。完全な stream と、部分 stream の完全な 8 語・端数・零拡張を分けた。公開入力に追加の整列・padding を要求せず、入力を保持し、内部の末尾 16 語は明示的に零にする。`Product::fits` の対象外の経路は変更しない。

2 番目の候補は `[0,2p)` の内部表現をそのまま読み、中間の和・差 3 箇所を `[0,2p)` に縮約する。Shoup 乗算の入力と最上段の出力を `4p` 未満に保ち、入力 4 本の正規化を減らした。

| 候補 | 現行 | 候補 | 原本 409465 |
| --- | ---: | ---: | ---: |
| コピー時の正規化を最上段へ融合 | 14.944374 ms | 15.203626 ms | 11.696329 ms |
| lazy 入力を直接処理 | 14.973619 ms | 15.143031 ms | 11.621800 ms |

各行は別の測定である。AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定で、コンパイルオプションは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。公式 `max_random_00`（両入力 524,288 項）を使い、ファイル入力・ファイル出力・プロセスの起動と終了を含む時間を測った。1 回の予備実行後、seed `20261020` で順序をシャッフルして各 21 回測定した中央値である。ビルド・検証は共有ロック、測定は `/tmp/cp-fps-tuning.lock` の排他ロックで分離した。

全試行の出力が公式 checker を通過し、3 版の出力ファイルも一致した。両候補とも今回の実装では採用する改善がなかった。この結果だけで、残る差の全額が非破壊 API に由来する、または融合による改善が不可能とは言えない。融合時のレジスタ使用や内部配列へ最初に書く順序も変わるが、その影響は分離して測定していない。

## 検証と保存物

両候補を `-Wall -Wextra -Werror` 付きで検証した。長さ 1024 の最上段は入力長 0〜512 のすべてを従来処理と比較した。長さ 4096・16384・1048576 でも stream 境界と 8 語の端数を検査した。積は短い入力・非対称な入力・最大入力・非正規化の内部表現で既存の別経路と照合し、600 項未満では素朴な積とも照合した。入力の `v` フィールドはすべて保持された。独立レビューでも追加の整列・padding 前提は見つからなかった。不採用候補のため sanitizer は追加実行していない。

保存先は [results/fps_convolution_input/](results/fps_convolution_input/)。

- `canonical.json`・`lazy.json` は実測時の JSON 原票そのもの。全試行、ソース・バイナリ・入出力のハッシュを含む。
- `canonical.patch`・`lazy.patch` は、それぞれ基準ヘッダーへ直接適用する候補差分。
- `verify.cpp` は実測前に実行した境界・積・入力保持の検証コード。
- `reproduce.py` は基準ソースの復元、パッチ適用、ハッシュ検査、ビルド、検証、任意の再測定を行う。実測時の測定手順をパス指定可能な形にまとめたもの。
- `archive_check.json` は保存物のハッシュと復元の照合記録。測定時の一時ソースと復元後のソースを各版すべて照合した。

## 再現

リポジトリのルートから実行する。ソースの復元と SHA-256 の照合だけなら、外部の参照ソース・公式入力は不要である。

```sh
python3 test/benchmark/results/fps_convolution_input/reproduce.py \
  --work /tmp/fps-conv-fused-reproduction --prepare-only
```

新しい環境では、公式の問題と参照ソースを用意する。Library Checker の使用コミットは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。以下は `/tmp` にキャッシュを復元する例である。

```sh
git clone https://github.com/yosupo06/library-checker-problems /tmp/library-checker-problems
git -C /tmp/library-checker-problems checkout 1814c4e5205517e368bb57a8d1127eb961cfeaae
flock -s /tmp/cp-fps-tuning.lock \
  python3 /tmp/library-checker-problems/generate.py -p convolution_mod
python3 test/benchmark/fps_speed_reference.py --cache /tmp/fps-reference
python3 test/benchmark/results/fps_convolution_input/reproduce.py \
  --work /tmp/fps-conv-fused-reproduction \
  --official-root /tmp/library-checker-problems \
  --reference /tmp/fps-reference/convolution_mod/409465.cpp --measure
```

`--measure` を外すとビルドと検証までを実行する。CPU 番号と回数は `--cpu`・`--trials` で指定できる。作業先・公式問題・参照ソース・ロックの場所も引数で変更でき、測定時の一時ファイルに依存しない。復元に使う `git archive` のため、基準コミットを含む履歴が必要である。再実行の結果とコマンド・ハッシュは作業先の `reproduction.json` へ保存し、原票は上書きしない。別の作業パスでコンパイルするとバイナリのハッシュは変わる場合があるが、候補ヘッダーは原票と完全一致することを検査する。
