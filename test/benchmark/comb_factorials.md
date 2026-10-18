# 二項係数の階乗生成

`init_comb()` と、既定の `mint` を使う二項係数 prefix sum の階乗・逆階乗生成を `components/fps_factorials.hpp` に共通化した。FPS 本体の include は不要。`init_comb()`・`COMB_MAX`・`fact`・`finv`・`comb` の使い方と配列の型は維持する。別の modint を使う prefix sum は従来のスカラー処理を使う。

共通部品に `fps_mint*` を受け取る制約付きテンプレートを追加した。内部ワード版の処理は変更せず、同じ Montgomery 表現の領域を直接渡す。元の `factorials(nullptr,nullptr,0)` も維持する。256 要素以上で AVX2 が利用できれば 32 本の積の列を並行生成し、その他の環境は既存のスカラー処理を使う。追加の配列やコピーは不要。

## 参考提出

Library Checker の実行時間順の上位 AC を確認した。[取得結果とソースの SHA-256](results/comb_factorials_reference.json)を保存している。

| 問題・提出 | 取得時の時間 | 確認した方法 |
| --- | ---: | --- |
| Binomial Coefficient (Prime Mod) [223248 / sortA0329](https://judge.yosupo.jp/submission/223248) | 180 ms | 静的・動的 modint を分岐。階乗を順方向に求め、末尾を反転して逆階乗を逆方向に求める |
| 同 [196412](https://judge.yosupo.jp/submission/196412)、[196634](https://judge.yosupo.jp/submission/196634) | 190 ms | 同様の逐次積によるテーブル生成 |
| Polynomial Taylor Shift [409534 / Aiyiyi](https://judge.yosupo.jp/submission/409534) | 10 ms | 独立した積の列と転置。開始値の階乗には生成済みの表を使用 |
| 同 [409734](https://judge.yosupo.jp/submission/409734)、[409733](https://judge.yosupo.jp/submission/409733) | 20 ms | 同じ系列の SIMD 実装 |

元の `init_comb()` は二項係数の上位実装と同じ逐次積だった。今回流用する FPS の共通部品は独立した 32 本の積を SIMD で生成し、区間ごとの積を接続する。Taylor shift の専用実装と異なり、生成済みの開始値の表は必要ない。法は 998244353 に固定したままであり、任意の素数を扱う Library Checker の問題全体の速度とは比較しない。

## 測定

WSL2 / Linux x86_64、Core i7-13700F、CPU 2 固定、GCC 15.2。`-std=gnu++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero -DATCODER -DONLINE_JUDGE` と、AtCoder の constexpr 上限を指定した。各条件 9 回、実行順の seed は 20261019。

各試行は新しいプロセスで、表を 1 回生成した後に繰り返し生成し、1 回あたりの時間を測る。繰り返し数は `max(1,min(20000,10000000/(N+1)))`。時間には階乗・逆階乗の全要素の書き込みを含み、prefix では 2 の冪の生成も含む。確保済み領域を再利用し、起動・入出力・チェックサムは除く。表の全要素を通常の剰余表現に戻してチェックサムを計算し、変更前後で一致を確認した。

| N | 階乗・逆階乗 変更前 ms | 変更後 ms | prefix 内部表 変更前 ms | 変更後 ms |
| ---: | ---: | ---: | ---: | ---: |
| 0 | 0.000042 | 0.000040 | 0.000045 | 0.000044 |
| 254 | 0.001688 | 0.001207 | 0.001767 | 0.001466 |
| 255 | 0.001759 | 0.000619 | 0.001775 | 0.000836 |
| 256 | 0.001776 | 0.000633 | 0.001776 | 0.000839 |
| 4096 | 0.027127 | 0.005376 | 0.027405 | 0.008721 |
| 502022 | 3.306147 | 0.622083 | 3.358155 | 1.035006 |
| 1048576 | 6.873575 | 1.310639 | 6.980577 | 2.227865 |
| 4194304 | 28.101356 | 5.338755 | 28.038274 | 9.064137 |

[native の全試行とソース](results/comb_factorials.json)、[`-march=native` を外した 5 回の測定](results/comb_factorials_generic.json)。後者も同じ AVX2 対応 CPU で、実行時判定により SIMD を使う。Mac の速度や AtCoder 実機での速度は測定していない。prefix の値は内部テーブルの生成時間で、平方分割表・Mo 法・問い合わせ全体の時間ではない。

## 検証

- 階乗・逆階乗を通常の 64 bit 剰余演算の結果と照合。0、SIMD の境界、非整列の出力先、余りの区間、1048576 要素、再初期化を含む。
- `COMB_MAX=0`、既存の Pascal の三角形との全比較、既存の prefix sum の総当たり・乱択テストを GCC / Clang で実行。別の素数 mod の modint も確認。
- 新規の階乗生成テストと prefix sum のテストを Clang の ASan / UBSan で実行。`NDEBUG` と AtCoder の constexpr 上限でも確認。
- yukicoder 3172 と 2206 の公開サンプルを、対象の全 3 検証コードについて GCC / Clang で確認。システムテストの取得はローカルの認証がないため実行できず、公式全ケースの検証済みとはしていない。
- ドキュメントの関数表と展開コードを確認。

[検証ログ](results/comb_factorials/check.log)、[検証対象とソースの記録](results/comb_factorials/check.json)、[ドキュメント検証](results/comb_factorials/docs.log)。

```sh
python3 test/benchmark/comb_factorials.py --baseline test/benchmark/results/comb_factorials.json --output /tmp/comb-factorials.json
python3 test/benchmark/comb_factorials.py --generic --trials 5 --baseline test/benchmark/results/comb_factorials.json --output /tmp/comb-factorials-generic.json
python3 test/benchmark/comb_factorials.py --references --output /tmp/comb-factorials-reference.json
```

初回の比較元のソースは結果 JSON の `baseline_sources` に保存している。CPU 番号は `--cpu` で指定できる。測定は `/tmp/cp-wm-opt.lock` で他の検証・測定と逐次実行した。
