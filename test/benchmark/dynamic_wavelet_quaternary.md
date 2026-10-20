# Dynamic Wavelet Matrix の4分実装

2026-10-20。比較元は `2559803022319333756173295947577d6f5f9b2e`。
公開 API と既定のテンプレート引数を維持し、既定の `DynamicBitVector<>` を指定した場合の内部を4分実装へ変更した。
独自 BitVector を指定する二分実装と `components/dynamic_bit_vector.hpp` は変更していない。

## 参照と速度差

ユーザーから提供された [Nim 提出80164506](https://atcoder.jp/contests/abc343/submissions/80164506) と
[C++ 提出80184430](https://atcoder.jp/contests/abc343/submissions/80184430) のソースを比較した。
AtCoder の提出ページはこの環境から HTTP403 となったため、提出の計測結果・実行バイナリは取得していない。
C++ 比較元は上記コミットの同じアルゴリズムと、提供された main と同じ呼び出しを再構成したもの。

Nim の依存元は [kemuniku/cplib の c95c3c3](https://github.com/kemuniku/cplib/tree/c95c3c35f66357921a0d3bcd8cb79cbc7b13d6c3)、ライセンスは CC0-1.0。
固定した依存7ファイルと LICENSE を公式 GitHub から取得し、SHA256 を確認して展開した。
`QuadVector[32,256]`、符号なし30 bit、`quadOptimized`・`quadLeafPrefix` を再現した。
`checks:off` は提供ソースと同じく依存定義の後に置いた。依存全体の checks を無効にする別条件は主比較に使っていない。

| 差 | Nim 参照 | 変更前 C++ | 採用した処理 |
| --- | --- | --- | --- |
| 段の分岐 | 2 bit・4分 | 1 bit・2分 | 4分、奇数 bit 幅にも対応 |
| 内部木 | 32分木 | 既定8分木 | 32分木、実行時 AVX-512F／AVX2 選択 |
| 葉 | 2 bit を交互に格納、512要素ごとの prefix | 1 bit、word ごとの prefix | 上下 bit の別配列、word ごとの4種類の prefix を隣接配置 |
| 葉の更新 | 桁値移動と prefix 更新 | 1種類の個数の更新 | 変化する2種類の個数だけを更新 |
| 代入 | 最初の相違後は削除・挿入 | 移動更新 | 共通祖先の更新を共有し、分割・結合時だけ通常の削除・挿入 |
| predecessor／successor | 境界に沿って下降、候補の枝を保存 | 個数検索と kth の2回 | 候補の枝を保存する直接探索 |
| 最大・最小 | 専用の探索 | 一般の kth | `k==0` の最大・最小探索 |
| 一致個数 | `count` | 提供 main は `range_freq(v,v+1)` | `count`、差が1の範囲検索も同じ経路 |

全要素が同じ桁値の段では順位取得を省略する。代入の値が変わらない場合も不要な更新を省く。
`select` は候補が一要素になったら残りを照合し、word 内の選択には対応 CPU で BMI2 の PDEP を使う。
GCC の O3 指定は新しい内部ヘッダー内だけに限定し、読み込み後は元の設定へ戻す。
全体への target 指定や AVX-512 オプションは不要で、非対応 CPU には通常の整数演算の経路がある。

Library Checker [Point Set Range Frequency](https://judge.yosupo.jp/problem/point_set_range_frequency) の上位 AC も再確認した。
[393918](https://judge.yosupo.jp/submission/393918) は34 ms、[404514](https://judge.yosupo.jp/submission/404514) は40 msで、値別のイベントをまとめるオフライン処理だった。
任意位置への挿入・削除を持つオンライン API には直接適用しない。judge の時間と下記ローカル時間は比較しない。
参照 URL・取得日時・原文 SHA256 は [参照記録](results/dynamic_wavelet_quaternary/nim-reference-files.json) と
[Library Checker 記録](results/dynamic_wavelet_quaternary/yosupo-reference.json) に保存した。

## 測定条件

Linux x86_64、AMD EPYC9V74、CPU2固定、GCC14.2。
C++ は `-O2 -std=gnu++23 -march=native -ftrivial-auto-var-init=zero`、assert 有効。
Nim2.2.4 は AtCoder の [公開コンパイル指定](https://img.atcoder.jp/file/language-update/2025-10/language-list.html) に合わせた C++ backend、`-d:release --opt:speed --multimethods:on` などを使用した。
Nim 側の C++ backend もローカルの GCC14.2 であり、AtCoder 本番の GCC15.2 とは異なる。

計測プロセスは CPU2 に固定し、共通ロックの排他区間で実行した。コンパイル・テスト・入力生成は共有ロックを使い、計測と重ねていない。
標準入力は通常ファイルで、標準出力もファイルへ保存して全回答を確認した。
ウォームアップ後に実行順を混ぜ、各ケース7回の中央値を使う。wall と process CPU の各生値を保存する。
プロセスの終了待ちは blocking wait とし、timeout polling による短時間計測の量子化を避けた。

全6入力で $$N=Q=200000$$、10万回ずつの点代入とクエリを交互に行う。
乱数種は入力順に20261020〜20261025で、全回答を独立した top-two segment tree oracle と照合した。
小3入力の15606回答では、oracle 自体も Counter とソートによる素朴解と照合した。

| 入力 | 内容 |
| --- | --- |
| random30 | 値は1〜10億、ランダムな位置と区間 |
| duplicates16 | 値は1〜16 |
| uniform_same | 全要素7、同じ値の代入 |
| no_op_random | random30 と同じ値分布、代入は現在値 |
| clustered | 更新位置の90%が中央、残りはその近傍 |
| short | クエリの半分は長さ1、残りは長さ2〜16 |

通常の区間には全区間と長さ1も混ぜる。入力・回答 SHA256 と厳密な操作内訳は
[大入力 manifest](results/dynamic_wavelet_quaternary/cases-large-manifest.json) にある。
C++ の型は従来の `DynamicWaveletMatrix<int>`、符号付き32 bitのまま。
Nim は提供ソース通り符号なし30 bitで、この設定差も維持している。

## 最終結果

単位は ms、7回の中央値。入出力・構築を含む全体時間です。

| 入力 | 元 C++ | 最終 C++・同じ呼び出し | 提供 Nim | 最終 C++・FastInput＋count |
| --- | ---: | ---: | ---: | ---: |
| random30 | 820.642 | 481.171 | 659.897 | 456.906 |
| duplicates16 | 227.732 | 109.509 | 322.403 | 89.072 |
| uniform_same | 48.410 | 37.704 | 217.830 | 20.801 |
| no_op_random | 530.403 | 323.762 | 308.287 | 295.794 |
| clustered | 801.782 | 472.251 | 642.946 | 453.078 |
| short | 788.721 | 453.254 | 592.189 | 426.792 |

元と同じ呼び出し方でも、全6入力で元実装より22〜52%短縮した。
提供 Nim と比較すると5入力で高速で、no_op_random の全体時間は約5%遅い。
入出力を除いた API 処理時間の中央値は全6入力で Nim を下回った。
no_op_random の差は小さく、この観測から全条件での優位性は主張しない。
FastInput と count を使う別の呼び出しでは全6入力で Nim より短いが、この入出力の改善をライブラリの改善とは分けて記録する。

構築と操作を別に測った process CPU 時間は以下。読み込み・操作列保存・回答領域確保は計測前、回答出力は計測後に行う。

| 入力 | 元 C++ 構築 | 最終 C++ 構築 | 元 C++ 操作 | 最終 C++ 操作 | Nim 操作 |
| --- | ---: | ---: | ---: | ---: | ---: |
| random30 | 9.225 | 7.523 | 771.666 | 433.350 | 626.589 |
| duplicates16 | 2.997 | 6.052 | 194.824 | 73.456 | 292.039 |
| uniform_same | 1.881 | 5.771 | 20.256 | 5.627 | 191.581 |
| no_op_random | 9.162 | 7.495 | 483.588 | 277.063 | 282.321 |
| clustered | 9.210 | 7.624 | 760.925 | 425.729 | 609.680 |
| short | 9.225 | 7.540 | 735.663 | 405.616 | 560.603 |

均一な値・少数の重複値では構築の定数倍が元実装より大きい。操作全体では改善している。
全生値・warmup・実行順・入力／回答・ソース／バイナリ SHA256 は [最終記録](results/dynamic_wavelet_quaternary/final.json) に保存した。
これは手元の同一マシン比較であり、AtCoder 本番の実測や fastest の取得を示すものではない。

## 操作別の対照実験

符号付き64 bit・符号なし64 bit・符号付き31 bit・符号なし7 bitについて、access、代入、同値代入、挿入、削除、count、rank、select、kth、1境界／2境界の頻度、prev、next を比較した。
各13種類、計52ケース。小入力は N=1100・Q=1400 とし、両実装の全回答と最終配列を独立した vector oracle に照合した。
大入力は N=100000・Q=20000、3回ずつを順序を混ぜて測定し、全回答と最終配列を両実装で比較した。

全104通りの独立照合と312回の計測後照合が成功した。
操作時間の最終／元の中央値比は0.243〜0.902だったが、46/52行は50 ms未満で、3回の短い記述的な対照実験である。
全ての操作・分布で速いという保証にはしない。
構築は64 bit・31 bitで改善、7 bitではほぼ同程度だった。
確保済みノード領域は64 bitで約17.4%、31 bitで約21.2%、7 bitで約34.2%増えた。空間計算量は変わらない。
[操作別まとめ](results/dynamic_wavelet_quaternary/api/SUMMARY.md) と [全記録](results/dynamic_wavelet_quaternary/api/results.json) に条件・各生値・構築・メモリを残した。


## 候補選別

同じ CPU・コンパイラで選別した。これらは最終比較とは別の計測で、短いケースや3回の反復も含む。
[二分実装の候補](results/dynamic_wavelet_quaternary/screen_binary.json)、
[初期4分実装](results/dynamic_wavelet_quaternary/screen_quad0.json)、
[均一な段の省略](results/dynamic_wavelet_quaternary/screen_uniform_200k.json)、
[移動更新・SIMD・O3](results/dynamic_wavelet_quaternary/screen_fast_200k.json)、
[prefix 配置と葉容量](results/dynamic_wavelet_quaternary/screen_layout_blocks.json) を保存した。

葉容量256・512・1024・2048・4096を比較し、1024を採用した。
prefix の隣接配置と2種類だけの更新の組み合わせは random30／short に有効だった。
duplicates16 は元の配置が約1.7%速い選別結果もあり、この分布だけで設定を決めていない。
[最初の全6入力比較](results/dynamic_wavelet_quaternary/revision1.json) では uniform_same が元実装より遅くなることを確認した。均一な段の順位配列を作らず直接進む経路、構築時の個数の再利用、均一な word の一括生成と並べ替え省略、同値代入の順位計算省略を追加して再測定した。
二分実装の子検索だけを SIMD 化する案など、安定した改善のなかった変更は採用していない。

## 再実行

入力と回答は、同梱の独立 oracle で再生成する。

```sh
python3 test/benchmark/dynamic_wavelet_quaternary/cases.py generate \
  --directory /tmp/dwm-small --small
python3 test/benchmark/dynamic_wavelet_quaternary/cases.py generate \
  --directory /tmp/dwm-large --large
python3 test/benchmark/dynamic_wavelet_quaternary/fetch_nim_reference.py \
  --directory /tmp/dwm-nim --nim /path/to/nim-2.2.4/bin/nim --compile
```

続いて同じ baseline／最終実装と Nim を測定する。`--work` は空の出力先を指定する。

```sh
python3 test/benchmark/dynamic_wavelet_quaternary/run.py \
  --repo . --work /tmp/dwm-results \
  --driver test/benchmark/dynamic_wavelet_quaternary/abc343f.cpp \
  --small-cases /tmp/dwm-small --large-cases /tmp/dwm-large \
  --nim-reference /tmp/dwm-nim --optimized-consumer --repeats 7 --cpu 2
```

`run.py` は再構築 helper の manifest を受け取る。実測に使用した原本は `measurement_run.py` に残し、生成したソース・バイナリ・ドライバのハッシュを結果に記録した。
操作別の対照実験は、同じ最終ヘッダーを固定して実行する。

```sh
python3 test/benchmark/dynamic_wavelet_quaternary/run_api.py \
  --repo . --work /tmp/dwm-api-results \
  --header-hashes test/benchmark/results/dynamic_wavelet_quaternary/final-header-hashes.json
```

Nim の再構築 helper は固定ファイルと生成ソースの SHA256 を検査し、LICENSE とコンパイル指定も保存する。
Nim ツールチェーン自体は取得しない。

## 検証

- 既存3単体テストと、新しい4値列・4分 Wavelet Matrix の単体テストを使用。
  GCC generic／native／AVX2強制／scalar強制／ASan+UBSan と Clang19、計30通りが成功。
  [各コマンドとハッシュ](results/dynamic_wavelet_quaternary/unit-validation.json) を保存した。
- 小さい全列・全区間、符号付き／符号なし、奇数 bit 幅、64 bit の最大最小値、空区間、一要素、値域外を照合。
- word・葉・内部木の境界、分割・結合、空からの増減、解放領域の再利用、copy／move／swap と move 元の再利用を検証。
- GCC14.2・Clang19でヘッダー単独／二重 include、マクロの漏出、standalone.cpp と公開ドキュメントの例を確認。
- Library Checker の公式データは upstream commit `1814c4e5205517e368bb57a8d1127eb961cfeaae` に固定。
  GCC・Clangそれぞれで Point Set Range Frequency 25ケース、Range Kth Smallest 24ケース、計98通りが一致。
- 3件の展開済みドキュメントをコンパイル。公開ヘッダー展開後は **71147 bytes（約69.5 KiB）** で、256 KiB未満。

[検証 manifest](results/dynamic_wavelet_quaternary/judge-docs/summary.json) にコマンド・ソース・バイナリ・全入力／出力のハッシュを記録した。

作業中に main へ追加された組合せの自動初期化変更も取り込み、更新された standalone.cpp だけを GCC・Clang で再検証した。[統合記録](results/dynamic_wavelet_quaternary/rebased-integration.json) に残した。
