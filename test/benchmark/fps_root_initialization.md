# FPS 根表の初期化

根表の `std::fill_n` を省く候補を比較し、**不採用**とした。2026-10-19（Asia/Tokyo）に測定した。この候補は製品ヘッダーへ取り込んでいない。

比較元は `4765ceb5f995801d27b23840392c9c354ecdecb9`。測定時の main `520e972` は wavelet の追加で、対象の 104 ヘッダーとドライバーは比較元と一致する。[一致の記録](results/fps_root_initialization/baseline_equivalence.json)を保存した。

## 候補と安全性

候補は `components/fps_blocked.hpp` の `blocked_poly::Transform` コンストラクターの 1 行だけを変える。

```cpp
if (!arena.zero_initialized()) std::fill_n(roots_, 2 * table, 0u);
```

`zero_initialized()` が真になるのは、新規の `MAP_PRIVATE | MAP_ANONYMOUS` の領域を所有するときだけである。この mapping は OS によりゼロで初期化される。`Arena::take()` は整列と padding を含めて単調に進み、返した領域を再利用しない。根表はコンストラクター内の新しい `take()` で確保するため、先に使った作業領域と重ならない。`MADV_HUGEPAGE` の成否は、匿名 mapping のゼロ初期化を変えない。

通常の整列した `new[]` や mmap 失敗時は `zero_initialized()` が偽になり、従来の `fill_n` を実行する。呼び出し側の入力・出力の整列、padding、API に前提を追加しない。根表以外の初期化、係数変換、入出力、計算手順は変えていない。[候補パッチ](results/fps_root_initialization/candidate.patch)と、比較元・候補の全ヘッダーハッシュを保存した。異なるヘッダーはこの 1 件だけである。

安全性の検査では、一つの Arena に `0xdeadbeef` で埋めた領域を置いてから、長さ $2^6$・$2^8$・$2^{18}$・$2^{20}$ の Transform を順に作る。両方の根表の全ワードを、別途明示的にゼロ化して構築した表と比較し、未使用部分の一致と既存領域の保存も確認する。小さい `new[]` の領域も調べる。

mmap を `MAP_FAILED` に置き換えた一時テストで大きい配列の fallback も確認した。比較元・候補 × 通常 mmap・強制 fallback × 通常・ASan/UBSan の 8 実行が成功した。`ASAN_OPTIONS=detect_leaks=0`、`UBSAN_OPTIONS=halt_on_error=1` を使う。LeakSanitizer の無効化は既存環境の ptrace 制約による。テストの原本は [safety.cpp](results/fps_root_initialization/safety.cpp)、コンパイル条件・出力は原票の `builds.safety_*` にある。

## 測定条件と統計量

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 固定。両版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でビルドし、assert を有効にした。各条件の予備実行後、seed 20261020 で各反復内の実行順をシャッフルして 21 回測定した。

ビルド・安全性検査には共有 flock、計時には `/tmp/cp-fps-tuning.lock` の排他 flock を使い、計測中に他のコンパイル・検証を並行実行しない。kernel は入力生成とチェックサムを除き、公開 API 内の確保と結果作成を含む。whole は通常ファイルからの入力、計算、通常ファイルへの出力、プロセスの起動・終了を含む。全 kernel チェックサムを両版で照合し、whole の予備実行を含む全 220 出力を公式 checker で `input actual official` の順に照合した。

次の表の `中央値差` は「比較元の中央値 − 候補の中央値」、`対応差中央値` は「同じ反復内の比較元 − 候補」の 21 個の差の中央値である。どちらも正なら候補が速い。二つは別の統計量なので一致しない。`勝数` は候補が速かった反復数である。単位は ms。

## 結果

kernel。

| 項数 | 操作 | 比較元中央値 | 候補中央値 | 中央値差 | 対応差中央値 | 勝数 / 21 |
| ---: | --- | ---: | ---: | ---: | ---: | ---: |
| 32,768 | log | 0.586054 | 0.579524 | +0.006530 | +0.003926 | 12 |
| 32,768 | exp | 0.719312 | 0.723750 | -0.004438 | +0.007321 | 11 |
| 32,768 | pow | 1.151167 | 1.138727 | +0.012440 | +0.013240 | 12 |
| 32,768 | inv | 0.454147 | 0.465884 | -0.011737 | -0.013209 | 8 |
| 32,768 | sqrt | 0.473106 | 0.462630 | +0.010476 | -0.000250 | 10 |
| 131,073 | log | 2.052360 | 2.059781 | -0.007421 | -0.030155 | 8 |
| 131,073 | exp | 3.946294 | 3.925072 | +0.021222 | +0.023895 | 13 |
| 131,073 | pow | 5.743455 | 5.712327 | +0.031128 | +0.019720 | 16 |
| 131,073 | inv | 2.045820 | 2.065910 | -0.020090 | -0.005638 | 9 |
| 131,073 | sqrt | 2.549822 | 2.536863 | +0.012959 | +0.014342 | 12 |
| 500,000 | log | 9.028052 | 9.064968 | -0.036916 | +0.018417 | 12 |
| 500,000 | exp | 11.540818 | 11.359758 | +0.181060 | +0.183664 | 14 |
| 500,000 | pow | 19.087782 | 19.049765 | +0.038017 | +0.028653 | 13 |
| 500,000 | inv | 7.347216 | 7.410070 | -0.062854 | +0.000552 | 11 |
| 500,000 | sqrt | 7.244271 | 7.122380 | +0.121891 | +0.089793 | 12 |

