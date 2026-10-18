# FPS の追加の境界処理候補

2026-10-19 JST。比較元は `144ae32387675fa2a889ea3e8cd19b03ad5016bf`。今回の入力・出力候補は production へ採用しなかった。

共通出力を 64 語から 256/1024 語へ広げる案、vector を resize/fill してから直接書き込む案、入力ブロックの拡大、shuffle control の算術生成、lockstep の反復数変更を隔離して比較した。出力単位を広げると既存の 64 語単位の呼び出しが scalar tail へ落ちる問題もあり、全体で安定する改善を確認できなかった。

最後に既存の読み取り専用ファイルマッピングへ `MADV_POPULATE_READ` を助言する候補を確認した。入力カーソルや公開配列は変更せず、助言の対象は既存 mapping の完全に所有する内側のページだけとした。小さい処理を避けるため 4 MiB 以上の範囲に限定し、助言に失敗した場合は通常の読み取りを続ける。最速原本の破壊的な作業配列への直接入力を再現するものではない。

GCC 14.2、AMD EPYC 9V74、CPU 2 固定、`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`。全ビルドとテストは共有 flock、計測は排他 flock で分離した。1 回の予備実行後、順序を seed `20261024` でシャッフルした 41 回の中央値。通常ファイルの入出力とプロセス起動・終了を含む ms で、全出力を公式 checker に渡した。

| 処理・入力 | 比較元 ms | ページ準備 ms | 同じ反復で速かった回数 |
| --- | ---: | ---: | ---: |
| 積 max_random_00 | 15.202 | 15.175 | 21/41 |
| 積 max_random_01 | 14.731 | 14.448 | 24/41 |
| 積 all_same_00 | 14.486 | 14.388 | 21/41 |
| 積 all_same_01 | 14.667 | 14.271 | 28/41 |
| 対数 max_random_00 | 14.871 | 14.884 | 21/41 |
| 対数 max_random_01 | 14.804 | 14.635 | 24/41 |

最大時間となる `max_random_00` の改善は小さく、対数では版別の中央値が僅かに逆転した。対象範囲が 4 MiB に届かず助言を行わない `all_same_00` でも値が動くため、差のすべてをページフォールト削減へ帰属させない。この結果から共通の OS 処理を追加するだけの安定した利益とは判断せず、既存の入出力を維持した。原本との差が不可避だと示す結果でもない。

[原票](results/fps_boundary_round2/results.json)には全試行、入力・正解・checker・全 baseline header・バイナリのハッシュとコンパイルコマンドを保存した。同じ場所の `measured.py` は実測時と SHA-256 が一致するハーネス、`candidate.hpp` と `candidate.patch` は未採用の分析専用ソースである。production の `fps_io.hpp` と `fps_output.hpp` は変更していない。

```sh
mkdir -p /tmp/cp-fps-round2-base
git archive 144ae32387675fa2a889ea3e8cd19b03ad5016bf | tar -x -C /tmp/cp-fps-round2-base
python3 test/benchmark/fps_boundary_round2.py --kind input \
  --work /tmp/fps-round2-populate-large --output /tmp/fps-round2-populate-large/results.json \
  --variants baseline populate_large \
  --cases max_random_00 max_random_01 all_same_00 all_same_01 \
  --trials 41 --seed 20261024
```

前回記録と同じ Library Checker 入力が必要で、`--official-root` に配置先を指定できる。再計測は新しい出力先へ保存し、追跡済みの原票を上書きしない。
