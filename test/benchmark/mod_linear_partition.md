# Mod of Linear の等差数列分割

## 変更

基準は `6af78c247f5c9beba109116966f3a3d4e9dc5104` の `math/sqrt_floor.hpp`。同じコードを `mod_linear_partition_baseline.hpp` に保存した。

- 公開名を `partition_mod_of_linear`、ファイルを `math/partition_mod_of_linear.hpp` に変更した。
- `(dx,dy,vector<tuple<x,y,count>>)` を、名前付きの `ModLinearSegment{x,y,dx,dy,count}` の配列に変更した。
- 同じ区間を直接消費するコールバック版を追加した。
- 刻みの全探索を、既存の prefix minimum の内部走査に置き換えた。正負両方の剰余列の最小位置を求め、旧版と同じ刻み・符号・同値時の規則を保つ。
- 正負の公差を直接扱い、値の反転と最後の復元を削除した。区間長は次の折り返しまでの距離から求める。
- `long` と途中の大きな積への依存を除き、`uint64_t` の全域に対応した。公差の絶対値は法の半分以下なので `int64_t` に収まる。

探索は O(log m)、出力数を k として列挙は O(k)。配列は各区間に公差も保存するため、測定環境では 1 区間 40 byte、旧版は 24 byte。保持する必要がなければコールバック版を使う。

## 参考

[ABC429 G の補足解説](https://atcoder.jp/contests/abc429/editorial/17143) の O(√N) 個への分割を確認した。指数の和を求める [公式解説](https://atcoder.jp/contests/abc429/editorial/14242) も参照した。

分割そのものを返す Library Checker 問題は見つからなかったため、刻み探索に関連する [Min of Mod of Linear](https://judge.yosupo.jp/problem/min_of_mod_of_linear) の上位 AC を 2026-10-17 に確認した。

| 提出 | 取得時の時間 | 手法 |
| --- | ---: | --- |
| [405749](https://judge.yosupo.jp/submission/405749) | 13 ms | 32 bit のユークリッド縮小。最小値だけを返す |
| [389955](https://judge.yosupo.jp/submission/389955) / [217811](https://judge.yosupo.jp/submission/217811) | 13 / 13 ms | 405749 とソースの SHA-256 が一致 |

最小値だけの実装とは出力が異なるため、速度の直接比較はしない。今回必要な最小値の位置は、既存の `components/mod_linear_prefix_min.hpp` が返す圧縮された更新点から取得する。この部品自体は変更していない。参考提出とソースのハッシュは `results/mod_linear_partition_reference.json` にある。prefix minimum の出典は [以前の測定資料](prefix_min_of_mod_of_linear.md) を参照。

```sh
python3 test/benchmark/mod_linear_partition_reference.py --cache .verify-helper/cache/mod-linear-partition-references --output test/benchmark/results/mod_linear_partition_reference.json
```

## 測定

WSL2/Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。seed 78419、各条件 7 回、実行順はシャッフル、ウォームアップは各 1 回。測定中は他のテストを実行していない。

子プロセスの起動・入力・分割・配列の確保と解放・全区間のチェックサム・出力・終了を含む実時間。コンパイル・入力生成・比較は時間外。新旧の区間を同じ順序・同じ項目で消費し、全試行のチェックサムを照合した。旧版は範囲内で途中の積が signed 64 bit に収まる入力だけを使う。

| 入力 | 内容 |
| --- | --- |
| small | 50000 件。n=64、m は 10^9 以下、a,b は法未満の乱数 |
| medium | 1000 件。n=10^6、その他は small と同様 |
| large | 100 件。n=10^9、その他は small と同様 |
| simple | 300 件。n=10^9、m=10^9-7、a は 0,1,m-1、b=m-1 |
| uint64 | 600 件。n=m=UINT64_MAX、a は 0,1,m-1、b は 0,m-1。旧版は対象外 |

generic は `-std=c++2b -O2 -DNDEBUG`、native はさらに `-march=native -ftrivial-auto-var-init=zero`。以下は中央値、単位は ms。AtCoder 実機の速度は未測定。

| 条件 | 入力 | コールバック | 配列 | 旧版 |
| --- | --- | ---: | ---: | ---: |
| generic | small | 9.188 | 11.540 | 12.559 |
| generic | medium | 7.560 | 12.962 | 15.172 |
| generic | large | 16.177 | 54.350 | 54.045 |
| generic | simple | 2.953 | 3.205 | 21.360 |
| generic | uint64 | 2.994 | 2.946 | 対象外 |
| native | small | 9.108 | 11.512 | 12.317 |
| native | medium | 7.902 | 13.375 | 16.117 |
| native | large | 16.181 | 55.111 | 54.756 |
| native | simple | 3.645 | 3.307 | 22.014 |
| native | uint64 | 3.054 | 3.049 | 対象外 |

large の配列版は旧版と同程度、コールバック版は generic で約 3.3 倍、native で約 3.4 倍速かった。simple では旧版が刻みの全探索を行う一方、新版は探索と出力が少なく、起動・入力の時間が目立つ。これら以外の入力や AtCoder 実機での同じ倍率は保証しない。

全試行・ソースのハッシュ・入出力のハッシュ・環境は `results/mod_linear_partition_generic/results.json` と `results/mod_linear_partition_native/results.json`。ソースのハッシュは CRLF を LF に正規化して算出する。

```sh
python3 test/benchmark/mod_linear_partition.py --output test/benchmark/results/mod_linear_partition_generic
python3 test/benchmark/mod_linear_partition.py --output test/benchmark/results/mod_linear_partition_native --native
```

## 検証

GCC 15.2・Clang 21.1・GCC 13 と ASan/UBSan で確認した。小さい全入力と乱数入力で全列を復元し、各添字が一度ずつ含まれることを検査した。旧版の条件内では刻み・符号・区間順・区間の各値まで一致する。

64 bit の境界値は、区間の被覆・項数の和・始点と中点と終点・公差の合同式を 128 bit の参照計算と比較した。空の列、法 1、公差 0、正負の公差、同値時の規則、法の倍数の a,b、UINT64_MAX の項数も含む。整数平方根の境界は 2^32・2^63・UINT64_MAX 付近で確認した。

main の `2f37b76` にある Wavelet Matrix と併用した standalone テストも GCC・Clang・ASan/UBSan で確認した。分割と測定対象の依存先は変わらないため、測定の再実行はしていない。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/mod_linear_partition_selection.json
UBSAN_OPTIONS=halt_on_error=1 python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/mod_linear_partition_selection.json
```
