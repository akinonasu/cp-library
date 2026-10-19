# 畳み込みの結果領域再利用

`Product` の作業配列 A を返り値の `vector<Mint>` 内へ移し、変換後に別の結果配列を確保してコピーする処理を省いた。公開 API、`const` 入力、正規化した出力は維持する。専用 `Product` の適用条件と、その他の長さ・Clang・Mac の既存経路は変更していない。

比較元は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。原本は [Library Checker 409465](https://judge.yosupo.jp/submission/409465)。候補間で I/O と `Mint` の実装は baseline に固定しており、同時進行の I/O・default constructor の変更はこの数値に含まない。原本の SHA-256、全入力・ヘッダー・実行ファイル・測定順は [原票](results/fps_convolution_borrow_round3/assign_confirm_41.json) に残した。

## Whole process の結果

AMD EPYC 9V74、CPU 2 固定、GCC 14.2.0。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。同一入力を baseline・候補・原本で順序を shuffle し、各 1 回の warmup 後に 41 試行。外部プロセスの起動、入力、計算、出力、終了を含み、出力ファイルの生成と全試行の公式 checker 検証を行った。コンパイル・検証は共有 lock、計時区間は排他 lock を使った。

| 入力 | baseline ms | 候補 ms | 原本 ms | paired 改善 ms | 候補が速い試行 |
| --- | ---: | ---: | ---: | ---: | ---: |
| max_random_00 | 15.084 | 14.449 | 11.645 | 0.543 | 37/41 |
| max_random_01 | 14.949 | 14.779 | 11.518 | 0.328 | 29/41 |
| small_and_large_03 | 10.878 | 10.599 | 8.967 | 0.373 | 30/41 |
| medium_02 | 1.406 | 1.403 | 1.159 | 0.015 | 26/41 |

各時間は中央値、paired 改善は同じ trial の `baseline - 候補` の中央値。大きい二つの入力と非対称入力で改善を再確認し、32 byte alignment の `assign` 版を採用した。medium は差が小さく、速度向上の根拠にはしない。先行する [21 試行](results/fps_convolution_borrow_round3/assign_21.json) でも大きい入力の改善を確認している。原本との差は残っており、この測定から fastest 到達は主張しない。AVX-512 は使わない。

## 初期化方法で変わった生成コード

返り値の全領域は、利用前に生存中の零の `Mint` にする必要がある。単に `resize(words)` すると、今回の GCC では各係数の零書き込み後に、もう一度零を書き込む `Mint` default constructor を呼ぶループになった。大きい入力の診断用 11 試行では、初期化だけで約 1.46 ms かかり、出力のコピー削減を上回った。

`insert(end,words,Mint::word(0))` も試したが、`-ftrivial-auto-var-init=zero` と一時値の生成コードが組み合わさり、stack 上の byte store と直後の word load が係数ごとに繰り返された。この候補は大きい入力で約 5.4–5.6 ms 遅く、不採用とした。採用版は `prepare_fps_output` で容量を確保した後、空の vector に `assign(words,Mint::word(0))` する。コピー初期化で生存期間を開始し、同じ零初期化をまとめて処理させる。

診断用 [profile 原票](results/fps_convolution_borrow_round3/profile_11.json) では、従来の結果コピーが 0.69–0.70 ms、借用領域の先頭への `memmove` が約 0.05–0.06 ms だった。一方で NTT は約 4.1 ms で大きく悪化していなかった。計時用 instrumentation はコード配置を変え得るため、採否は上の instrumentation のない whole process 比較で決めた。区間ごとの中央値の和は whole の中央値とは一致しない。

生成コードの抜粋は [resize](results/fps_convolution_borrow_round3/borrow32_codegen.txt)、[fill insert](results/fps_convolution_borrow_round3/borrow32_color_fill_codegen.txt)、[assign](results/fps_convolution_borrow_round3/borrow32_assign_codegen.txt) に保存した。

## 配列と所有権

返り値は変換長に末尾 16 係数と alignment 用 7 係数を加え、全領域を零初期化する。32 byte 境界からの内側を A として借用し、B と root 表は `Product` が所有する。変換後は必要な係数だけ先頭へ `memmove` し、公開サイズへ縮める。変換後の vector の容量には作業用の余白が残り、時間・空間の漸近計算量は従来と同じ。

B は A の次の 64 byte cache set から始めるよう、owned 領域内の開始位置を最大 1008 word ずらす。追加領域は 1024 word。これは候補に残っているが、resize のまま色だけを変えた比較では改善しなかったため、独立した速度向上として扱わない。

`Mint` は 4 byte の trivially copyable 型で、借用範囲の全要素は生存中。入力取り込みは `Mint` と `memcpy` を使い、`Mint[]` を横断する scalar `uint32_t` 書き込みを行わない。変換の A に対するアクセスは GCC の `__may_alias__` を持つ `__m256i` または assembly で行う。最終段は正規化した Montgomery 表現に戻す。公開入力は別領域のまま変更せず、同じ配列を両引数に渡した場合も扱う。既存の所有型 `Product(n,m)` と raw API は維持した。

## 検証と不採用候補

採用版は公式 53 入力すべてで checker を通過。追加の [単体テスト](../../test/unit/fps_convolution_borrow.cpp) は GCC の `-Wall -Wextra -Werror` と ASan/UBSan を通過した。read-only 入力ページ、入力の完全な word 一致、同一配列、lazy 表現、零入力、公開長、出力正規化を確認する。借用配列は配置 new で全 `Mint` を生存させ、末尾を guard page に接して配置し、末尾 16 係数と `Product` 破棄後の借用領域も確認する。Sanitizer は `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`。検証時のヘッダーは採用時と同じ `c756be0b18cd5435641e568c977e2e71aced49d3a7393bd27fb749372ed8067e`。

| 候補 | 大きい入力での傾向 | 判断・原票 |
| --- | --- | --- |
| 32 byte + resize | 約 0.7–0.9 ms 遅い | [不採用](results/fps_convolution_borrow_round3/initial_resize_21.json) |
| 32 byte + cache color + resize | 約 0.8–1.0 ms 遅い | [不採用](results/fps_convolution_borrow_round3/color_resize_21.json) |
| 32 byte + fill insert | 約 5.4–5.6 ms 遅い | [不採用](results/fps_convolution_borrow_round3/fill_insert_21.json) |
| 2 MB + assign | 大きい二入力の改善が小さく、非対称入力は遅い | [不採用](results/fps_convolution_borrow_round3/assign_21.json) |

2 MB alignment は追加確保・初期化の費用を含めて測定した。小さい入力も 2 MB に揃える初期案は特に遅いため、後続候補では変換長 $$2^{19}$$ 以上だけ 2 MB にした。各候補の patch と元の生成・測定 script は [保存先](results/fps_convolution_borrow_round3/metadata.json) に hash 付きで残した。

## 再現

次の script は固定 baseline を `git archive` し、保存 patch を当て、ヘッダーと原票の hash を確認する。既存ディレクトリは上書きしない。準備だけではコンパイル・計測しない。

```sh
python test/benchmark/results/fps_convolution_borrow_round3/reproduce.py --work /tmp/reproduce-conv-borrow
```

再検証・計測は未使用の `--work` と `--check --measure` を指定する。公式入力・checker・原本の場所は `--official` と `--reference` で変更できる。`--variant` で不採用候補も再構成できる。通常の whole process 測定 harness は [fps_convolution_borrow_round3.py](fps_convolution_borrow_round3.py)、当時の正確なコピーは `initial_whole.py`。計測は CPU 2 を使い、各候補のコンパイルごとに共有 lock を解放し、排他 lock の内側で計時する。
