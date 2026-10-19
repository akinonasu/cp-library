# 定数係数畳み込み

基準は `bd29f70c70e24e27770b9e9a3777b6cf85664b63` に、採用済みの汎用結果借用と非対称畳み込みを統合した freeze 版。比較原本は [Library Checker 提出 409465](https://judge.yosupo.jp/submission/409465) の未編集コード。両列の全係数がそれぞれ一定のとき、係数同士の積に重なりの個数を掛ければ、NTT を使わず線形時間で畳み込める。

短辺が 4096 以上のときだけ、両入力の先頭 8 項を正規化して SIMD 比較する。ここで不一致なら直ちに既存経路へ戻り、一致したら全残部を厳密に確認する。入力中の二つの剰余表現が同じ値を表す場合も定数として扱う。判定・計算の対象は係数の値と長さであり、入力名に依存しない。

定数係数の積を正規化した Montgomery 値 $c$ とし、前半を $c,2c,\ldots$ の 8 lane 等差列で作る。対称な後半には lane を反転して書き、中央の一定区間を埋める。`vector<Mint>` の全要素の生存期間を開始してから `memcpy` で書く。零でも公開サイズを保ち、全 `.v` を $p$ 未満にする。時間は $O(n+m)$、返す配列を除く作業領域は $O(1)$。

## 21 回の確認

GCC 14.2、Linux x86_64、AMD EPYC 9V74。全版 `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、CPU 2 固定。コンパイルと正しさ検証は共有 lock、計時は排他 `/tmp/cp-fps-tuning.lock`。各版 warmup 一回の後、各入力 21 回、版の順序を毎回 shuffle した。child process の起動・入出力・計算・終了を含める。全 352 出力が公式 `checker input actual answer` に成功した。

| 入力 | 基準 ms | 採用 noinline 版 ms | 未編集原本 ms | paired 改善中央値 ms | 改善回数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| all_same_00 | 13.110064 | 8.880370 | 8.575195 | 4.199821 | 21/21 |
| all_same_01 | 12.135312 | 8.061457 | 9.160287 | 4.076869 | 21/21 |
| max_random_00 | 12.384984 | 12.344543 | 9.918466 | −0.012298 | 10/21 |
| max_random_01 | 12.547194 | 12.462880 | 10.128489 | 0.030705 | 12/21 |

定数入力で約 4.1〜4.2 ms の改善が続き、ランダム入力は paired 差で約 −0.01 / ＋0.03 ms だった。個別中央値の差と paired 中央値は異なる統計量である。原本との比較はこの環境の同条件測定であり、Yosupo 実機の最速獲得を示す値ではない。

最初の 9 回では、inline 候補の定数入力が約 4.2 ms 改善した一方、random_00 に paired 0.196 ms の退行が見えた。元の基準・候補・原本の binary を変更せず、`fits` と `multiply` の二関数だけ `[[gnu::noinline]] inline` にした版を一つ追加し、上の四版・21 回比較を行った。noinline 版では最初の random_00 の退行は再現しなかった。元 inline 候補の 21 回の random 差は −0.057 / −0.047 ms で、採用版が良好だった。9 回の値は原票から消さず、21 回の値で上書きしていない。

## 検証

private noinline 版で、素朴な重なり式だけでなく、既存の所有型 `run<ntt::Convolution>`、公開 `convolution`、`fps::multiply` と照合した。4095 / 4096 / 4097 / 8192 項で判定の前後を確認し、末尾だけ非定数に変えて既存 NTT へ戻る経路も比較した。入力交換、同一入力、零、遅延剰余、read-only 入力、先頭・末尾 guard page、最終 SIMD 端数、全出力の `.v<p` を確認した。

最終 unit は GCC 14 normal / ASan・UBSan、GCC 13 Werror、Clang 19 fallback、GCC 14 `-U__linux__` standalone、GCC 13 / Clang 19 standalone に成功した。最終検証の再利用用に、`-std=c++2b -O2 -Wall -Wextra -DDEBUG=1 -I tree` とその sanitizer flags でも normal / sanitizer を追加確認した。コンパイラ実体の SHA-256・version、全 actual `-MM` 依存、実コマンド・flags、source / binary / log SHA、戻り値を保存した。

採用 header は `bf6976669ca416df9c5cb8999814a164fc04502eaa8be900061684674df01ca6`、最終 unit は `3594f641df7aac90c65ca0e5ae3fc528681c76151a843f9386c65b350068bd3f`。private の header と dispatch は配置した production と byte 同一。今回の private 比較とは別に、root が最終統合状態の公式全入力・13 入力の全体計測・公開 FPS 依存先を確認する。[最終統合比較](fps_convolution_constant_final_round4.md)を参照。

## 原票と再現

[原票](results/fps_convolution_constant_round4/)に 9 / 21 回の全試行と checker、四つの binary の SHA、元の harness、flags・compiler・actual `-MM`、基準 tree、二つの patch、未編集原本を保存した。元三本の binary は二つの比較間で同一。9 回時の unit と harness の SHA も保持する。`private_validation/reuse_checks.json` は DEBUG 込み normal / sanitizer の再利用用証拠、`final_checks.json` は他七条件の検証である。

```sh
python3 test/benchmark/results/fps_convolution_constant_round4/reproduce.py --work /workspace/constant-prepare
python3 test/benchmark/results/fps_convolution_constant_round4/reproduce.py --work /workspace/constant-run --check --measure --official /path/to/library-checker-problems/convolution/convolution_mod
```

最初のコマンドは全 artifact の hash 確認と基準・patch・原本の復元までを行う。二つ目は四版を同じ flags でビルドし、unit と 21 回の測定を行う。`--screen9` で最初の三版・9 回、`--sanitize` で sanitizer unit も再現できる。新しい結果は作業先の `reproduced.json` に保存し、元の raw 値を書き換えない。生 driver は `.txt`、基準 tree と原本は圧縮ファイルに保存し、artifact に `*.test.cpp` を置いていない。
