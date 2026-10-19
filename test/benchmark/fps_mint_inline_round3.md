# Mint の既定コンストラクターの inline 指定

2026-10-19 UTC。`Mint() = default;` に `[[gnu::always_inline]]` を追加した。公開 API と初期値は維持し、GCC14 で vector 初期化に残っていた要素ごとの呼び出しを消す。

## 固定した比較対象

基準は commit `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0` に、このラウンドの Bostan–Mori・exp・入出力の採用変更を反映した header snapshot。候補との差分は `math/mint.hpp` の属性1行だけ。新しい convolution の借用経路はこの snapshot に含まれず、最終統合版での組合せの結果は総合測定で確認する。

- 測定候補の Mint: `85a05c86ecd5f5b46b0103081871cb4bd166983163dfc25bf251b3be69b9c79e`
- 採用した Mint: `733ff4a2ef2ac34b0174980ec373570c45429731301dcf55f72e768011d748fc`

この2ファイルの違いは採用版の末尾の改行だけで、C++ のトークンは同じ。測定原票は実際にコンパイルした候補のハッシュを保持した。

## GCC14 のコード生成

`Mint` の定義は GCC の O3 pragma 内にあるが、O2 の `std::vector` の初期化ではコンストラクターの呼び出しが残っていた。基準版の `vector<Mint>(n)` のループは、要素へ0を保存してから、もう一度0を保存する `Mint::Mint()` を呼ぶ。候補はこれを `memset` にまとめる。`resize()` と配列の初期化でも要素ごとの呼び出しがなくなる。

[検査プログラム](results/fps_mint_inline_round3/traits_codegen.cpp) の assembly では、基準版の既定コンストラクターの call site が5個、候補は0個となった。`make_vector()` はループ内の call 1個から `memset` 1個へ変わる。一つの既定値を返す `make_default()` は両版とも `xor eax,eax; ret` で同じだった。assembly・objdump・[集計](results/fps_mint_inline_round3/compat.json) を保存した。

## API と型の性質

宣言は初回宣言の `= default` を維持する。explicitly defaulted かつ non-user-provided のままで、implicit inline / constexpr、暗黙の exception specification、アクセス、overload、レイアウトを変えない。`v=0` の default member initializer も同じ。既定コンストラクターは、この initializer があるため元から non-trivial である。

GCC14・GCC13・Clang19 で、以下をコンパイル時・実行時に確認した。

- `sizeof` / `alignof` は4、standard layout / trivially copyable は真。
- aggregate / trivial / trivially default constructible は偽。
- default constructible / nothrow default constructible は真。`noexcept(T())` / `noexcept(T{})` は真で、constexpr な既定値は0。
- copy / move construction、copy / move assignment、destruction の triviality を維持。
- vector、`new[]`、`uninitialized_default_construct_n` は0で初期化され、vector の拡張で既存の lazy ワードを保存する。

各コンパイラの O2 と O0 を `-Wall -Wextra -Werror` で検証し、すべて成功した。GNU 属性の効果は実装依存であり、型・初期値の確認とコード生成の確認を分けている。[独立した読み取りレビュー](results/fps_mint_inline_round3/independent_review.md) も、今回の unsigned 初期化だけの constructor に具体的な追加懸念はないと判断した。

## 全体時間

AMD EPYC 9V74 / Linux x86_64 / GCC 14.2.0、CPU2固定。全版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でビルドし、assert を有効にした。ビルド・互換性検証は `/tmp/cp-fps-tuning.lock` の共有ロック、計時は排他ロック。元の公開 API のテスト用ドライバーをそのまま使った。

起動・ファイル入力・計算・出力・終了を含む wall time、単位はms。各版を1回予備実行し、反復ごとに順序をシャッフルした。checker は計時後に `checker input actual answer` の順で実行した。paired 差は同じ反復の「基準版−候補」の中央値で、個別の中央値の差とは異なる。

初回9回、seed 20261105。

| 処理・入力 | 基準版 | 候補 | paired 差 | 勝数 |
| --- | ---: | ---: | ---: | ---: |
| convolution max_random_00 | 15.083 | 14.819 | +0.119 | 6/9 |
| exp max_random_00 | 18.046 | 17.946 | +0.256 | 6/9 |
| Bostan max_random_00 | 54.975 | 54.235 | +0.701 | 8/9 |
| Bostan near_65536_00 | 28.803 | 28.379 | +0.549 | 8/9 |

同じバイナリを使った追加21回、seed 20261106。

| 処理・入力 | 基準版 | 候補 | paired 差 | 勝数 |
| --- | ---: | ---: | ---: | ---: |
| convolution max_random_00 | 14.748 | 14.661 | +0.110 | 14/21 |
| exp max_random_00 | 17.874 | 17.971 | +0.006 | 11/21 |
| Bostan max_random_00 | 56.617 | 55.691 | +0.689 | 15/21 |
| Bostan near_65536_00 | 29.433 | 29.161 | +0.251 | 13/21 |

Bostan–Mori の改善が再現し、4入力に paired で退行は見られなかったため採用した。exp の追加21回はほぼ中立で、高速化とは扱わない。全256回の公式 checker が成功し、依存ソースと control/candidate バイナリの前後ハッシュが一致した。この比較は属性の効果を分離するもので、最速原本との直接比較は総合測定へ分ける。

## 原票と再現

[原票ディレクトリ](results/fps_mint_inline_round3/) に、初回・追加21回の全試行と順序、入出力・checker・ドライバー・全 header snapshot・バイナリのハッシュ、コンパイルコマンド、traits / O0 の結果、assembly、独立レビュー、候補パッチを保存した。`snapshot_base.patch` は固定 snapshot と基準 commit の差分、`candidate.patch` は属性の1行である。`manifest.json` は原票ファイルのハッシュ一覧。

次で固定 snapshot を復元し、記録した全ヘッダー・ドライバーと照合できる。

```sh
python3 test/benchmark/results/fps_mint_inline_round3/replay.py \
  --repository "$PWD" \
  --work /workspace/cp-fps-scratch-round3/mint-inline-replay-new \
  --prepare-only
```

この復元・ハッシュ照合は [実行済み](results/fps_mint_inline_round3/replay_preparation.json)。`--prepare-only` を省略すると、同じ flags で2版を再ビルドし、GCC の traits 検証と9回＋21回の全体測定を再実行する。GCC13 / Clang19 の互換性検証は原票のコマンドに保存した。再現ハーネスで全ビルド・計時を再実行してはいない。コンパイラやパスが異なると、バイナリのハッシュや絶対時間は変わり得る。
