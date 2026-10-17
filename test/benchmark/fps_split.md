# FPS の分割

`math/fps.hpp` を `fps/fps.hpp` に移し、Bostan–Mori、線形漸化式、合成、合成逆元、多点評価、補間、Taylor shift を通常の関数に分離しました。本体は 1,474 行から 763 行です。共通演算は `components/fps_operations.hpp`、多点評価・補間の木は `components/fps_product_tree.hpp` に置きます。NTT のアルゴリズム、閾値、AVX2・スカラー・NEON の選択は変更していません。

旧メンバー関数の互換ラッパーや旧パスのヘッダーは残していません。各機能を必要な場所で include します。Taylor shift は値を返す関数になったため、書き換える場合は `f = taylor_shift(std::move(f), c)` とします。

## 参考

2026-10-17 UTC に各問題の AC 実行時間順で先頭 3 件を取得しました。取得時の一覧・提出時間・ソース SHA-256 は `results/fps_split_reference.json` にあります。以下のソースを読み、今回の切り出し対象と比較しました。

| 問題・参考提出 | 取得時の実行時間 | 手法と比較 |
| --- | ---: | --- |
| [線形漸化式 #405848](https://judge.yosupo.jp/submission/405848) | 68 ms | NTT 上での Bostan–Mori、変換の倍化と次数の縮小。本実装も NTT 上で係数を半分にする方式を維持 |
| [合成 #409336](https://judge.yosupo.jp/submission/409336) | 3 ms | 二変数の係数列を段階的に処理する NTT。作業領域を共有。本実装も既存の NTT 合成 kernel を維持 |
| [合成逆元 #409399](https://judge.yosupo.jp/submission/409399) | 3 ms | power projection と冪。本実装も既存の projection と log・exp を利用 |
| [多点評価 #409516](https://judge.yosupo.jp/submission/409516) | 15 ms | 積の木と変換キャッシュ、小入力は Horner。本実装も同じ構成を保ち、木を内部部品へ分離 |
| [補間 #409545](https://judge.yosupo.jp/submission/409545) | 20 ms | 積の木、導関数、評価値を使う補間。本実装も木と NTT キャッシュを共用 |
| [Taylor shift #409534](https://judge.yosupo.jp/submission/409534) | 10 ms | 階乗・逆階乗と NTT、小入力用処理。本実装は既存の階乗付き畳み込みを維持 |

参考提出には AVX2、専用の入力・出力、起動処理なども含まれます。今回は API とファイル構成の整理であり、それらを移植していません。参考提出との実行時間比較や、AtCoder の実機での測定は行っていません。

## 分割前後の測定

比較元は commit `0841637ff8d5f59b4021775e812f66abef82cbdd` の `math/fps.hpp` です。スクリプトが Git から取り出し、include の解決先だけを変更してコンパイルします。共有する `components/ntt.hpp`・`math/mint.hpp` のハッシュも保存します。

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。CPU 2 に固定、各 7 回、分割前後の実行順を固定 seed でシャッフルした中央値です。ベンチマーク同士・他のテストとは逐次実行します。入力生成・結果のチェックサム計算を除き、各関数の実行を測定します。係数数・点数は 32,768、seed は 731、漸化式・Bostan–Mori の項番号は 10^18 です。Taylor shift は分割前の `f.shift(17)` と分割後の `taylor_shift(std::move(f), 17)` を比較します。

- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra`
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加

全試行で分割前後の結果のチェックサムを比較します。環境・ソースの SHA-256 と生データは `results/fps_split_generic/`・`results/fps_split_native/` に保存します。

| 操作 | generic 分割前 / 分割後 | native 分割前 / 分割後 |
| --- | ---: | ---: |
| `bostan_mori` | 10.987 / 11.140 ms | 11.882 / 12.217 ms |
| `linear_recurrence` | 11.725 / 11.412 ms | 11.408 / 11.402 ms |
| `compose` | 8.893 / 8.485 ms | 8.612 / 8.661 ms |
| `compositional_inverse` | 8.510 / 8.417 ms | 8.530 / 8.565 ms |
| `multipoint_eval` | 6.062 / 5.955 ms | 6.075 / 6.229 ms |
| `interpolate` | 8.761 / 8.997 ms | 8.874 / 8.863 ms |
| `taylor_shift` | 0.882 / 0.843 ms | 0.853 / 0.843 ms |
| `inv` | 0.522 / 0.513 ms | 0.520 / 0.523 ms |
| `exp` | 0.956 / 1.033 ms | 0.921 / 0.964 ms |

これは単一サイズでの観測結果です。最速性や全入力での同速性は主張しません。

## 検証

- 影響先は `results/fps_split_selection.json` に記録。新ヘッダーと、畳み込みなどの推移的な依存先を対象にします。
- GCC・Clang で各ヘッダー単体、既存ライブラリとの同時 include、単体テストを確認。
- 小規模な合成・漸化式・有理式の係数を愚直計算と比較。多点評価・補間・Taylor shift・合成逆元を確認。
- 内部 modint と別実装の modint の双方を確認。空入力、定数、末尾の零係数、非 2 冪サイズ、NTT 使用サイズを含みます。
- ASan / UBSan、GCC 13.4、AArch64 GCC + QEMU で確認。
- Library Checker の切り出し対象と既存の依存先を検証。yukicoder No.1307 のローカルでの公式ケース取得は認証が必要なため、対応する cyclic / ordered convolution は単体テストと認証済み CI で確認します。
- `fps/` の探索・依存先選択に関する CI テスト、API 表・Bundle・公開ページを確認。

## 再現

```sh
python3 test/benchmark/fps_split_reference.py --cache .verify-helper/cache/fps-split-references --output test/benchmark/results/fps_split_reference.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/fps_split_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/results/fps_split_selection.json
python3 test/benchmark/fps_split.py --output test/benchmark/results/fps_split_generic
python3 test/benchmark/fps_split.py --native --output test/benchmark/results/fps_split_native
```
