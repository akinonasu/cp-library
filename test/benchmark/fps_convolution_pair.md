# Convolution の paired forward の適用範囲

2026-10-18 UTC、radix-4 の forward を入力 2 本まとめて処理する範囲を広げたが、kernel・プロセス全体ともに退行したため不採用とした。製品ヘッダーは変更していない。

基準は `144ae32387675fa2a889ea3e8cd19b03ad5016bf`、比較原本は [convolution_mod / 409465](https://judge.yosupo.jp/submission/409465)。原本の SHA-256 は `6cd230b64c18b2d41870f0be209727b019e0b7fca6b036f19f9d802b9d0a883d`。

現行と原本の `Product::Subtrees::forward` は、`h == 4` でだけ `forward_pair` を使い、それより上の段では通常の forward を入力ごとに実行する。候補は一般の `h` でも既存の paired asm を使い、入力 2 本の独立した演算を交互に実行した。入力コピー・正規化・根表の初期化・配列確保・出力処理は変えていない。前回の入力融合や根表初期化省略の候補とは別の変更である。

## 測定

AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2 固定。全版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でビルドし、assert を有効にした。予備実行 1 回の後、seed `20261021` で反復内の順序をシャッフルして各 21 回測定した中央値。ビルド・検証は `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロックを使った。

kernel は公開 `fps` の積で、作業領域と結果の確保を含み、乱数入力の生成と結果のチェックサムは除く。入力 2 本は同じ長さで、seed `731` の乱数から作った。

| 入力の項数 | 基準 | 候補 |
| ---: | ---: | ---: |
| 8,192 | 0.098037 ms | 0.097986 ms |
| 131,072 | 1.166389 ms | 1.380599 ms |
| 500,000 | 5.280593 ms | 5.950111 ms |
| 524,288 | 5.268034 ms | 5.771214 ms |

whole は公式入力を使い、通常ファイルの入出力とプロセスの起動・終了を含む。

| 公式入力 | 基準 | 候補 | 原本 409465 |
| --- | ---: | ---: | ---: |
| max_random_00 | 15.180912 ms | 15.459697 ms | 11.572332 ms |
| max_random_01 | 14.792302 ms | 15.055194 ms | 11.339505 ms |

同じ反復内の「基準 − 候補」の中央値もそれぞれ -0.306216 ms、-0.229012 ms だった。候補が速かった回数は 9/21 と 5/21。大きい段に paired kernel を使う利益は確認できず、現行の適用範囲を維持した。レジスタ使用やロードの配置の影響は分離して測定していない。

## 検証と保存物

一般の `h` で正しく計算できることは、`h = 4,16,64,256,1024,4096,16384`、根の group `1,2,3,17`、`[0,4p)` の内部表現を使って、従来の forward 2 回と剰余で照合した。末尾 16 語の canary も保持された。全 kernel のチェックサムは基準と一致し、whole の予備実行を含む全出力が公式 checker を `input actual answer` の順で通過した。不採用候補のため追加の sanitizer は実行していない。

[results/fps_convolution_pair/](results/fps_convolution_pair/) に原票 `results.json`、原測定ハーネス `bench.py`、差分 `candidate.patch`、範囲検証 `verify.cpp`、全ヘッダーハッシュと検証記録 `metadata.json`、実行ログを保存した。公式問題のコミットは `1814c4e5205517e368bb57a8d1127eb961cfeaae`。復元では基準・候補それぞれ 174 ヘッダーを照合する。

```sh
flock -s /tmp/cp-fps-tuning.lock \
  python3 test/benchmark/results/fps_convolution_pair/reproduce.py \
  --work /tmp/fps-convolution-pair-reproduce --prepare-only
```

`--prepare-only` を外すと paired kernel の検証をビルドして実行する。`--measure` を付けると、保存した原本ハーネスのパスだけを置き換えたコピーで全測定を行う。測定用には生成済みの公式 `convolution_mod` と原本 409465 が必要で、場所は `--official-root` と `--reference` で指定できる。測定ハーネスが共有・排他ロックを分けて取るため、`--measure` の呼出し全体へ共有ロックをかけない。復元先は存在しないディレクトリを指定し、保存済み原票は上書きしない。
