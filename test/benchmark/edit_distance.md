# 編集距離のベンチマーク

```sh
g++ -std=c++23 -O2 -DNDEBUG -I . test/benchmark/edit_distance.cpp -o /tmp/edit-distance-bench
/tmp/edit-distance-bench library random26 200000 5
/tmp/edit-distance-bench scalar64 random26 200000 5
/tmp/edit-distance-bench avx2 random26 200000 5
/tmp/edit-distance-bench avx512 random26 200000 5
```

引数は実装、入力の種類、文字列長、繰り返し回数です。単位は ms、中央値を出力します。`avx2`・`avx512` は対応する x86_64 CPU で実行します。`library` は共通部分の除去・部分列判定・共通文字の判定・処理量を制限した wavefront 探索も含み、CPU に応じて更新処理を選びます。

入力は `random2`・`random4`・`random26`・`random256`、同一文字列の `identical`、1・100・1,000 箇所を置換する `near1`・`near100`・`near1000`、挿入と削除の `indel100`、共通文字のない `disjoint`、大きなブロックを逆順にする `blocks`、周期列をずらす `periodic`、短い方を 100 文字にする `unbalanced` です。乱数 seed は 1234 です。

両方 200,000 文字の場合、更新ループの反復回数は以下です。

| 更新処理 | 1 回の位置数 | 反復回数 |
| --- | ---: | ---: |
| `scalar64` | 64 | 625,000,000 |
| `avx2` | 256 | 156,400,000 |
| `avx512` | 512 | 78,200,000 |

状態と文字マスクを 64 byte 境界に整列し、8 ワード単位に埋めて端数処理をなくしています。文字は整数列に変換せず、256 個のポインターからマスクを引きます。SIMD の加算は生成・伝播マスクで桁上がりを一括処理します。AVX2 の生成マスクは符号反転と比較を使わず、AVX-512 の論理式は ternary logic 命令にまとめています。GCC の `O3` は SIMD 関数だけに適用し、呼び出し側の設定は変更しません。

比較用に、ブロックごとに境界差分を渡す `block`、独立した桁上がりを持つ `add128`、ループを展開する `add128_unroll2`・`unroll2`・`unroll4`、論理式を変える `d0_block`・`d0_add128`、128 bit ブロックの `block128`・`d0_block128`、状態を交互に置く `interleaved`、通常の DP の `scalar`、wavefront 探索の `wave1`・`wave4`・`wave16` を残しています。wavefront 単体は処理量の上限で打ち切ると `-1` を返します。

2026-10-15 の macOS arm64 / GCC 15.2 では、26 種類・100,000 文字同士の更新処理で `block` 約 417 ms、`add128` 約 223 ms を記録しました。AVX2・AVX-512 の実機時間はこの環境では計測できていません。

両 SIMD 処理は、通常の DP との比較、20 万文字同士、生成・伝播・非伝播の組み合わせ全 6,561 通り、ASan・UBSan で検証しています。AVX 非対応の環境でも、[SIMDe](https://github.com/simd-everywhere/simde) を使って同じ関数本体を検証できます。ライブラリ本体に SIMDe は必要ありません。

```sh
git clone --depth 1 https://github.com/simd-everywhere/simde.git /tmp/simde
python3 test/verify_edit_distance_simd.py --simde /tmp/simde --cxx g++
python3 test/verify_edit_distance_simd.py --simde /tmp/simde --cxx clang++ --sanitize
```

ビット並列法は [Myers の論文](https://www.gersteinlab.org/courses/452/09-spring/pdf/Myers.pdf)、編集回数ごとの探索は [wavefront alignment](https://github.com/smarco/WFA-paper) を参考にしています。

## 共通端の除去のローカル比較

2026-10-18 の Linux x86_64 / AMD EPYC 9V74 / GCC 14.2.0 で、`bce99d05caeea138a0598eab170f1110c1c6f786` の byte loop と、8 byte ごとの `memcpy` / XOR による prefix・suffix 除去を比較しました。API、wavefront の extend と budget、SIMD dispatch は変更していません。

seed 1234、各入力で AB/BA/AB/BA の4組です。回数は測定前に固定し、再測定や閾値調整はしていません。CPU0 に固定し、`-std=c++23 -O2 -DNDEBUG` と、さらに `-march=native` を指定するビルドを分けました。generic でも既存の runtime AVX dispatch は有効です。入力生成・独立 DP・事前の正解確認1回を除き、API 全体と同じ wrapper・checksum の時間を含みます。別 translation unit の noinline/noclone wrapper を使い、LTO は使いません。

数値は4組の baseline/candidate 比の中央値です。1より大きい場合に候補が速くなります。

| 入力 | generic | native |
| --- | ---: | ---: |
| identical200k | 8.640 | 8.061 |
| last_sub200k | 8.006 | 8.057 |
| first_sub200k | 6.746 | 6.309 |
| middle_sub200k | 7.287 | 7.156 |
| prefix199999_200000 | 8.209 | 8.081 |
| random26_4096 | 1.003 | 1.019 |
| disjoint200k | 1.006 | 1.011 |
| short_corpus | 1.083 | 1.051 |

共通端の長い5入力では明確な改善があります。短い入力の合計は全組で改善しましたが、個々の長さや形の退行が合計で隠れる可能性はあります。random26 と disjoint はばらつきがあり、改善とは扱いません。長い入力は同じ1組を反復する warm-cache の API workload です。全体の編集距離の性能向上を示すものではありません。

random26 は両端を異なる文字にした独立4096文字です。短い corpus は長さ0..17・63..65、view offset0..7、置換・長さ違い・NUL・上位byteを含む1344組です。これらは通常の DP、20万文字の例は既知の正解と照合し、全呼び出しを checksum に含めています。

全試行・固定条件・集計・環境は遅かった組や揺れも含めてローカルに保存し、リポジトリには追加していません。random26 の4組の比は generic が1.093/0.960/1.045/0.937、native が0.973/1.030/1.008/1.039です。disjoint は generic が0.962/1.045/1.046/0.966、native が0.988/0.996/1.025/1.030です。

既存 unit・standalone と UBSan は通過しました。別の境界検証では空 view、全256byte、offset0..7、保護ページの前後、prefix/suffix の正確な一致を828,387回確認しました。通常の ASan+UBSan は LSan の ptrace 非対応で失敗し、`detect_leaks=0` の別診断のみ通過しました。通常 sanitizer 成功や leak 検証とは扱いません。

GCC15.2/C++23 の対象比較、実際の macOS、通常 sanitizer、judge/CI は未実施です。準備した `edit_distance.py` は同じ8入力・4組を generic/native で1回ずつ測り、正確な commit、GCC15.2.0、固定 corpus、全 checksum を検証します。対象 flags は `-O2 -std=gnu++23 -ftrivial-auto-var-init=zero -fmodules -DONLINE_JUDGE -DATCODER -fno-lto -Wall -Wextra -Werror` に generic の `-march=x86-64 -mtune=generic` または native の `-march=native` を加え、assertion を有効にします。両者に同じ候補側 driver を使い、比較する header だけを各 commit から取得します。

AOJ DPL_1_E の上限は1000文字で、この20万文字の API 比較から judge 最速とは主張しません。直接対応する Yosupo のランキング比較は確認できていません。対象最新版の必要なテストと CI、Mac 互換性の確認前には採用しません。
