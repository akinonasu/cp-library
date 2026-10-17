# 合成・Power Projection・線形漸化式の調整

## 参考

2026-10-18 JST に Library Checker の AC 一覧を実行時間順に確認しました。取得時の一覧・ソース SHA-256 は [fps_tuning_reference.json](results/fps_tuning_reference.json) にあります。

| 対象・参考提出 | 表示時間 | 確認した手法 |
| --- | ---: | --- |
| [合成 #409336](https://judge.yosupo.jp/submission/409336) | 3 ms | 二変数の係数列を段階的に処理する NTT、作業領域の共有 |
| [合成 Large #409535](https://judge.yosupo.jp/submission/409535) | 35 ms | 最初の 2 段・最後の 3 段を 1 変数の積に展開、中間段の NTT を枝刈り |
| [合成逆元 #409399](https://judge.yosupo.jp/submission/409399) | 3 ms | Power Projection と Lagrange 反転、最後に冪を計算 |
| [合成逆元 Large #409549](https://judge.yosupo.jp/submission/409549) | 33 ms | Projection の最初と最後の段をまとめ、中間の変換を枝刈り |
| [線形漸化式 #405848](https://judge.yosupo.jp/submission/405848) | 68 ms | NTT 上の Bostan–Mori、逆変換と twist の融合、変換の倍化・次数縮小 |

表示時間は入出力を含む提出全体です。下記の関数単体の時間とは直接比較できません。これらの提出には専用の入出力、AVX2、起動処理の省略も含まれます。

## 採用した変更

- `compose`: 内側の最小の正の指数を $$v$$ とすると、外側の $$x^i$$ は $$iv\ge\mathrm{deg}$$ で寄与しません。不要な外側の係数を先に除き、定数や短い外側の経路を使います。内側の非零係数の正の指数がすべて $$s>1$$ の倍数なら、$$x^s$$ を 1 つの変数として圧縮して計算します。
- `power_projection`: 同じ指数の圧縮を使います。冪の非零係数も $$s$$ の倍数の位置にしか現れないため、重みはその位置だけ残します。非零の定数項も圧縮先へ残します。
- `linear_recurrence`: 係数の末尾の零を除いた次数を $$r$$、元の初期値の個数を $$d$$ とすると、$$k\ge d$$ の計算には末尾 $$r$$ 個の初期値を使い、項番号を $$k-d+r$$ にずらします。$$r=1$$ は末尾の初期値と通常の冪だけで計算します。指定された初期値の範囲は直接返します。

密な入力では指数が 1 のため、圧縮用の全体走査は行いません。API・法・返す長さは変更していません。ヘッダーに追加の ISA 要件はなく、Mac でも従来の処理を利用します。

## 測定

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定し、共通の排他ロック内でコンパイル・測定します。1 回のウォームアップ後、実行順を seed 731 でシャッフルして各 15 回の中央値を取ります。入力も seed 731 の乱数です。

フラグは `-std=c++2b -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero`。実行ファイル・出力ファイルは Linux の `/tmp` に置きます。入力生成後に AVX2 の積・加算でウォームアップし、関数呼び出しに含まれるコピー・確保・変換の準備を測ります。入力生成、チェックサム、入出力は測定外です。各試行の出力全体のチェックサムを旧版と照合します。

旧版は commit `fd41ae5df33960b93288ae98f9edac4e9fdb166f` の公開ヘッダーです。共通の FPS・NTT は測定時の同じファイルを双方に使用し、この 3 ヘッダーの変更の寄与を分けます。全ライブラリの旧 commit との比較は [fps_tuning.md](fps_tuning.md) に記録します。生データ・固定したヘッダーのハッシュは [results.json](results/fps_tuning_advanced/results.json) にあります。

係数数は 131,072、時間は ms です。`stride 4` は内側の正の指数が 4 の倍数の位置だけ非零、`valuation` は内側の最初の 16,384 係数を 0 にします。Projection の重みは乱数、漸化式の項番号は 10^18 です。

| 入力 | 旧版 | 改善後 |
| --- | ---: | ---: |
| 合成、密 | 45.477 | 45.465 |
| 合成、stride 4 | 45.382 | 9.867 |
| 合成、valuation 16,384 | 46.219 | 13.085 |
| 重み付き Projection、密 | 33.818 | 33.510 |
| 重み付き Projection、stride 4 | 33.677 | 7.236 |
| 線形漸化式、密 | 47.546 | 47.479 |
| 線形漸化式、実効次数 8 | 5.923 | 0.066 |

密な入力の差は小さく、この変更による改善とは断定しません。構造を持つ入力では計算に必要な係数数が減っています。AtCoder の実機や全入力での最速性を示す測定ではありません。

## 不採用の試行

以下の試行は最終コードに含めません。共有マシンでは同じ排他ロックを使い、他のコンパイルやテストと同時実行せず、CPU 2・GCC 15.2・native・固定 seed の入力で測りました。最初の Bostan–Mori の予備測定は各 3 回、その他の NTT の試行は各 7 回です。生の表示値は `results/fps_tuning_advanced/rejected_*.log` に残しています。

| 試行 | 131,072 係数の旧 / 候補 | 判断 |
| --- | --- | --- |
| Bostan–Mori の逆 NTT 最終段と幾何スケールを融合 | 47.371 / 48.191 ms | 小サイズも含め一貫した利益なし |
| Projection の `double_y` の最終 inverse 段と行スケールを融合 | Projection 33.527 / 33.510 ms、合成逆元 40.971 / 41.601 ms | 誤差内、一部悪化 |
| Projection の forward 最終段と積和を融合 | Projection 33.765 / 33.811 ms、合成逆元 41.061 / 41.468 ms | 利益なし |
| 32 / 64 列の `project` を固定長のレジスタ処理に展開 | 8,192 係数の合成 1.730 / 1.688 ms | 数％の観測で、大きいサイズの利益は確認できず。特殊化の追加を見送る |
| 合成逆元の log・scale・exp を改善済みの `pow` で置換 | 39.913 / 40.016 ms | 各 15 回。利益を確認できず |

最後の冪の試行は [rejected_pow.json](results/fps_tuning_advanced/rejected_pow.json) に分離しました。上位実装の中間 NTT の枝刈りは、係数配置と往復の処理全体の組み替えが必要です。今回試した局所的な融合では同じ演算数の削減にならず、密な大入力の合成・Projection の差は残っています。Bostan–Mori と合成逆元の公開ヘッダー・NTT のこれらのカーネルは維持しました。

## 検証

- 指数の公約数、非零の定数項、非 2 冪、長さを切り詰める合成、零に潰れる外側の係数を愚直な積と照合。
- 元の初期値数と実効次数が異なる漸化式を逐次計算と照合。実効次数 0 / 1、指定済みの初期値、`LLONG_MAX` の項番号を含む。
- GCC・Clang、`NDEBUG`、Clang ASan / UBSan で専用の単体テストを確認。既存の `fps_algorithms.cpp`・`power_projection.cpp` も確認。
- 合成逆元は 4,097 係数まで両方向の合成で恒等写像を確認。公式ケース・依存先全体は FPS 全体の検証記録へまとめます。

```sh
flock -x /tmp/cp-fps-tuning.lock python3 test/benchmark/fps_tuning_advanced.py --output test/benchmark/results/fps_tuning_advanced --trials 15
```
