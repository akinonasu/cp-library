# Dynamic Wavelet Matrix の count

## 既存 API

`count(l,r,x)` と `rank(x,r)` は変更前から存在します。元から `x` の経路を 1 回だけたどる実装です。`range_freq(l,r,x,x+1)` は上限未満の個数を 2 回求めて差を取るため、通常は 2 回探索します。整数型の最大値では `x+1` を作れないので、`count` を直接使えます。

今回の変更は既存の `count` の高速化です。新しい公開 API は追加せず、比較用に変更前のヘッダーも同じ条件でビルドしています。

## 参考

2026-10-17 に Library Checker Point Set Range Frequency の上位 AC を調査しました。

| 提出 | judge の記録 | 方法 |
| --- | ---: | --- |
| [393918](https://judge.yosupo.jp/submission/393918) | 34 ms | 値ごとのイベント列を CSR にまとめ、位置のビット集合と Fenwick 木で処理 |
| [312395](https://judge.yosupo.jp/submission/312395) | 34 ms | 値ごとに点代入の履歴と問い合わせを集約し、BinaryIndexedTree01 で処理 |
| [404514](https://judge.yosupo.jp/submission/404514) | 40 ms | 393918 と同系統のオフライン処理 |

これらは全問い合わせを先読みし、値ごとに処理順を変えます。今回のオンライン API と任意位置への挿入・削除にはそのまま適用しません。参照提出は手法の調査のみで、下の時間の測定には含めていません。judge の記録と手元の測定値は比較できません。

取得時刻・順位一覧・原文の SHA256 は `results/dynamic_wavelet_count_reference.json`、取得スクリプトは `dynamic_wavelet_count_reference.py` にあります。

## 実装

- 空区間と値域外は即座に 0 を返す。
- 先頭の全要素で共通する bit は、各段の 0 の個数で判定。値が合わなければ 0、合えば区間を移動せずに次の段へ進む。
- 区間が最初から 1 要素なら、`access_rank` で bit と順位を同時に取得し、異なる bit があれば打ち切る。
- その他の区間は、従来通り両端の順位をまとめて求めて下降する。
- 更新時の追加処理・追加メンバー・キャッシュ・メモリ確保はない。

途中で候補が 1 要素になった場合も専用経路へ移す案は、native 条件で遅くなる入力があったため採用していません。全段で共通 bit を毎回判定する案も、先頭の連続部分だけに限定しました。

計算量は従来と同じ O(B log n) です。`rank` は `count` を利用するため同じ変更が反映されます。`range_freq`・`select` の探索や更新処理は変更していません。

## 測定条件

Linux x86_64 / WSL2、Core i7-13700F、GCC 15.2、CPU 2 固定です。`-std=c++2b -O2 -DNDEBUG` と、さらに `-march=native -ftrivial-auto-var-init=zero` を加えた条件を測定しました。AtCoder 本番環境の実測ではありません。

初期要素数・操作数は各 100,000、seed は 18106471 です。ウォームアップ 1 回の後、実行順を混ぜた 7 回の中央値です。プロセス内で操作列の実行だけを測定し、入力生成・構築・検証・入出力・終了は含めません。計測中に他のコンパイル・テストは実行していません。

変更前の基準は `22c50c36e5066c07c2e6c34ab2c81a1896ce53ae` です。スクリプトがそのコミットの Wavelet Matrix と Bit Vector を取得してビルドします。今回 Bit Vector の変更はありません。

`range_freq` 列は変更後のヘッダーで `range_freq(l,r,x,x+1)` を呼びます。`x` が整数型の最大値なら `r-l-range_freq(l,r,x)` とし、オーバーフローを避けます。全実装で同じ操作列を実行し、答えの checksum を照合しました。混合操作は実行後の全要素も確認しています。

全て値型は `int64_t` です。名前が `20` で終わる入力は非負の 20 bit の値を格納するため、符号 bit を含め `B=21` を指定します。それ以外は既定の `B=64` です。

| 入力 | 内容 |
| --- | --- |
| random20 | 20 bit の一様乱数。通常のランダム区間 |
| short20 | 同じ値分布、区間長 0〜16 |
| single20 | 同じ値分布、区間長 0〜1 |
| full64 | 64 bit 全域の一様乱数 |
| missing64 | 64 bit 全域の値に、独立な 64 bit 乱数を問い合わせる |
| narrow64 | 20 bit の値を既定の 64 bit 設定で保持 |
| duplicates64 | 値は 0〜15 の 16 種類 |
| uniform64 | 全要素 7、問い合わせる値は 7 と 8 を交互に選ぶ |
| mixed20 | 25% が代入・挿入・削除、75% が通常の区間の count |

`missing64`・`uniform64` 以外は、問い合わせの半分で区間内の値を選び、残りは値分布から独立に選びます。空区間では値分布から選びます。

## 結果

単位は ms です。

| 入力 | 変更前 count generic | 変更後 count generic | range_freq generic |
| --- | ---: | ---: | ---: |
| random20 | 59.520 | 57.859 | 121.957 |
| short20 | 28.840 | 27.113 | 63.163 |
| single20 | 25.206 | 19.230 | 54.623 |
| full64 | 119.160 | 112.665 | 265.151 |
| missing64 | 55.172 | 53.748 | 114.937 |
| narrow64 | 65.640 | 60.252 | 136.990 |
| duplicates64 | 27.090 | 22.828 | 55.679 |
| uniform64 | 9.463 | 4.995 | 19.613 |
| mixed20 | 87.890 | 86.512 | 137.091 |

| 入力 | 変更前 count native | 変更後 count native | range_freq native |
| --- | ---: | ---: | ---: |
| random20 | 53.418 | 53.370 | 111.187 |
| short20 | 25.047 | 25.742 | 57.127 |
| single20 | 21.949 | 18.216 | 49.460 |
| full64 | 100.123 | 104.467 | 227.327 |
| missing64 | 49.954 | 49.728 | 104.828 |
| narrow64 | 58.661 | 56.839 | 123.159 |
| duplicates64 | 23.760 | 21.192 | 47.817 |
| uniform64 | 6.942 | 4.456 | 14.581 |
| mixed20 | 83.356 | 83.853 | 129.707 |

変更後の `count` は、問い合わせだけの入力では `range_freq` による計算の約 2.1〜3.9 倍、更新込みでは約 1.5〜1.6 倍の速度でした。変更前の `count` も既に 1 回の探索であり、元から `range_freq` より高速です。

変更前の `count` に対する追加の改善は、uniform64 で約 1.6〜1.9 倍、single20 で約 1.2〜1.3 倍、duplicates64 で約 1.1〜1.2 倍です。native の full64 は約 4.3%、short20 は約 2.8% 遅くなりました。更新込みの mixed20 はほぼ同程度です。全ての入力で変更前より速いという結果ではありません。

全測定値・フラグ・コンパイラ・基準コミット・ソースの SHA256 は `results/dynamic_wavelet_count_generic/results.json` と `results/dynamic_wavelet_count_native/results.json` にあります。SHA256 は LF に正規化しています。

## 検証

- 長さ 0〜7、値 -1・0・1 の全列を列挙し、全区間と対象値 -5〜4 を単純な走査に照合。
- 64 bit 符号付き・符号なしの最大最小値、全 bit の反転、空区間、長さ 1 の区間を検証。
- 64 bit word・ブロックの境界を含む位置で検証。
- 全要素が同じ状態から各 bit の異なる値を代入・挿入・削除し、再び同じ状態へ戻した後も照合。
- `range_freq` から安全に求めた個数との一致を確認。
- 既存のランダムな挿入・削除・代入・全問い合わせ・copy/move/swap・再構築のテストを実行。
- GCC 13.4・15.2、Clang 21.1.8、ASan・UBSan、ヘッダー単独 include、単独利用テスト、リリース時の `-Werror`。
- Library Checker の Point Set Range Frequency 25 ケース、Range Kth Smallest 24 ケースを GCC・Clang で確認。
- 新しい単体テストを依存先の期待値に加え、増分検証の 37 テストを確認。

## 再実行

```sh
python3 test/benchmark/dynamic_wavelet_count_reference.py --ids 393918 312395 404514 \
  --cache .verify-helper/cache/dynamic-wavelet-count-reference \
  --output test/benchmark/results/dynamic_wavelet_count_reference.json
python3 test/benchmark/dynamic_wavelet_count.py \
  --output test/benchmark/results/dynamic_wavelet_count_generic
python3 test/benchmark/dynamic_wavelet_count.py \
  --output test/benchmark/results/dynamic_wavelet_count_native --native
python3 test/verify.py --unit-only --selection test/benchmark/results/dynamic_wavelet_count_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize \
  --selection test/benchmark/results/dynamic_wavelet_count_selection.json
oj-verify run --timeout 1200 --tle 10 -j 2 \
  test/dynamic_wavelet_matrix_freq.test.cpp test/dynamic_wavelet_matrix_kth.test.cpp
```
