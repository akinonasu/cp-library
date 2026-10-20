# 非対称畳み込みのブロック化

基準は `7521ea8703a362aaf3ccd370188fccab86050848`。比較原本は Library Checker [提出 409465](https://judge.yosupo.jp/submission/409465) の未編集コード。現行と原本は短辺が 100 / 1000、長辺が 524288 の入力でも全長 $$2^{20}$$ の変換を使っていた。

短い側の変換を一度作り、長い側をブロックに分けて同じ変換を再利用する。短辺を $$m$$、長辺を $$n$$ として、変換長は $$L=8\operatorname{bitceil}(m)$$、ブロック幅は $$L-m+1$$。各ブロックの積の長さが $$L$$ 以下なので循環積から線形積が得られる。隣接ブロックの重なり $$m-1$$ 項だけを正規化して加算し、残りは生存中の `Mint` のバッチを使って `vector` へ追加する。

採用条件は短辺が 32 より大きく、長辺が 32768 以上、長辺が短辺の 32 倍以上。小さい入力や均衡した形状の既存経路はこの条件の外にある。公開 API の変更、入力の書き換え、alignment / padding の要求追加はない。

## 全体時間

GCC 14.2、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。CPU 2 に固定し、共通 lock を排他取得して各入力 21 回、各回の順番を shuffle した。各組の前に warmup を一回行い、warmup と全計時出力を公式 checker で検証した。checker の引数は `input actual answer`。全 264 出力が成功。

| 公式入力 | 基準 ms | 採用候補 ms | 原本 ms | 基準−候補の paired 中央値 ms | 改善回数 |
| --- | ---: | ---: | ---: | ---: | ---: |
| small_and_large_00 | 10.693612 | 7.046736 | 9.103171 | 3.776059 | 20/21 |
| small_and_large_01 | 10.691179 | 7.335195 | 9.121198 | 3.469933 | 20/21 |
| small_and_large_02 | 10.737838 | 6.892115 | 8.967209 | 3.776991 | 21/21 |
| small_and_large_03 | 10.873340 | 7.184321 | 9.095410 | 3.504215 | 20/21 |

中央値の差と paired 中央値は異なる統計量である。これは上記環境での process 全体時間であり、Yosupo 実機での最速獲得を示すものではない。四つの入力では同条件の未編集原本より約 1.8〜2.1 ms 速い。最大の均衡入力は対象外である。

## 候補の選別

最初は全長ゼロ初期化した出力へ各ブロックを加算し、変換長 256〜16384 を 5 回ずつ比較した。次に `4*bitceil(m)` と `8*bitceil(m)` を全四入力 21 回ずつ比較し、後者を選んだ。短辺 1000 では後者が約 0.16〜0.34 ms 速く、100 ではほぼ同等だった。

出力を重なりの加算と残りの append に分けた候補も 9 回ずつ比較した。同じ計測内のゼロ初期化版→append 版は 00: 7.168→6.965、01: 7.579→7.209、02: 7.646→7.458、03: 7.248→7.098 ms だった。最終表はこの append 版を公開経路に接続した後の独立した 21 回の確認測定である。

短辺 33 / 100 / 1000 / 4096 / 16000、長短比 16 / 32 / 64 / 128、長辺 524288 以下の 18 形状も kernel 計測で確認した。採用条件に含まれる形状では候補 / 基準の時間比が約 0.37〜0.75。これは同一 process 内の補助計測であり、上表の全体時間と混ぜていない。

## 正しさと安全性

短い側だけを通常剰余へ変換してから NTT に渡す。長い側は正規化した Montgomery 係数として渡すため、積の出力も Montgomery 係数のままであり、追加の全長変換が不要になる。逆変換結果と既存の出力はともに $$[0,p)$$、和は $$[0,2p)$$ に収まり一回の剰余補正で正規化できる。返すすべての `.v` が $$p$$ 未満であることも検証した。

`Arena` 内の独立した生存中の整数配列を使い、公開入力は `span<const Mint>` として読む。入力末尾を越える SIMD 読み取り、入力の書き換え、未初期化の padding への依存は追加しない。出力は別の `vector<Mint>` に構築し、予約容量への直接書き込みを行わない。

GCC 14 の Werror / ASan / UBSan と GCC 13 Werror で、素朴積、遅延剰余、入力の交換、同一入力、零、ブロック幅の前後、最終ブロック一項の端数を確認した。入力の先頭・末尾に guard page を置いた読み取り専用領域も照合した。GCC 14 と Clang 19 の公開 test 形式、および新 component の GCC 13 / Clang 19 / GCC 14 `-U__linux__` standalone Werror が成功した。GCC 14 sanitizer 時点から最終 header への差は include 明示と空行だけで、差分を原票へ保存している。最終 header は GCC 13 と production 形式の GCC 14 unit で再確認した。private 統合版は公式全 53 入力の checker に成功した。

`compose_reversion` 担当が数式、範囲、生存期間、const、alias、dirty padding の扱いを独立に読み取りレビューした。レビューの前提と結論は原票ディレクトリに保存した。

## 再現

[原票](results/fps_convolution_unbalanced_round4/)に全試行、checker 結果、候補・基準・原本の SHA-256、各段階の harness、patch、環境、テスト条件を保存した。初期・確認・append・最終の各 harness は raw JSON に記録した SHA-256 と完全一致する。初期 driver も当時の SHA-256 と照合済み。

```sh
python3 test/benchmark/results/fps_convolution_unbalanced_round4/reproduce.py --work /workspace/fps-unbalanced-replay
python3 test/benchmark/results/fps_convolution_unbalanced_round4/reproduce.py --work /workspace/fps-unbalanced-run --check --measure --official /path/to/library-checker-problems/convolution/convolution_mod
```

一つ目は固定基準の archive、patch 適用、原本展開、全 source / artifact SHA-256 の照合までを行う。二つ目はビルド、unit、公式全入力検証、同じ 21 回の測定を行う。コンパイルと正しさ検証は共有 lock、時間計測は排他 lock を使う。基準・候補・原本を同じ flags でビルドし、元の raw JSON は書き換えない。
