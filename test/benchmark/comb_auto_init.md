# 二項係数の自動初期化

2026-10-20。`math/combinatorics/comb.hpp` の include により、起動時に `fact`・`finv` を自動で前計算するようにした。既存の `init_comb()` は再計算用として残し、C 配列の型・`COMB_MAX`・`comb(n,k)` の意味を維持する。

## 初期化方法

各翻訳単位に、順序のある初期化を受ける登録用オブジェクトを置く。そのコンストラクタ内のローカル static は inline なコンストラクタで共有されるため、複数の cpp から include しても前計算は一度だけになる。全ての翻訳単位で同じ `COMB_MAX` を指定することが必要。

初期案の単純な inline 変数では、Clang 21.1.8 / `-O2` において、include 後の大域変数から取得した値が 0、`main` 内で取得した値が正しい値になるケースを確認した。その方式は採用せず、登録用オブジェクトと共通のローカル static に置き換えた。通常の `comb()` の中身には初期化の分岐を追加しない。配列は `constinit` で静的にゼロ初期化する。

参考：[C++ draft の動的初期化の順序](https://eel.is/c++draft/basic.start.dynamic)。関数の呼び出し位置を変更する修正であり、階乗・逆階乗の生成アルゴリズムは変更していない。[既存の手法比較・Library Checker の参考提出・測定](comb_factorials.md)をそのまま参照できる。起動時間やジャッジ上の速度を今回新たに測定したものではない。

## 検証

- WSL2 / Linux x86_64、GCC 15.2.0・Clang 21.1.8、`-std=c++2b -O2`。
- `init_comb()` を呼ばず、Pascal の三角形と全ての二項係数を比較。`main` 前の大域変数からの `comb`・`fact` の利用も確認。
- `COMB_MAX=0`・200・70000・既定の 502022。70000 と既定上限では全ての階乗・逆階乗を通常の剰余演算と照合。
- C 配列の型・大きさを維持。配列を書き換えた後の `init_comb()` での再計算も確認。
- 2 つの cpp を別々にコンパイルし、両方のリンク順で確認。同じ配列を参照することと、初期計算後に置いた印が重複初期化で消えないことを検証。
- GCC の `NDEBUG`・`-march=native`・`-ftrivial-auto-var-init=zero` と、AtCoder の constexpr 制限（loop 524288 / ops 2097152）で既定上限を検証。Clang の `NDEBUG` でも確認。
- Clang ASan/UBSan、既存ヘッダーとの併用、ヘッダーと提出コードの Bundle、説明内コードの実行。
- `init_comb()` を削除した yukicoder 3172 のドライバーで、公開サンプル 4 件を両コンパイラで確認。全システムケースやオンライン提出は今回の検証に含めない。

結果と対象は `results/comb_auto_init` の `selection.json`・`unit.log`・`sanitize.log`・`check.log`・`validation.json` に保存する。共有する FPS の階乗生成や二項係数 prefix sum の実装は変更していない。

リモートの `2559803` までを取り込んだ後、include 先の配置が変わった `test/unit/standalone.cpp` のみ GCC・Clang で再検証し、`integration.log` に保存する。他の検証済みソースには変更なし。説明ページの生成と二項係数 Bundle の検証は `docs.log` に保存する。

```sh
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/comb_auto_init/selection.json
flock /tmp/cp-wm-opt.lock python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/comb_auto_init/selection.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/comb_auto_init_check.py
```
