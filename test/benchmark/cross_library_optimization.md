# 高速化の共有 2

2026-10-20。基準は `8f30994`。オフライン動的連結性で追加した安定基数ソートを、Mo 法の処理順と Auxiliary Tree に適用した。公開 API・返り値の順番は維持した。

## 調査と採用

| 既存の高速化 | 適用先 | 判断 |
| --- | --- | --- |
| `stable_radix_sort` の整数キー、共通バイトの省略、整列済み判定 | `mo_order` | 採用。ブロック番号・右端の64 bitキーを安定ソートし、同じキーのクエリは元の添字順を保つ |
| 同じ基数ソート | `AuxiliaryTree::build` | 採用。非負の DFS 番号をキーにする。重複頂点・圧縮 ID・親子の順番は維持 |
| CSR の累積和を格納位置にも使い、書き込み位置のコピーを省く方法 | `UndirectedCSR`、利用先の Low-Link・二辺連結成分 | 試作して見送り。配列の入力順を保つため逆順に格納したが、速度が安定して改善しなかった |
| 整数キーの基数ソート | 二項係数 prefix sum の多項式評価点の整列 | 候補のみ。多項式演算を含む全体の比較は未測定のため、今回は変更しない |
| 共通の基数ソートへの置換 | Wavelet Matrix の座標圧縮、Run 列挙 | 専用の基数ソートが既にある。置換はせず、今回は未測定 |
| 同じ置換 | Suffix Array の比較ソート | 比較ソートは16文字未満の接尾辞比較に限定され、大きい入力は SA-IS。単一の整数キーへの置換はしない |

Interval Set などの一般の比較型を扱う場所に、64 bit整数への変換を持ち込んではいない。共有部品自体は変更せず、既存の利用先への影響を抑えた。

Mo 法は2048件未満、Auxiliary Tree は256件未満で従来の比較ソートを使う。切り替え位置は [screen.json](results/cross_library_optimization/screen.json) の256・512・1024・2048件の候補比較から選んだ。Mo 法は64 bitキーの上位・下位に異なる情報が入り、Auxiliary Tree の DFS 番号より分配回数が多く、小さい入力で基数ソートを使うと遅かった。これは入力サイズによる切り替えであり、特定の問題・頂点番号・seed の答えには依存しない。

整数幅を固定すると、Mo 法の処理順の生成と DFS 番号の整列はともに線形時間になる。LCA を使う Auxiliary Tree の構築も $$O(k)$$、HLD を使う場合は LCA 呼び出しを含めて $$O(k\log n)$$。既に整列した入力には引き続き `build_sorted` を使え、基数ソートの作業領域も確保しない。

## 参考提出

Library Checker 公式 API の AC 実行時間順の上位20件を取得し、C++ 上位2件ずつのソースを確認した。一覧・取得時刻・ソースの SHA-256 は [references.json](results/cross_library_optimization/references.json)。Mo 法の処理順と Auxiliary Tree 自体の直接の対応問題はないため、後者が使う LCA と、CSR の適用候補の二辺連結成分を調べた。

