# 畳み込みの確保・コピー候補の追加比較

2026-10-19 UTC。固定比較元は `7521ea8703a362aaf3ccd370188fccab86050848`、未編集原本は [409465](https://judge.yosupo.jp/submission/409465)。`run_borrowed` の結果生成・入力取り込み・最後のコピーを12候補で調べた。**この資料の候補は production へ採用していない。** 追加容量1 MiBの候補だけは21回と41回で大入力の改善方向が一致したが、効果が小さく、入力側の追加容量との統合が未確認であるため採用保留とした。公開共通 `fps_output.hpp` は変更していない。

## 条件

AMD EPYC 9V74、Linux x86_64、GCC 14.2.0、CPU 2固定。全版を `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` でコンパイルした。AVX512は使わない。ビルド・検証は `/tmp/cp-fps-tuning.lock` の共有lock、計時は排他lockで分離した。

公式の通常ファイルを入力し、別の通常ファイルへ出力するchild processの起動・計算・入出力・終了を計時した。各版1回のwarmup後、反復ごとに順序をshuffleした。すべての計時出力で `checker input actual answer` を実行した。入力は最大2件（各524288×524288係数）、非対称入力 `small_and_large_03`（524288×1000）、小さい入力 `medium_02`（4184×5515）。異なるsessionの絶対値をつないで改善率を計算しない。

## 構築方法と生成コード

比較元は、生存中の `Mint` を `len+23` 個零で構築し、その内側の32 byte境界へA入力を正規化してコピーする。`Mint` の既定constructorには既にalways_inlineがある。今回のresize対照は、この変更後の文脈を測った。

- `range_import`: offset分の零→Aのraw range copy→残部のresizeで全要素の生存を開始する。Aの正規化は最初のradix8のloadへ移す。
- `range_lazy`: raw range copyは同じまま、最初のradix8で和・差のlazy上限を縮約し、入力4本それぞれの正規化を省く。
- `range_inplace`: raw range copyの直後に、その生存中の領域をSIMDで正規化し、最後に残部だけresizeする。FFTを変えない。
- `range_transform` / `range_transform_inline`: `transform_view` の値で正規化しながら構築する。前者はlambdaのcallが各係数に残り、後者はalways_inlineで除去した。しかし値を返すviewはlegacy iterator_categoryがinputとなり、GCC14のvector range insertは毎係数のcapacity比較とsize更新へ落ちた。生成コードの [call版](results/fps_convolution_allocation_round4/range_transform_codegen.txt)・[inline版](results/fps_convolution_allocation_round4/range_transform_inline_codegen.txt)を保存した。iteratorのカテゴリを偽装して最適化させる変更は使っていない。
- `batch_construct`: 64個の生存中のMintへSIMDで正規化してからrange insertし、A部分の初回零埋めを省く。
- `tail_store`: 最後の逆変換のstoreだけ結果vectorの先頭へずらし、最後のmemmoveを省く。stripe境界の未読入力を壊さないよう、8stripeの先頭vectorを退避してから残りを昇順に処理し、最後に退避した先頭を計算する。
- `reserve_only` / `huge_only`: 結果の事前POPULATEを省く対照。後者はHUGEPAGE助言だけ維持する。

range・batchの生存期間と境界、tail storeの重なりは別担当が読み取りレビューした。[記録](results/fps_convolution_allocation_round4/read_review.md)を参照。tail storeは任意overlapを扱う部品ではなく、実呼出の0〜7 Mintの左移動に限定した試作である。forced offsetのguard unitは準備したが、速度が安定しなかったため実行していない。

## 全候補の全体時間

次は同じ反復で対応させた `比較元−候補` の中央値、単位はms。正が改善。9回の探索で利益の可能性があったbatch/tailと追加容量を21回へ進め、追加容量1 MiBだけを別seed41回で確認した。小さい値や入力ごとに方向が異なる候補を、安定した高速化として扱わない。

| 候補 | 試行 | max00 | max01 | 非対称 | 小入力 | 原票 |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| [range_import](results/fps_convolution_allocation_round4/range_import.patch) | 9 | -0.190 | -0.192 | -0.377 | -0.026 | [range_resize_9.json](results/fps_convolution_allocation_round4/range_resize_9.json) |
| [resize_control](results/fps_convolution_allocation_round4/resize_control.patch) | 9 | -0.133 | -0.342 | -0.354 | -0.008 | [range_resize_9.json](results/fps_convolution_allocation_round4/range_resize_9.json) |
| [range_transform](results/fps_convolution_allocation_round4/range_transform.patch) | 9 | -0.411 | -0.656 | -0.558 | -0.014 | [transform_advice_9.json](results/fps_convolution_allocation_round4/transform_advice_9.json) |
| [reserve_only](results/fps_convolution_allocation_round4/reserve_only.patch) | 9 | -0.518 | -0.507 | -0.298 | -0.034 | [transform_advice_9.json](results/fps_convolution_allocation_round4/transform_advice_9.json) |
| [huge_only](results/fps_convolution_allocation_round4/huge_only.patch) | 9 | -0.072 | -0.129 | -0.120 | -0.057 | [transform_advice_9.json](results/fps_convolution_allocation_round4/transform_advice_9.json) |
| [range_transform_inline](results/fps_convolution_allocation_round4/range_transform_inline.patch) | 9 | -0.314 | -0.308 | -0.341 | -0.040 | [transform_inline_9.json](results/fps_convolution_allocation_round4/transform_inline_9.json) |
| [batch_construct](results/fps_convolution_allocation_round4/batch_construct.patch) | 9 | +0.214 | -0.174 | +0.192 | -0.020 | [batch_tail_9.json](results/fps_convolution_allocation_round4/batch_tail_9.json) |
| [tail_store](results/fps_convolution_allocation_round4/tail_store.patch) | 9 | +0.375 | -0.084 | -0.003 | -0.013 | [batch_tail_9.json](results/fps_convolution_allocation_round4/batch_tail_9.json) |
| [batch_construct](results/fps_convolution_allocation_round4/batch_construct.patch) | 21 | -0.073 | +0.109 | +0.041 | +0.012 | [batch_tail_21.json](results/fps_convolution_allocation_round4/batch_tail_21.json) |
| [tail_store](results/fps_convolution_allocation_round4/tail_store.patch) | 21 | +0.013 | -0.046 | +0.042 | -0.020 | [batch_tail_21.json](results/fps_convolution_allocation_round4/batch_tail_21.json) |
| [range_lazy](results/fps_convolution_allocation_round4/range_lazy.patch) | 9 | -0.198 | -0.019 | -0.208 | +0.005 | [lazy_9.json](results/fps_convolution_allocation_round4/lazy_9.json) |
| [range_inplace](results/fps_convolution_allocation_round4/range_inplace.patch) | 9 | -0.132 | -0.220 | -0.138 | +0.004 | [range_inplace_9.json](results/fps_convolution_allocation_round4/range_inplace_9.json) |
| [capacity_1m](results/fps_convolution_allocation_round4/capacity_1m.patch) | 9 | +0.009 | +0.294 | +0.125 | -0.010 | [capacity_9.json](results/fps_convolution_allocation_round4/capacity_9.json) |
| [capacity_2m](results/fps_convolution_allocation_round4/capacity_2m.patch) | 9 | -0.037 | +0.236 | -0.067 | -0.002 | [capacity_9.json](results/fps_convolution_allocation_round4/capacity_9.json) |
| [capacity_1m](results/fps_convolution_allocation_round4/capacity_1m.patch) | 21 | +0.123 | +0.217 | +0.041 | +0.013 | [capacity_21.json](results/fps_convolution_allocation_round4/capacity_21.json) |
| [capacity_2m](results/fps_convolution_allocation_round4/capacity_2m.patch) | 21 | -0.329 | +0.188 | -0.077 | -0.007 | [capacity_21.json](results/fps_convolution_allocation_round4/capacity_21.json) |
| [capacity_1m](results/fps_convolution_allocation_round4/capacity_1m.patch) | 41 | +0.146 | +0.088 | +0.114 | -0.002 | [capacity_41.json](results/fps_convolution_allocation_round4/capacity_41.json) |

## 追加容量1 MiB

実際の生存中の要素数・零初期化語数は比較元と同じとし、vectorのcapacityだけ1 MiB増やした。HUGEPAGE助言はそのowned capacityの内側、POPULATE_WRITEは実使用分の内側だけへ与える。追加reserveがbad_allocなら従来の容量で再試行する。追加capacityは返り値にも残る。これは零初期化を省略する変更ではない。

41回確認の個別中央値は次のとおり。

| 入力 | 比較元 ms | 候補 ms | 原本 ms | paired改善 ms | 勝数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| max_random_00 | 12.010 | 11.909 | 9.685 | +0.146 | 29/41 |
| max_random_01 | 11.972 | 11.918 | 9.657 | +0.088 | 25/41 |
| small_and_large_03 | 9.626 | 9.535 | 8.096 | +0.114 | 22/41 |
| medium_02 | 1.386 | 1.386 | 1.116 | −0.002 | 19/41 |

対応差の中央値を10000回bootstrapした95%区間は、順に `[+0.030,+0.279]`、`[−0.060,+0.243]`、`[−0.093,+0.234]`、`[−0.007,+0.005]` msだった。[計算記録](results/fps_convolution_allocation_round4/capacity_bootstrap.json)を保存した。最大2件の方向は21回と一致したが、後者と非対称入力では区間が0を含む。原本との差も残り、fastest到達を示す結果ではない。

既存binaryを使った別の3回ずつの [resource診断](results/fps_convolution_allocation_round4/resources.json) では、比較元/候補のpeak RSS中央値はmax00が26204/26268 KiB、max01が26264/26280 KiB、非対称が20632/20632 KiB。minor fault中央値は2284/2284、2281/1917、1678/1294だった。これは全processの値で、結果vectorだけのRSSやページ費用ではない。追加capacityが末尾のTHP被覆を変える仮説はあるが、ASLR・malloc配置・実際のhuge page利用を分離して測ってはいない。準備区間のreserve・POPULATE・assignの合計を、零埋め単独の費用と解釈しない。

追加容量1 MiB版は公式53件すべてに成功し、既存のborrowed convolution単体を通常WerrorとGCC ASan/UBSanで検証した。入力のread-onlyページ、lazy値、同一配列、零入力、出力正規化、guard page、借用領域の破棄を含む。`ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`。全private headerの前後hashも一致した。[実行条件と結果](results/fps_convolution_allocation_round4/capacity_validation.json)に残した。追加reserve失敗のfault injectionはしていない。 この保留prototypeは `owned_end` / `used_end` に `[[maybe_unused]]` が無く、対応する `MADV_*` が未定義の古いLinuxヘッダーではWerrorの未使用変数になる。再検討時には修正が必要で、実測ソースは書き換えず保存した。Mac・Clangでは変更コードが外側guardで除外される。他の不採用候補は、原票にある4入力の計時出力と読み取りレビューの範囲であり、この53件・San結果を流用しない。

## 保存と再現

[保存物](results/fps_convolution_allocation_round4/metadata.json)に、比較元・12候補の実ヘッダーgzip、全patch、driver、未編集原本の圧縮ソース、全trial・実行順・compiler/flags・source/header/binary/input/answer/checker hash、元のsetup/計時script、再構成scriptを置いた。`whole_v1.py` と `whole.py` はrawのharness hashに対応する。原票の絶対pathは当時のものを保持する。

```sh
python test/benchmark/results/fps_convolution_allocation_round4/reproduce.py \
  --repository "$PWD" --work /tmp/fps-convolution-allocation-round4-reproduce
```

固定commitから復元し、patchと実測ヘッダーのbyte一致、全記録hash、未編集原本のhashを確認する。既存のworkを上書きしない。この準備だけの再現を実行済み。`--measure` を加えると、復元した比較元・候補・原本を同じflagsで再コンパイルして計時する。`--variant` で各不採用候補も選べ、`--official` で公式入力の場所、`--trials` / `--seed` で当時のsessionを選択できる。保存後の同一計測は繰り返していない。
