# Aho–Corasick

2026-10-20。`string/aho_corasick.hpp` を追加した。標準 C++ で実装し、x86 専用命令は使わない。文字集合はテンプレート引数で指定し、既定は英小文字。複数パターン検索・出現回数・一致 ID 列挙・分割した本文の走査・suffix link 上の集計に対応する。

## 参考提出

Library Checker の [Aho Corasick](https://judge.yosupo.jp/problem/aho_corasick) の AC 上位20件を公開 API で取得し、C++ の上位3件を確認した。取得時刻・一覧・各ソースの SHA-256 は [references.json](results/aho_corasick/references.json)。

| 提出 | 取得時の最大時間 | 方式 |
| --- | ---: | --- |
| [307744](https://judge.yosupo.jp/submission/307744) | 102 ms | 文字ごとの1バイトの位置表、実際の辺だけを入れた `basic_string<int>`、suffix link をたどる構築、blazingio |
| [307742](https://judge.yosupo.jp/submission/307742) | 103 ms | 同じ構造と構築方式 |
| [307737](https://judge.yosupo.jp/submission/307737) | 103 ms | 同じ構造と構築方式 |

これらは固定配列を使い、Trie の親・suffix link・パターンの終端を出力する。今回のライブラリでは本文を繰り返し走査する用途も扱うため、全遷移を配列で完成させた。検索中の suffix link の探索を省く代わりに、構築時に文字集合全体を走査し、メモリも多く使う。上位提出より常に速いという結果ではない。

## 測定条件

WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2。`-std=c++23 -O2 -DNDEBUG -march=native -ftrivial-auto-var-init=zero -DONLINE_JUDGE`。ウォームアップ1回、7回測定、各ラウンドの実行順は固定 seed でシャッフル。他のコンパイル・テストとは同時に測定しない。

公式生成器の commit は `1814c4e5205517e368bb57a8d1127eb961cfeaae`。各入力のハッシュ、コンパイラ・CPU、対象ソースのハッシュ、全試行は [measurements.json](results/aho_corasick/measurements.json)。公式全72ケースは GCC・Clang で checker を通した。参考提出も初期比較で全72ケースを通し、変更していないため最終版では再検証を省いた。初期版の結果は [measurements_initial.json](results/aho_corasick/measurements_initial.json)。公開ジャッジへのオンライン提出はしていない。

## プログラム全体

公式入力を通常ファイルから読み、標準出力を `/dev/null` へ出す。プロセス起動・入力・構築・出力・終了を含む。今回のドライバーは FastInput/FastOutput、参考提出は元の blazingio なので、アルゴリズム単体の比較ではない。次は中央値、ms。

| 入力 | 今回 | 提出307744 |
| --- | ---: | ---: |
| bfs_03 | 37.69 | 38.09 |
| random_03 | 96.82 | 57.22 |
| random_11 | 117.27 | 58.74 |
| short_period_00 | 56.50 | 47.36 |
| fibonacci_00 | 59.65 | 44.43 |
| almost_single_00 | 61.18 | 50.26 |

Trie の構築とリンクの出力だけを行うこの問題では、完成した遷移表の検索速度は利用されない。疎な子の表を持つ参考提出は、特にランダム入力で速かった。

## 構築と検索

比較用の failure 遷移版は、同じクラスから「欠けた遷移を埋める処理」を除き、suffix link の構築と本文の走査で必要に応じて suffix link をたどる形にした。頂点情報・文字集合・API・メモリ配置は共通。生成は計測スクリプト内で行い、本体には別の方式を追加していない。

入力後に計時を始め、オブジェクト作成・reserve・全パターン登録・build を構築時間とする。検索時間は長さ約200万の本文に対する `count_matches` だけを計測する。入力・出力・チェックサム・破棄はこの計時に含めない。両実装の suffix link のチェックサムと総出現回数を照合した。次は中央値、ms。

| 分布 | 完成配列の構築 | failure 版の構築 | 完成配列の検索 | failure 版の検索 |
| --- | ---: | ---: | ---: | ---: |
| ランダム26文字、長さ30を2万個 | 59.02 | 49.89 | 31.71 | 49.54 |
| ランダム2文字、長さ30を2万個 | 36.81 | 35.56 | 32.39 | 69.62 |
| 同じ文字60万個のパターン | 23.51 | 23.99 | 7.87 | 15.31 |
| 同じ文字1～1000個のパターン | 0.97 | 0.95 | 3.28 | 6.78 |
| 長い共通接頭辞 | 1.74 | 2.25 | 3.29 | 6.73 |
| 同じ長さ7のパターン8万個 | 0.57 | 0.58 | 3.29 | 2.18 |

完成配列は多くの分布で検索を短縮した一方、同じ短いパターンを多数登録した分布では failure 版の方が速かった。文字集合・Trie の形・本文・走査回数に依存するため、常に一定倍率で速いとは扱わない。

## 試した配置

初期版は、26文字の遷移表と7個の整数を持つ132バイトの頂点だった。終端の登録数は、構築後の一致数から suffix link 先の一致数を引いて得られるため、専用の整数を省いた。

その上で64バイト境界へ揃えた128バイトの頂点とし、検索時に参照する一致数を先頭へ置いた。小さい文字集合では不要に領域を広げない。重複 ID の情報は頂点の終端リストに残る。全て標準 C++ の `alignas` と `vector` を使う。

[layout_screen.json](results/aho_corasick/layout_screen.json) は一致数を末尾へ置いた候補の3回比較、[layout_front_screen.json](results/aho_corasick/layout_front_screen.json) は先頭へ置いた候補の7回比較。先頭配置の候補は、初期版に対して構築が各分布で約4～9%短く、検索はランダム26文字で約11%、接尾辞の連鎖・共通接頭辞・重複で約17～18%短かった。ランダム2文字と長い単一パターンの検索は約0～1%遅かった。比較は同じ入力と GCC の条件で行ったもので、Mac の性能は測っていない。

## 検証

- 2文字・長さ2以下の全パターン集合128通りと、ランダム300通りを素朴な文字列比較と照合。
- 全頂点の suffix link・output link・遷移・Trie の直接の子・終端数・一致数を照合。
- 空のパターン集合、空文字列、重複、重なる一致、範囲外の文字、NUL・128以上を含む全256バイト。
- パターンごとの出現数・総数・一致 ID 列挙・分割した本文の走査・集計・コピー・clear・build の再呼び出し。
- 長さ20万のパターンと、32 bit を超える総出現回数。
- GCC・Clang、Clang ASan/UBSan、公式72ケース、説明内コードの実行、ヘッダーと提出コードの Bundle。

範囲は [selection.json](results/aho_corasick/selection.json)、ログは同じディレクトリの `unit.log`・`sanitize.log`・`docs.log`・`benchmark.log`。

```sh
python3 test/benchmark/aho_corasick.py --references
flock /tmp/cp-wm-opt.lock python3 test/benchmark/aho_corasick.py
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/aho_corasick/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/aho_corasick/selection.json
```
