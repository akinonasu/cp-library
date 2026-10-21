# Cartesian Tree

2026-10-21。`tree/cartesian_tree.hpp` に、比較関数・同値の優先方向を指定できる Cartesian Tree の構築を追加しました。

## 参考提出と手法

Library Checker の [Cartesian Tree](https://judge.yosupo.jp/problem/cartesian_tree) について、公式 API の AC 実行時間順で先頭 3 件を取得してソースを確認しました。取得時刻・一覧・ソースの SHA-256 は `results/cartesian_tree/reference.json` に保存しています。

| 提出 | 取得時の時間 | 構築方法 |
| --- | ---: | --- |
| [215554 / oldyan](https://judge.yosupo.jp/submission/215554) | 21 ms | 添字・値・右の子を動的バッファの単調スタックに保存し、取り除く際に子の情報を確定 |
| [215553 / oldyan](https://judge.yosupo.jp/submission/215553) | 21 ms | 同じ手法で固定長バッファを使用 |
| [405124 / Rohan_Kapri](https://judge.yosupo.jp/submission/405124) | 22 ms | 動的バッファと子の確定処理を使う同系統の構築 |

いずれも線形時間で、入力の読み取りを値の取得関数に組み込み、コールバックで親を記録しています。本実装は既存の列から根・親・左右の子を返す API とし、親配列で右の経路をたどります。明示的なスタックの代わりに出力配列を使うため、出力以外の追加空間は定数です。

[公式解答](https://github.com/yosupo06/library-checker-problems/blob/master/tree/cartesian_tree/sol/correct.cpp)の親ポインタによる構築も確認しました。本実装では取り除いた経路の最後の頂点だけを新しい左の子へ付け替えます。最小値・最大値などの番兵を必要とせず、同じ値の左優先・右優先をそれぞれ 1 回の比較で判定します。

比較した手法はいずれも各頂点を高々 1 回取り除く単調スタック法です。公開提出の 21〜22 ms は入出力を含む judge 上の値で、本実装の実測値ではありません。今回、実行時間の比較測定や公開 judge への提出は行っていません。

## 検証

- 値が 0・1・2 の長さ 0〜8 の全列 9,841 個、および長さ 1〜8 の全順列 46,233 個。
- それぞれについて最小・最大の比較と、同値の左優先・右優先の 4 通りを確認しました。区間ごとに優先する値を全探索して再分割する独立した参照実装と、根・親・左右の子を比較しています。
- seed `20261021` の重複を含むランダムな列 1,000 個で、構造と入力の非変更を確認しました。
- 空列、整数の最小値・最大値、`array`・C 配列・`span`・`deque`・`vector<bool>`・文字列・iota view、コピー不能な要素と比較関数。
- 長さ 1,000,000 の昇順・降順・ランダム順列・全要素同値で、中間順走査、親子関係、ヒープ条件、比較回数が `2n` 以下であることを確認しました。走査と構築は再帰を使いません。
- GCC 14.2・Clang 19 の単体テスト、Clang の ASan・UBSan、単独・二重 include、テンプレートマクロの非混入。
- GCC・Clang の両方で公式 25 ケースの親列が公式解答と一致しました。根自身を親とする出力規約は検証コード側で変換しています。
- `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror` で検証コードをコンパイルしました。ドキュメント例も、未使用変数の警告だけを除外して同じ設定で確認しています。

実行環境は Linux x86_64 / AMD EPYC 9V74 です。検証結果・公式入力のハッシュ・利用した生成器のコミット・実装のハッシュは `results/cartesian_tree/validation.json` に保存しています。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/cartesian_tree_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/cartesian_tree_selection.json
oj-verify run test/cartesian_tree.test.cpp
```
