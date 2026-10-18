# 多点評価・補間・Taylor shift

2026-10-19（日本時間）の比較。baseline は `ce81108fe8e4f3a93d816c94e475d02ef0329608`。Linux x86_64、AMD EPYC 9V74、GCC 14.2、CPU 2。公開 API と汎用係数型の変換は維持しています。

## 参照と不足していた実装

| 関数 | 最速 AC 原本 | 旧実装に不足していたもの |
| --- | --- | --- |
| `multipoint_eval` | [409516](https://judge.yosupo.jp/submission/409516)、15 ms | SIMD の各 lane に1本ずつ置く8本の product tree、変換を保持した中間積の降下、上位の標準配置から lane 配置への変換 |
| `interpolate` | [409545](https://judge.yosupo.jp/submission/409545)、20 ms | 上記の木の共有、評価の降下と補間の上昇の統合、lane 内の小さい積和、lane 単位の逆数の一括計算 |
| `taylor_shift` | [409534](https://judge.yosupo.jp/submission/409534)、10 ms | 32本の独立した階乗の列、重みと係数の積の融合、上半分の中間積、最終逆変換・差・出力重みの融合 |

`fps_evaluation.hpp` は409545に同梱された product_tree/evaluation/interpolation を移植しています。409516より新しい、標準配置の変換を lane 配置へ直接移す処理も含みます。Taylor は409534の階乗表・32本の列・重み・出力を移植し、変換と葉の積は共通の `fps_convolution.hpp` にまとめています。

native の入力多項式・補間の値・Taylor の係数については線形性を使います。Montgomery の共通因子を出力まで保持することで、API の境界での往復変換を省きました。評価点と Taylor のシフト量は通常表現へ変換します。

原本の匿名 mmap に依存したゼロの初期値は維持し、通常の整列確保へフォールバックする場合だけ明示的にゼロを書きます。配列と padding は RAII の所有領域内に置きます。原本の単発プログラムが行う早期起動・終了や、作業領域の解放省略はライブラリへ導入していません。

## 最大入力の全体時間

`max_random_00.in` と `max_random_01.in`。1回の warm-up 後、3実装の実行順をシャッフルして11回。通常の main、入出力、内部の確保・解放、プロセス終了を含みます。原本は early entry を含めて改変せず実行しています。

| 操作・入力 | baseline ms | 変更後 ms | 最速原本 ms |
| --- | ---: | ---: | ---: |
| 多点評価・00（131072係数、131072点） | 48.671 | 16.841 | 15.368 |
| 多点評価・01 | 49.564 | 17.360 | 15.250 |
| 補間・00（131072点） | 72.201 | 20.778 | 19.237 |
| 補間・01 | 72.518 | 20.904 | 19.519 |
| Taylor・00（524288係数） | 25.593 | 12.782 | 9.887 |
| Taylor・01 | 24.039 | 12.805 | 9.687 |

コンパイル条件は `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。assertion は有効です。各候補のスナップショットの全ヘッダーハッシュ、ドライバー・バイナリー・原本・入力のハッシュ、全観測値は [fps_evaluation_speed.json](results/fps_evaluation_speed.json) にあります。時間を取る区間は共有の排他ロックを使い、コンパイル・テストと重ねていません。

## 入出力を除く比較と残る差

原本を同じ `Mint` の入力・返り値配列へ包んだ診断です。入力生成・出力走査・返り値の破棄を除き、呼び出し、内部確保、内部領域の解放を測ります。固定 seed 731、131072項、1回の warm-up 後11回。多点評価は409516、補間は409545そのものを使用し、全係数の checksum が一致しました。

| 操作 | 変更後 ms | 原本の API ラッパー ms |
| --- | ---: | ---: |
| 多点評価 | 12.136 | 12.835 |
| 補間 | 16.300 | 16.917 |

この比較には現在版の多点評価で `FormalPowerSeries` の一時配列を作るコストも含まれます。原本にラッパーを付けた結果を、改変しない原本の whole-process 時間と同一視していません。[fps_evaluation_kernel.json](results/fps_evaluation_kernel.json) に全観測値・生成コードとバイナリーのハッシュを残しています。

全体ではまだ原本より遅いため、fastest 達成とはしていません。Montgomery 共通因子を保持する変更の前に行った Taylor の診断では、入力 3.124 ms / 計算 5.610 ms / 出力 1.736 ms、原本を通常 main で動かした場合は入力 1.239 ms / 計算 5.129 ms / 出力 1.459 msでした。入力には係数の確保も含み、参照原本の Arena 初期確保 0.0048 ms は別です。この診断は改善後の最終速度の主張に使用せず、入出力の残差を調べるための結果です。原本の汎用一括パーサーと、ライブラリの parser の差が引き続き残ります。

## 検証と再現

公式 checker で多点評価11、補間9、Taylor38の計58ケースを確認し、上記の計時出力216回も checker に渡しています。checker の引数は `input actual official` です。

`test/unit/fps_evaluation.cpp` は非2冪、点数と次数の非対称、ゼロ・重複点、lazy Montgomery 表現、補間の round-trip、Taylor の最大524288項を検証します。通常の Werror と ASan/UBSan が成功しました。2本の新規 component は `-march` なしの standalone Werror も成功しています。Linux x86_64 / GCC 以外と AVX2 非対応時は従来実装を使います。Mac の速度向上は主張しません。

```
python3 test/benchmark/fps_evaluation.py --output /tmp/fps-evaluation-run
python3 test/benchmark/fps_evaluation_kernel.py --output /tmp/fps-evaluation-kernel-run
```

参照キャッシュは `test/benchmark/fps_speed_reference.py` で取得できます。公式問題は `/tmp/library-checker-problems`、参照は `/tmp/fps-reference` が既定です。どちらも引数で変更できます。最新ランキングの取得記録と原本ハッシュは [fps_evaluation_reference.json](results/fps_evaluation_reference.json) にあります。Bostan–Mori・線形漸化式は [fps_bostan_speed.md](fps_bostan_speed.md) に分けています。
