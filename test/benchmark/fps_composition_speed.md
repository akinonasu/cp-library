# FPS 合成・合成逆元・射影

比較元は `ce81108`。2026-10-19（Asia/Tokyo）に測定した。公開 API、入力の保存、出力次数の指定、汎用係数型と他環境のフォールバックを維持する。

## 参照と不足していた部分

| 問題 | 参照提出 | 原本 SHA-256 |
| --- | --- | --- |
| `composition_of_formal_power_series` | [409336](https://judge.yosupo.jp/submission/409336) | `766d2101d5e2c619f9fa3d73e57099871f112b91345f463fa2e17fb6c6bb6188` |
| `composition_of_formal_power_series_large` | [409535](https://judge.yosupo.jp/submission/409535) | `1829103fcfeb5f979b111354a58a95caeedb5b44615eb24197e111fc10b126c9` |
| `compositional_inverse_of_formal_power_series` | [409399](https://judge.yosupo.jp/submission/409399) | `51f5819987ba31df921f041d640216b404593ad5b293732aa519ecf9e9b19145` |
| `compositional_inverse_of_formal_power_series_large` | [409549](https://judge.yosupo.jp/submission/409549) | `266ebfa429c5413f815d504684bb4ec69fcbf8bf743a777873b76486f82f5edf` |

旧合成は Kinoshita–Li 法の高位の手順を持つが、零となる列を通常の変換と fill で処理し、一般の葉の積を使っていた。原本は専用の列変換 assembly、零の列を飛ばす pruned transform、最初の 2 段と最後の 3 段の一変数への分解、転置した葉の組の積をまとめて使う。これらを `components/fps_composition.hpp` へ移植した。

合成は外側係数に線形なので、その入力を Montgomery 表現のまま扱い、結果も同じ表現で格納する。内側係数は普通剰余へ変換する。密な入力では `outer.pre(deg)`・`inner.pre(deg)` のコピーを省き、元の入力を直接読む。係数ごとの `Mint::raw`・`Mint::val` の target 境界を越える呼び出しも避ける。

旧合成逆元は射影の後に複数の FPS 配列を生成し、対数と指数を公開 API から別々に計算していた。`components/fps_reversion.hpp` は射影、添字による除算、反転、冪を同じ Arena で連結する。冪の定数項に Montgomery の倍率を含め、結果を直接返す。

参照の射影は単一係数の抽出に特化している。一般の重みについては分子を最初の 2 段に渡す処理を追加し、それ以降の専用射影を共用する。定数項の除去と最後の二項変換、疎な次数の圧縮、単項式と小さい出力の処理は既存のものを使う。

## 計算部分の比較

Linux x86_64、AMD EPYC 9V74、GCC 14.2.0、CPU 2 固定。`-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -ftrivial-auto-var-init=zero -march=haswell`、assert 有効。入力生成とチェックサムを除き、関数内部の確保・初期化・係数変換を含む。予備実行 1 回後、seed 20261019 で順番をシャッフルした 9 回の中央値、単位 ms。計時中は共通ロックで他のコンパイル・検証を止め、チェックサムを比較元と照合する。

以下は各専用処理を接続した時点の記録。後続の共通 NTT と一括入出力の更新前なので、最終版のプロセス全体は総合比較を参照する。全 snapshot のヘッダー・バイナリ・入力のハッシュと全試行は [記録](results/fps_composition_speed.json) に残す。

| 処理 | 項数 | 比較元 | 専用処理 |
| --- | ---: | ---: | ---: |
| compose | 32,768 | 10.689 | 6.152 |
| compose | 131,072 | 52.948 | 31.259 |
| inverse | 32,768 | 9.823 | 6.101 |
| inverse | 131,072 | 48.313 | 28.481 |
| projection | 32,768 | 8.229 | 5.006 |
| projection | 131,072 | 40.816 | 24.307 |
| projection_weighted | 32,768 | 8.229 | 5.507 |
| projection_weighted | 131,072 | 41.229 | 28.077 |

## 不採用の候補

原本のコンパイル設定に合わせて、新しい合成部品だけ `O3` から `O2` に戻す候補も測定した。131,072 項で合成 31.362→43.324 ms、合成逆元 28.540→39.276 ms、一般重みの射影 27.047→37.756 ms と悪化したため採用しない。他の部品と最適化指定が異なることも含む比較であり、原本一般の `O2` が遅いとする結論ではない。[候補の全試行・実行コード・ハッシュ](results/fps_composition_options.json) を保存した。

## 安全性と検証

Linux x86_64 / GCC と AVX2 の実行時判定を組み合わせる。作業領域は既存の RAII 確保を使い、mmap が失敗した場合には整列した通常の配列へ戻る。変換表の未使用部分は明示的に初期化する。入力に余分な padding を要求せず、vector のサイズ外には書かない。

既存の `fps_tuning_advanced`・`power_projection` と新しい `fps_composition_blocked` の単体テストを確認した。新しいテストは次数 33〜4097 の境界、入力長と出力次数の違い、短い入力の合成逆元、合成と重み付き射影の双対性、入力の保存、`0xfdfdfdfd` で埋めた作業領域への独立性を確認する。通常と ASan/UBSan で検証する。最終の共通 NTT 更新後の再確認は総合検証に含める。

合成・合成逆元の通常版と large 版の公式 110 ケースを確認した。[公式ケース記録](results/fps_composition_judge.json) を保存する。最終版の checker 検証と速度比較は総合検証にまとめる。

## 再現

```sh
python3 test/benchmark/fps_composition_speed.py --baseline ce81108 --output composition.json --trials 9 --official-root /tmp/library-checker-problems --references /tmp/fps-reference
```
