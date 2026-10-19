# FPS 合成・合成逆元の追加候補

比較元は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。今回の合成・合成逆元・射影の候補は、全体時間の安定した短縮を確認できず、すべて不採用とした。production の `fps_composition.hpp`・`fps_reversion.hpp` と公開 API は変更していない。

## 比較条件

2026-10-19 UTC、Linux x86_64、AMD EPYC 9V74、GCC 14.2.0。CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効。計時は `/tmp/cp-fps-tuning.lock` の排他 lock、コンパイル・unit は共有 lock を使う。

参照は通常合成 [409336](https://judge.yosupo.jp/submission/409336)、large 合成 [409535](https://judge.yosupo.jp/submission/409535)、通常合成逆元 [409399](https://judge.yosupo.jp/submission/409399)、large 合成逆元 [409549](https://judge.yosupo.jp/submission/409549)。未編集原本と比較元は前回の binary を、compiler・flags・header/binary SHA-256 を確認して再利用した。最後にも原本 source と binary の hash を確認し、[記録](results/fps_composition_round3/artifacts.json)へ保存した。

各問題の公式 `max_random_00`・`01`・`02` を使う。通常は 8,000 項、large は 131,072 項。ファイル入出力を含むプロセス全体を測り、予備実行 1 回後、比較元・候補・原本を試行ごとにシャッフルした各 21 回の中央値を比べる。計時外で毎回 `checker input actual answer` を実行する。各 JSON に全試行・実行順・source/header/binary/input/answer/output の hash を残した。

これは同じローカル環境での原本との比較であり、judge の実測値や順位ではない。AVX-512 候補はこの CPU でのみ評価した。Library Checker の公開環境は C2D 系であり、AVX-512 対応を前提にできない。

## 候補と結果

表は各 3 入力での「候補 / 比較元 − 1」の範囲。負数は短縮、正数は遅延であり、別セッションの絶対時間を直接比較していない。

| 候補 | large 合成 | large 合成逆元 | 判断 |
| --- | ---: | ---: | --- |
| AVX-512 の 16 係数列演算 | +0.99〜+1.06% | −0.92〜+0.06% | large 合成が全入力で退行 |
| AVX2 intrinsic の 8 係数列演算 | +1.85〜+4.11% | 未測定 | 合成の時点で退行 |
| 入出力 buffer の寿命を分けて再利用 | −0.44〜+0.82% | +0.40〜+1.16% | 合成逆元が全入力で退行 |
| 射影最終係数の生成・添字除算・反転を融合 | 対象外 | +0.10〜+0.61% | large で短縮せず |
| 外側入力融合、元の Arena 配置を維持 | −0.39〜+0.08% | 対象外 | 別 seed では全入力 +0.08〜+0.11%、通常も改善方向が反転 |
| 根 1 の列乗算を省略 | −0.55〜+0.42% | −1.47〜−0.92% | 通常合成逆元が全入力 +0.51〜+1.25% |
| 根 1 の列を `h>=2048` に限定 | −0.81〜+0.06% | −0.98〜+0.77% | 大きい列への限定で利益が安定せず |
| large 合成逆元だけ根 1 を選ぶ runtime policy | −0.13〜+0.89% | +0.16〜+2.30% | 対象を限定しても退行 |
| 同じ選択を constexpr policy に変更 | −3.27〜+0.47% | −0.31〜+0.39% | 大きい逆元の短縮が混在、通常合成も +0.96〜+1.20% |

[wide](results/fps_composition_round3/wide_columns.json)、[narrow](results/fps_composition_round3/narrow_columns.json)、[reuse](results/fps_composition_round3/reuse.json)、[fused](results/fps_composition_round3/fused_projection.json)、[outer 初回](results/fps_composition_round3/outer_reserved_initial.json)・[再測定](results/fps_composition_round3/outer_reserved.json)、[zero 全体](results/fps_composition_round3/zero_columns.json)・[列サイズ限定](results/fps_composition_round3/zero_large.json)・[runtime](results/fps_composition_round3/zero_inverse.json)・[constexpr](results/fps_composition_round3/zero_policy.json) が原票である。最後の constexpr 候補の large 合成逆元は、入力順に 30.421→30.354、30.940→30.843、30.246→30.365 ms。原本は同じ順に 29.686、30.229、29.629 ms で、差は残る。

### 実装上の確認

SIMD 候補は元の手書き AVX2 asm と同じ列だけを読み書きする。AVX-512 では runtime 判定を置き、AVX2 intrinsic 版とともに非整列 load/store を使った。[wide primitive](results/fps_composition_round3/wide_primitive.cpp)・[narrow primitive](results/fps_composition_round3/narrow_primitive.cpp) で順変換入力 $$<4p$$、逆変換入力 $$<2p$$、複数の根と列幅に対し元 asm と全 word の一致を確認した。

射影と除算の融合では、最終係数 $$a_k$$ を逆順に読み、分母 $$p-n+1+j$$ と倍率 $$p-1$$ を使って $$a_{n-1-j}/(n-1-j)$$ を直接生成する。射影の最終変換が終わってから古い Tables 領域を出力へ転用し、生成元の product・r3 とは重ならない。既存の `fps_composition_blocked` はこの候補と buffer 再利用候補で成功した。buffer 再利用を再検討する場合、直接法が入力を再読する小さい native 次数では別 buffer を保つ必要がある。今回測った公開経路の次数では直接法を使わないが、試作をそのまま一般化しない。

根 1 の候補は、$$k=0$$ で `wx=wy=1` となる Shoup 積 3 回を `reduce(2p)` に置き換え、残る $$w_z=i$$ の積だけを実行する。lazy 表現は元の asm と異なることがあるが、mod $$p$$ で一致し、順変換出力 $$<4p$$・逆変換出力 $$<2p$$ を維持する。[primitive](results/fps_composition_round3/zero_primitive.cpp) は 0、上限直前、乱数の係数、列幅 1 を含む複数の幅と $$4\le h\le8192$$ を使い、合同性・lazy 上限・対象外列の完全一致を確認した。数学・読み境界・scratch の寿命は別担当の読み取りレビューでも確認した。

prototype の通常 GCC `-Wall -Wextra -Werror` 検証結果と binary/header hash は [artifacts.json](results/fps_composition_round3/artifacts.json) にある。不採用候補のため ASan/UBSan や本番用 unit の追加は行っていない。既存の成功済み検証を今回の採用実装の証拠に流用するものでもない。

## 再現

[測定 harness](fps_composition_round3.py) は固定比較元と候補の snapshot を分ける。比較元は以前の `fps_round2_all_speed.json` にある current binary と hash が一致する `bd951dc` のものを使う。候補ごとの `.patch` は同じ名前で原票の横にある。外側入力の再測定は `outer_reserved.patch` を使う。

```sh
mkdir -p /tmp/cp-fps-round3-base
git archive bd951dce6fdcb3ea9db4fcef342147012a8ab0c0 | tar -x -C /tmp/cp-fps-round3-base
cp -a /tmp/cp-fps-round3-base /tmp/composition-candidate
patch -d /tmp/composition-candidate -p1 < test/benchmark/results/fps_composition_round3/zero_policy.patch
python3 test/benchmark/fps_composition_round3.py --candidate-root /tmp/composition-candidate --label zero_policy --trials 21 --seed 20261033
python3 test/benchmark/fps_composition_round3.py --label zero_policy --trials 21 --seed 20261034 --reuse-candidate
```

`--reuse-candidate` は既存候補の全 header・driver・binary hash を確認して再ビルドを省く。以前の原票を残す場合は JSON を別名へコピーしてから実行する。古い原票の `harness_sha256` に対応するコードは `harness_v1.source`・`harness_v2.source` に保存した。現行版は結果ファイルを含む不要なディレクトリの snapshot コピーを省き、binary 再利用を追加したもので、計時区間は同じである。

一時ディスク満杯で出力に失敗した外側入力候補の最初の測定は破棄した。空き領域を確保後、同じ binary を使って最初から 21 回を完了した原票だけを保存している。