| 問題・提出 | 取得時の最大時間 | 確認内容 |
| --- | ---: | --- |
| LCA [278239](https://judge.yosupo.jp/submission/278239) | 24 ms | 親番号の大小関係を利用した DFS 番号の構築、64要素のブロックと前後の最小値による RMQ |
| LCA [370969](https://judge.yosupo.jp/submission/370969) | 27 ms | 同じ DFS 番号の構築とブロック RMQ。区間内の LCA は定数時間 |
| Two-Edge-Connected Components [305681](https://judge.yosupo.jp/submission/305681) | 26 ms | CSR と別の書き込み位置配列、反復 DFS、Union Find による成分の統合 |
| 同 [390338](https://judge.yosupo.jp/submission/390338) | 27 ms | 同じ構成。書き込み位置配列を残している |

LCA の上位提出は入力で親番号が小さい条件を利用する。一般の木の頂点番号にその条件を課してはいない。今回は LCA 本体や DFS のアルゴリズムを変更せず、Auxiliary Tree の指定頂点の整列だけを改善した。上位提出との実行時間の直接比較やオンライン提出は行っていない。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。
- `-std=c++23 -O2 -DNDEBUG -ftrivial-auto-var-init=zero`。native は `-march=native` を追加、generic は追加しない。
- 1回のウォームアップ後に7回。各ラウンドの実行順を固定 seed でシャッフルし、中央値を使う。コンパイルやテストと同時には測定しない。
- 入力生成・LCA/HLD の前処理・入出力は計測外。Mo 法は処理順生成を計測し、チェックサムは計測外。Auxiliary Tree は補助木構築と全生成ノードのチェックサムを計測する。両者とも関数内部の作業領域の確保・解放を含む。
- Mo 法は同じ区間集合を表の回数だけ反復する。Auxiliary Tree は独立に生成した頂点列を順に処理する。全反復のチェックサムを変更前後で照合した。
- 元の値域は Mo 法が $$10^6$$、Auxiliary Tree が20万頂点のランダムな木。Auxiliary Tree の `k` は重複を含む入力列長。
- 全試行・コマンド引数・環境・対象ヘッダーとドライバーのハッシュは [native.json](results/cross_library_optimization/native.json)・[generic.json](results/cross_library_optimization/generic.json)。AtCoder 実機と macOS 実機では未測定。

次は反復全体の中央値、ms。

| 操作・入力 | 反復数 | native 前 → 後 | generic 前 → 後 |
| --- | ---: | ---: | ---: |
| Mo 法、ランダム20万件 | 5 | 50.242 → 19.069 | 50.705 → 20.022 |
| Mo 法、ランダム100万件 | 1 | 59.346 → 28.056 | 60.593 → 28.603 |
| Mo 法、同じ少数の区間を多数含む20万件 | 3 | 36.809 → 11.571 | 36.743 → 12.025 |
| Mo 法、左端・右端の辞書順に並べた20万件 | 3 | 18.484 → 11.372 | 19.019 → 12.424 |
| Auxiliary Tree / LCA、k=256 | 1562 | 24.008 → 15.408 | 22.946 → 15.579 |
| Auxiliary Tree / LCA、k=8192 | 48 | 28.010 → 15.127 | 27.544 → 14.625 |
| Auxiliary Tree / LCA、k=20万 | 3 | 41.662 → 15.767 | 42.177 → 15.986 |
| Auxiliary Tree / HLD、k=256 | 1562 | 28.387 → 20.545 | 29.687 → 21.429 |
| Auxiliary Tree / HLD、k=20万 | 3 | 44.481 → 15.881 | 42.320 → 15.436 |

小さい入力やソートを省く経路も測定した。全てのケースで同じ割合の改善とは扱わない。

| 操作・入力 | 反復数 | native 前 → 後 | generic 前 → 後 |
| --- | ---: | ---: | ---: |
| Mo 法、16件 | 62500 | 5.837 → 7.036 | 6.322 → 6.082 |
| Auxiliary Tree / LCA、k=2 | 200000 | 10.419 → 10.176 | 10.986 → 10.962 |
| Auxiliary Tree / HLD、k=255 | 1568 | 28.755 → 29.923 | 29.381 → 29.305 |
| Auxiliary Tree / LCA、build_sorted、k=4096 | 100 | 14.721 → 14.671 | 14.250 → 14.130 |
| Auxiliary Tree / HLD、build_sorted、k=4096 | 100 | 16.540 → 16.631 | 17.385 → 16.515 |

16件の Mo 法は native で1呼び出しあたり約19 ns遅くなった。HLD の255件も native で約4%遅い。これらは従来の比較ソートを使う経路で、基数ソートを実行していない場合にもコンパイル後の時間差は残った。

## 見送った CSR の候補

3回の予備測定では、20万頂点・60万辺のランダムな入力の構築が21.593 → 20.436 msだった一方、同じ規模で辺を辞書順に並べた場合は13.973 → 23.744 msだった。Low-Link 全体のランダム入力も69.904 → 83.146 ms。逆順の格納と作業領域の変更を含む差であり、原因を特定の命令に帰属させてはいない。追加配列の削減だけでは採用せず、`UndirectedCSR` とその利用先は変更していない。

## 検証と再現

変更箇所と推移的な利用先に絞る。Mo 法は比較ソートによる独立な順序生成と照合し、同一キーの添字順・境界値・切り替え位置・入力不変を確認する。Auxiliary Tree は既存の愚直解・30万頂点のテストに加え、256件前後・重複・全キー一致・根を変更した木で `build_sorted` と比較する。二項係数 prefix sum の比較用実装も Mo 法を使うため、その単体テストを含める。

GCC・Clang で変更した2ヘッダーの単独 include と関連3単体テストに成功した。Clang の ASan・UBSan でも同じ3単体テストに成功した。Library Checker の LCA 公式25ケースは GCC・Clang の両方で期待出力と一致した。全180ライブラリのドキュメント情報を確認し、変更した2ページと提出ドライバーの展開コードをコンパイルした。

検証対象は [selection.json](results/cross_library_optimization/selection.json)、結果は [validation.json](results/cross_library_optimization/validation.json) と [validation.log](results/cross_library_optimization/validation.log)。

```sh
python3 test/benchmark/cross_library_optimization.py --screen --trials 3 --output test/benchmark/results/cross_library_optimization/screen.json
python3 test/benchmark/cross_library_optimization.py --output test/benchmark/results/cross_library_optimization/native.json
python3 test/benchmark/cross_library_optimization.py --generic --output test/benchmark/results/cross_library_optimization/generic.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/cross_library_optimization/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/cross_library_optimization/selection.json
```

基準のヘッダーは Git の `8f30994` から取得し、比較対象と別の include ディレクトリに固定する。予備測定の候補はスクリプト内の置換で再構成する。一時ブランチは作っていない。
