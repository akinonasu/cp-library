# FPS 指数・冪の専用バックエンド

比較元は `ce81108fe8e4f3a93d816c94e475d02ef0329608`。2026-10-19（Asia/Tokyo）に Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 固定で測定した。公開 API、const 入力、短い入力に対する零拡張、汎用係数型、他環境のフォールバックを維持する。

## 参照と不足していた処理

指数は [409662](https://judge.yosupo.jp/submission/409662)、冪は [409718](https://judge.yosupo.jp/submission/409718) を比較した。原本の SHA-256 はそれぞれ `ed6a6a427359ce548106dddc7dacb0a7de7bb9bbe5f1db1442799d5ed6998031`、`c11e38eb1639af2738f64482ce505c823801b6f2dadcbd72f4ded9a3c4199757`。原本の全体比較ではソースを変更しない。

従来の指数は 32 分割したブロックごとの積を既存 NTT で求めていた。最速版が使う逆数を保持した Newton 更新、長さを半分にした最終更新、積の一部だけを直接結果へ復元する処理が不足していた。これらを `components/fps_exp_blocked.hpp` に移植した。指数の本体は 409718 と共通の実装で、外部から変換を渡さない場合には指数単体の処理になる。

従来の冪は対数微分を通常の FPS 配列へ出し、それを新しい指数の計算へ渡していた。最速版は対数微分のブロックの変換を使って、指数の最後の更新で必要な変換を組み立てる。この再利用を取り込み、対数微分の変換後 callback を共有部品へ追加した。通常の対数では callback は何もしない。

native の指数は既知の最初の係数を Montgomery 倍にして Newton 更新し、結果全体を内部表現のまま生成する。冪は入力全体の共通の Montgomery 倍を対数微分で相殺し、定数項に出力の内部表現を持たせる。内部 Arena の配列はインプレースに使い、公開入力を変更しない。通常の vector オブジェクトを生成して返す。

共通 Transform には冪の最速版が持つ radix-8 の最上段融合、入力の配置判定、事前計算した逆変換倍率を追加した。これは指数単体の最速 409662 には無い処理で、指数の改善要因とは断定しない。

## 部品ごとの比較

コンパイルオプションは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。assert 有効。1 回予備実行した後、seed 20261019 で順序をシャッフルして 9 回測定し、中央値を取る。単位 ms。計算部分は確保と初期化を含め、入力生成とチェックサム計算を除く。計測中は排他ロックを保持し、他のビルド・検証を動かさない。

最初の専用バックエンドを、共通 radix-8 追加前・従来一括入出力で比較した結果。

| 項数 | 指数・比較元 | 指数・専用版 | 冪・比較元 | 冪・専用版 |
| ---: | ---: | ---: | ---: | ---: |
| 32,768 | 1.164 | 0.741 | 1.824 | 1.212 |
| 131,073 | 4.602 | 4.102 | 7.816 | 6.074 |
| 500,000 | 19.019 | 11.879 | 31.126 | 20.225 |
| 524,288 | 19.727 | 12.042 | 31.549 | 20.547 |

公式 `max_random_00.in` のプロセス全体。通常ファイルから読み、通常ファイルへ書く。全試行の出力を公式 checker で照合する。

| 操作 | 比較元 | 専用版 | 最速の原本 |
| --- | ---: | ---: | ---: |
| exp | 26.194 | 19.068 | 409662: 15.631 |
| pow | 37.222 | 26.887 | 409718: 22.791 |

この段階では全体に約 3〜4 ms の差が残っていた。専用演算だけでは一括入出力や配列の変換の差は解消しない。後続の入出力変更を含む全体の評価は、総合測定と区別する。[初回記録](results/fps_exp_initial.json) に全試行・バイナリ・ソースのハッシュを残す。

radix-8 追加だけを同じ条件で比較した 50 万項の計算時間は、対数 9.642→9.409 ms、指数 11.999→12.024 ms、冪 19.976→19.976 ms だった。指数と冪の大きな短縮にはなっていない。[独立比較](results/fps_exp_radix8.json) に他の次数も記録する。

内部の冪の配列を 1 本減らし、共有 callback と入出力の途中版を含めた次の段階では、50 万項の指数 11.945 ms、冪 19.672 ms となった。[途中記録](results/fps_exp_stage2.json)。これは出力 helper 導入前の比較であり、最終統合結果ではない。

## 結果配列への追加

`reserve` 後に係数ごとに `emplace_back` すると、各要素で容量を判定する。50 万項について、同一バイナリで比較した。1 回以上予備実行した後、41 回の中央値。計測範囲は vector 確保と追加で、チェックサムと破棄を除く。この比較は独立した配列追加の測定で、全体の時間へ差を加減算しない。

| 方法 | 既に Montgomery 表現 | 通常剰余から変換 |
| --- | ---: | ---: |
| scalar emplace | 0.275 | 0.408 |
| 8 個ずつ insert | 0.179 | 0.232 |
| 64 個ずつ insert | 0.048 | 0.068 |
| resize + memcpy / SIMD | 0.739 | 0.755 |
| 共通 helper | 0.046 | 0.066 |

64 個の生存する `Mint` 配列にコピー・変換し、通常の `vector::insert` で追加する方法を `components/fps_output.hpp` にまとめた。vector の要素の寿命を省いたり、サイズ外へ書き込んだりしない。対数・指数・冪から使う。入力は範囲内だけを読む。

[全試行](results/fps_output.json)、[再現コード](fps_output.cpp)。`memcpy` の宛先は trivially copyable を確認したうえで `void*` に明示し、`-Wclass-memaccess` を避ける。

## 検証と再現

指数の微分恒等式、小次数の素朴な漸化式、冪の微分恒等式と定数項を照合する。1〜16,385 項、各変換の境界、入力長 0・1・出力より短い／長い入力、lazy Montgomery ワードの上限付近、指数 1・5・998244353・10^18、入力の保存を検証する。出力 helper は 0〜257 項の両表現と、非空の結果配列への追加を確認する。

移植した指数・冪は、公式の全 26・37 ケースを照合済み。radix-8 と内部配列削減後にも公式 63 ケースを照合した。途中の backend の通常・ASan/UBSan 単体も成功した。最終 helper を含めた `fps_exp_blocked.cpp`・`fps_log.cpp` の ASan/UBSan も 2 回成功した。[実行記録](results/fps_all_exp_sanitize.log)・[対象と依存ハッシュ](results/fps_all_exp_sanitize.json)。検証中の逆数／除算部品の変更は前後のハッシュ差として残し、各部品の検証と最終統合検証を組み合わせて確認する。

```sh
python3 test/benchmark/fps_exp_speed.py --baseline ce81108 --output exp.json --official-root /tmp/library-checker-problems --references /tmp/fps-reference
flock -s /tmp/cp-fps-tuning.lock g++ -std=c++23 -O2 -march=haswell -I. test/benchmark/fps_output.cpp -o /tmp/fps-output
flock -x /tmp/cp-fps-tuning.lock taskset -c 2 /tmp/fps-output
```

## 新しい結果配列のページ準備

上の配列追加の比較は、再利用されるメモリを含む。新しいプロセスごとに結果 vector を確保し、ページ準備・追加・破棄までを含めた比較を別に行った。glibc が大きな mmap を free した後に割り当て方針を変える影響を避けるため、1 試行につき新しいプロセスを使う。各版 21 回の中央値。入力生成と全要素の値の照合は計測外とし、確保・助言の syscall・追加・破棄は全て計測に含む。環境とフラグは上記と同じ。

既に Montgomery 表現の 64 語 insert。単位 ms。

| 語数 | 現行 | POPULATE_WRITE | HUGEPAGE + POPULATE_WRITE |
| ---: | ---: | ---: | ---: |
| 32,768 | 0.034 | 0.028 | 0.028 |
| 500,000 | 0.424 | 0.267 | 0.271 |
| 524,288 | 0.469 | 0.296 | 0.290 |
| 999,999 | 0.879 | 0.525 | 0.520 |
| 1,048,575 | 1.076 | 0.655 | 0.483 |

通常剰余からの変換でも、50 万語は 0.434→0.268 ms、1,048,575 語は 0.991→0.450 ms となった。32,768 語の差は約 0.006 ms のため、ページ準備は新たに書く範囲が 131072 語以上の大きな割り当てに限定する。

助言対象は実際に書く所有範囲へ内側に丸める。HUGEPAGE は 2 MiB 単位、POPULATE_WRITE は 4 KiB 単位で、最初と最後の部分ページには指定しない。50 万語と 524288 語には完全な 2 MiB ページが入らず、HUGEPAGE の syscall は無い。1,048,575 語では内側 2 MiB に指定でき、minor fault の中央値は 1030→519 になった。POPULATE_WRITE 単独では記録された fault 数は変わらず、fault 処理をまとめることで追加中の時間が減っている。

`prepare_fps_output(out,total)` で容量が増える場合だけ事前準備する。対数・逆数などの分割出力や入力の一括読み込みでは、最終サイズで一度準備し、個々の小さい追加で syscall を繰り返さない。既存要素は変更しない。未対応の助言は省略し、失敗時は通常の割り当てで継続する。この比較で改善した割り当ての処理だけで、FPS 全体の残差が全て解消したとは言えない。

[原票](results/fps_output_pages.json)、[再現ドライバー](fps_output_pages.cpp)、[再現スクリプト](fps_output_pages.py)。最終 helper のしきい値の前後、非空 vector の既存要素の保存、大きな追加の両表現は通常・ASan/UBSan の単体で確認済み。[通常](results/fps_output_pages_check.log)・[Sanitizer](results/fps_output_pages_sanitize.log)・[対象とハッシュ](results/fps_output_pages_check.json)。

```sh
python3 test/benchmark/fps_output_pages.py --output pages.json --trials 21 --cpu 2
```
