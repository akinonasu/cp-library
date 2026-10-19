# AVX512 IFMA leaf 積の試行

`bd951dce6fdcb3ea9db4fcef342147012a8ab0c0` の `leaf_product` を、8 lane の64bit IFMAで積和して32bit Montgomeryへ戻す候補と比較した。候補は既存AVX2の約2倍を要したため不採用。productionは変更していない。

AMD EPYC 9V74、GCC 14.2、CPU 2で測定した。共通flagsは `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、候補関数だけ `target("avx512f,avx512ifma")` を付けた。compile/checkは共有flock、計時は排他flock。各sampleは1,048,576 leaf呼出しで、1回warmup後に8モードをshuffleして21回測定した。表は中央値、単位はns/leaf。

| Window件数 | AVX2 inline | IFMA 1組 | IFMA 2組 | IFMA 4組 | AVX2 call | IFMA call | runtime dispatch |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 4.078 | 8.093 | 8.312 | 8.709 | 4.586 | 8.779 | 8.461 |
| 128 | 4.019 | 8.132 | 8.075 | 8.647 | 4.711 | 8.719 | 8.739 |
| 4096 | 4.155 | 8.073 | 8.201 | 8.783 | 4.768 | 8.830 | 8.805 |

IFMAのlo52/hi52を別々に蓄積し、`lo + (hi << 52)` でexact sumを復元した。lo累積を52bitへmaskすると繰上がりを失うため、そのまま使う。実際のleaf入力はweighted Windowが `[0,P]`、残りWindowとbが `[0,P)`。各laneの積和は最大 `(8P-1)(P-1) = 7971934297386909696 < 2^63`、lo累積は `2^55` 未満、hi累積は1768以下。Montgomeryの `S+qP` も `2^64` 未満で、32bitへpackする前の値は `3P` 未満となる。既存と同じ2Pの1回減算でraw出力も一致した。両入力を独立に2P未満とする範囲は元leafの契約外。

20万Window・160万laneを既存実装、unsigned 128bit oracle、IFMAの1/2/4組、dispatchで比較しraw一致を確認した。公開inv/log/exp/sqrt/powのn=257/4097/32768/131073/500000をlazy入力で実行し、293万leaf呼出しで上記範囲を確認した。bは8wordちょうどでguard pageに接続し、最終呼出しではread-onlyにした。IFMAのWindow loadはword[1..15]だけで、公開入力へのpadding要求を増やさない。

Windowのalignasだけ32へ変更した一時コピーでも、実アドレスmod64=32のlive Windowを1万件・8万lane比較しraw一致した。候補はunaligned loadを使い、64byte整列を要求しない。この確認の追加計時は行っていない。

runtime dispatchはAVX512FとIFMAを確認してtarget関数を呼び、YMM戻り値を維持する。今回のdispatch測定は対応CPU上の分岐。`forced_fallback` は同じCPU上で旧AVX2関数本体を測ったもので、非対応CPUの速度ではない。IFMA本体だけで退行したため、全体へ組み込む案や公式fastestとのprocess全体比較には進めなかった。

[原票・source・再現手順](results/fps_avx512/leaf_ifma/README.txt)に、504 sample、shuffle順、build/check log、依存header hash、carry/overflowの根拠を保存した。
