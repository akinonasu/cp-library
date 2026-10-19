# NTT 最上段・Window 配置の追加比較

`bd951dce6fdcb3ea9db4fcef342147012a8ab0c0` から、AVX-512 の16係数処理、32本のYMMレジスタを使えるEVEX256、AVX2でのscale分配、Windowの縮小を試した。**安定したプロセス全体の改善を確認できず、productionは変更していない。** IFMA leafの別比較も約2倍遅く、[別資料](fps_avx512_leaf_ifma.md)に記録した。

測定はAMD EPYC 9V74、GCC 14.2、CPU 2固定。共通flagsは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。compile/checkには `/tmp/cp-fps-tuning.lock` の共有ロック、計時には排他ロックを使った。1回の予備実行後、版の実行順をshuffleした。原票にseed、全sample、コンパイルコマンドとhashを保存した。

このCPUはAVX512F/DQ/IFMA/BW/VLに対応する。候補の512命令は専用target関数内に限定し、AVX512F/DQ/VLを確認してから呼ぶ。非対応CPUでは従来のAVX2処理を使う設計とした。ただし今回の計時は対応CPU上だけであり、非対応CPUの性能や公式judgeの性能ではない。公式Yosupo helpの `c2d-highcpu-8` はMilan世代の可能性があり、512のローカル改善だけではjudge最速の根拠にならない。

## 候補

| 保存名 | 変更 |
| --- | --- |
| `evex` | 既存256bit最上段をAVX512F/DQ/VL targetの関数へ複写し、追加YMMレジスタを利用可能にする |
| `wide` | inverse top2/top4を16係数ずつ処理する512bit Shoup積へ変更。64byte load/storeはunaligned。hが32以上かつ偶数のときだけdispatch |
| `reordered` | top4のz乗算とscaleを分配し、odd列を独立に計算。Both出力でShoup積を5回から4回へ減らす |
| `reordered_late` | even列は従来の式を保ち、odd列の生存区間を短くする |
| `reordered_late2` | scale後のodd列では不要になる `low(diff(p0,p1))` のlowも除去する |
| `window32` | 17語のlive領域を維持し、align64/sizeof128をalign32/sizeof96へ変更 |
| `window16` | 先頭odd積の36..67byte読取りを32..63byteのaligned loadと64bit shiftへ置換。不要な17語目を除き、align64/sizeof64へ縮小。leaf_squareも同じ変更 |

`wide_kernel` は最初の計算本体比較時の版で、hの偶数確認をまだ入れていなかった。実際の呼出しは2冪であり、比較した呼出し範囲は同じ。プロセス全体の測定と最終境界検証は偶数確認を追加した `wide` で行った。両方のpatchと個別hashを保存している。

scale分配ではShoup積の入力をuint32範囲内に保つ。`diff(p0,p1)` は4P未満であり、Shoup積の結果は2P未満なので、その前の2Pへの正規化を省ける。Half出力では積の回数は従来と同じ3回で、主な変化は依存するz→scaleを並列の2積へ変える点になる。Both出力の乗算削減だけで、native inverseの大きな改善を期待することはできない。

## 計算本体の比較

`fps_tuning_basic.cpp` の入力生成後から公開APIの結果生成までを計時した。I/Oを含まず、演算に必要な確保・正規化・結果vector生成を含む。すべて21試行の中央値、単位はms。各行のbaselineと候補を同じrun内で比較する。run間のbaselineの違いを候補の効果として扱わない。

| run / 500000項 | inv baseline→候補 | sqrt baseline→候補 | log baseline→候補 | exp baseline→候補 |
| --- | ---: | ---: | ---: | ---: |
| EVEX256 | 7.902→7.852 | 7.549→7.489 | 9.272→9.146 | 11.789→12.051 |
| wide512 | 7.108→7.110 | 7.060→7.013 | 9.077→8.918 | 11.330→11.446 |
| scale分配最終版 | 7.543→7.532 | 7.458→7.443 | 8.989→9.033 | 11.814→11.769 |
| Window align32 | 7.235→7.315 | 7.374→7.246 | 8.996→9.053 | 11.210→11.550 |
| Window 16語 | 7.321→7.319 | 7.266→7.336 | 8.879→8.866 | 11.600→11.680 |