whole。各操作は公式 `max_random_00`、sqrt は解のある `max_random_01` を使用した。入力のハッシュは原票にある。

| 操作 | 比較元中央値 | 候補中央値 | 中央値差 | 対応差中央値 | 勝数 / 21 |
| --- | ---: | ---: | ---: | ---: | ---: |
| log | 15.203100 | 15.185183 | +0.017917 | -0.093491 | 10 |
| exp | 17.449869 | 17.829676 | -0.379807 | -0.167840 | 9 |
| pow | 25.193876 | 25.274336 | -0.080460 | -0.037396 | 9 |
| inv | 12.724903 | 12.781168 | -0.056265 | -0.032468 | 7 |
| sqrt | 13.009748 | 12.977630 | +0.032118 | -0.009916 | 9 |

50 万項の kernel では exp と sqrt に小幅な短縮が見られたが、log と inv の版別中央値は悪化した。whole の log は版別中央値では約 0.018 ms 短縮し、対応差中央値は約 -0.093 ms となる。sqrt も同じ傾向で、中央値の引き算だけでは候補が速く見える。

whole の対応差中央値は 5 操作とも負で、候補の勝数は 7〜10 / 21 にとどまった。処理全体で安定した改善が得られず、残る数 ms の差を埋める利益も確認できなかったため採用しない。

## 原票と再現

[raw.json](results/fps_root_initialization/raw.json) に全反復、対応差、コンパイラーと CPU、入力・ドライバー・ヘッダー・バイナリの SHA-256、安全性検査の条件と出力を保存した。[実行ログ](results/fps_root_initialization/run.log)、[測定時のハーネス原本](results/fps_root_initialization/original_harness.py)、[SHA-256 一覧](results/fps_root_initialization/manifest.json)も含む。原票が記録したハーネスと安全性テストの SHA-256 が保存したファイルに一致することを確認した。

GitHub から別の一時ディレクトリへ clone し、比較元の commit から source を archive した。パッチを候補の tree だけに適用し、比較元・候補それぞれの全 104 ヘッダー、kernel ドライバー 2 件、whole ドライバー 10 件が原票の SHA-256 と一致した。[復元結果](results/fps_root_initialization/restoration.json)と[clone ログ](results/fps_root_initialization/clone.log)を保存した。

資料を含む clone を用意した後、原票の照合と tree の復元だけを実行できる。`--work` は存在しないディレクトリを指定する。

```sh
git clone https://github.com/qandori/cp-library.git /tmp/cp-library-replay
python3 /tmp/cp-library-replay/test/benchmark/results/fps_root_initialization/restore.py --repo /tmp/cp-library-replay --work /tmp/fps-root-restored
```

全測定を再実行する場合は、GCC、Linux x86_64 / AVX2、CPU 2、生成済みの公式入力・出力と checker を用意する。ハーネスはビルド・検査を共有 flock、計時を排他 flock で実行する。

```sh
python3 /tmp/cp-library-replay/test/benchmark/results/fps_root_initialization/replay.py --repo /tmp/cp-library-replay --work /tmp/fps-root-rerun --official-root /tmp/library-checker-problems
```

`replay.py` は原本の SHA-256 と source を検証してから、パスと baseline の完全な SHA だけを環境に合わせたコピーを作る。候補パッチ、flags、seed、21 試行、計時と照合方法は維持する。`--prepare-only` で復元とハーネス作成だけを確認でき、この準備も検証済みである。[準備の記録](results/fps_root_initialization/replay_preparation.json)を保存した。

バイナリの SHA-256 は原票の実体の記録であり、コンパイラー・標準ライブラリ・絶対パスが異なる再ビルドでの一致は要求しない。source の復元と保存原票の SHA-256 を照合する。別の実行環境で同じ測定時間になるとは仮定しない。
