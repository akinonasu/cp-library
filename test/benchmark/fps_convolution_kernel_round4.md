# 畳み込みの計算本体の追加比較

今回の計算本体の候補はすべて不採用とした。最大の均等長入力における最速提出との差は、これらの候補では解消していない。採用した非対称長の分割や返り値の作業領域借用とは別の比較であり、この資料の候補による production の変更はない。

基準は `7521ea8703a362aaf3ccd370188fccab86050848`、原本は未編集の [409465](https://judge.yosupo.jp/submission/409465)。AMD EPYC 9V74、GCC 14.2、CPU 2 で、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell` を使った。ビルドは共有 flock、プロセス全体の計時は排他 flock を保持した。各条件は 1 回の予備実行と、順序をシャッフルした 9 回の実測からなる。初回比較で安定した短縮を確認できなかったため、21 回の確認測定と採用用の追加検証は行っていない。

以下は公式 `max_random_00` の中央値。別の行同士は測定時刻が異なるため、同じ行の基準と候補を比較する。単位は ms。

| 候補 | 基準 | 候補 | 原本 409465 |
|---|---:|---:|---:|
| cached subtree を 64 vector へ縮小 | 11.808836 | 11.965639 | 9.728314 |
| cached subtree を 1024 vector へ拡大 | 11.808836 | 11.927933 | 9.728314 |
| tile 内の各段の長さを template 定数化 | 11.951108 | 11.929056 | 9.678859 |
| 最上段で 2 因子を同時処理する AVX512VL | 11.951108 | 12.849057 | 9.678859 |
| 4 leaf の入力と窓を追加 register に保持する C++ 版 | 12.603630 | 13.522821 | 10.830557 |
| 同じ 4 leaf の積だけを 32 register 固定 asm 化 | 12.738601 | 12.976525 | 9.897846 |
| 偶奇 Karatsuba により leaf の scalar 積を 64 から 48 へ削減 | 12.198296 | 13.262312 | 9.950765 |

固定した register への定数の移動も別途比較した。元の `forward_pair` の命令順、16 個の scratch register、leaf と次の forward を重ねる処理は保ち、stack 上の 3 個の twiddle vector だけを YMM16–18 へ移した。ループ 1 回につき 8 個、既存の `h == 4` の呼び出しでは計 32 個の L1 vector read を除いた。private header 全体を AVX512VL 対象にするだけの対照も含め、コンパイラの対象命令の変更と定数の移動を区別した。

| 公式入力 | AVX2 基準 | VL 対象への変更のみ | VL + 定数 register 化 | 原本 409465 |
|---|---:|---:|---:|---:|
| max_random_00 | 12.089415 | 12.176935 | 12.247960 | 10.037545 |
| max_random_01 | 12.258845 | 12.246117 | 12.168073 | 9.790856 |

定数 register 化と VL 対照の paired 差の中央値は、00 が +0.200890 ms、01 が +0.028302 ms。9 回中の短縮はそれぞれ 3 回、4 回で、安定した改善は観測していない。

追加 register による窓の保持は load を減らす一方、窓や係数を並べ替える費用を増やした。C++ 版は compiler が中間値を先に生成し、32 vector register を使っても多数の stack access を出した。leaf だけを固定 asm にした版と Karatsuba 版は元の forward と積を重ねる構成も失っており、演算数やメモリアクセス数の削減だけでは全体の短縮につながらなかった。この結果は試した実装の比較であり、他の演算や配置の改善が不可能だと示すものではない。

[原票と再現 pack](results/fps_convolution_kernel_round4/) に、全 620 出力の公式 checker 成功、全試行、実測した header、未編集原本、完全な command、入力・正解・checker・source・binary の SHA-256、差分、再現手順を保存した。checker は `input actual answer` の順で実行した。各実験の終了時に source・binary・依存 tree の hash 不変を確認した。`arithmetic/` と最後の `twiddle/` は別々に凍結した資料をそのまま保持している。

AVX512VL の private 候補は対応 CPU 上でだけ実行した。`twiddle/` の候補は実験用に header 全体の target を変更しており、CPUID dispatch を持たないので production へ置換できるものではない。AVX512 を持たない Library Checker の環境での高速化は主張しない。