262144項も同じrunで比較した。例えばscale分配最終版はsqrtで3.540→3.396msだった一方、expは5.704→6.087msへ退行した。Windowの縮小も複数入力・複数操作で安定した利益にならなかった。powも後半3候補で比較し、採用に足る利益は見つからなかった。

## プロセス全体の比較

公式入力を通常ファイルから読み、通常ファイルへ出力した。各実行の出力を `checker input actual answer` の順に検証した。参照は未編集の原本で、同じflagsで再ビルドした。

| wide512・21試行 | baseline ms | 候補 ms | 参照409452 ms |
| --- | ---: | ---: | ---: |
| log/max_random_00 | 14.694 | 14.453 | 12.951 |
| log/max_random_01 | 14.335 | 14.279 | 12.899 |

wide512の同一trial番号におけるbaseline−候補の中央値は、それぞれ+0.185ms、+0.001ms。10000回のbootstrapによる95%区間は−0.067..+0.499ms、−0.130..+0.359msで、21回から安定利益は確認できない。単純な中央値の差だけで採用しなかった。

| scale分配最終版・9試行 | baseline ms | 候補 ms | 原本 ms |
| --- | ---: | ---: | ---: |
| convolution/max_random_00 | 14.722 | 14.741 | 11.512（409465） |
| convolution/max_random_01 | 14.636 | 14.785 | 10.908（409465） |
| inv/max_random_00 | 12.399 | 12.507 | 10.813（409370） |
| inv/max_random_01 | 12.774 | 12.532 | 10.962（409370） |
| sqrt/max_random_01 | 12.743 | 12.793 | 10.743（409728） |

convolutionの専用NTTは今回変更したblocked_polyの最上段を呼ばず、対照の意味で測った。invの片方だけの改善は複数caseで安定した短縮ではなく、共通部へ採用しなかった。

sqrtのmax_random_00は定数項192279220が平方非剰余で、baselineと候補は正しく `-1` を出力した。原本409728だけが予備実行でSIGABRTとなったため、このcaseの参照時間は採らなかった。原本は修正せず、従来の正式比較でも使っているmax_random_01を別runで比較した。失敗したrunのtrace、入力・ソース・バイナリhashと無出力を別原票に残した。

## 境界確認と保存物

inverse top4はh=4/16/32/64/256/1024/16384、全Half指定、scale=0/1/P−1/R²で従来式とのraw一致を確認した。top2は同様の範囲に加え、奇数hのHalf指定でfallbackを確認した。16語canaryも一致した。Window16は20万件のproduct/squareで従来式とraw一致し、live Windowの64byteとbの32byteをPROT_NONEの直前に置いて、末尾を越えるloadがないことを確認した。新しい算術を使わないfallbackの式は維持した。これらは候補の確認であり、未採用のproduction変更に対する検証とは扱わない。

[保存物と再現手順](results/fps_avx512/top/README.txt)にはpatch8種、実測harness、全sample、参照4本の圧縮ソース、header hash、probeを含む。基準commitから別workspaceへ9treeを再構成し、それぞれ175headerが実測snapshotと完全一致した。原本4本も既存の取得記録とSHA-256が一致した。候補ごとのバイナリ・変更header hashと原票の対応も監査した。

今回の不採用はNTTの高速化が不可能であることを意味しない。少なくとも、512bit幅、追加レジスタ、乗算数の局所削減、作業配列のサイズ削減だけでは安定した公開API全体の短縮にならなかった。[畳み込みの段階別監査](fps_convolution_phases.md)で確認したAPIの変換・コピー・I/Oの差を、未移植のNTT演算と混同しない。
