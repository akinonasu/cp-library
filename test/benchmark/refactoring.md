# ライブラリの整理

2026-10-20。基準は `7a67c6e`。公開 API を保ち、重複する探索・確保・初期化・コピーを整理した。

## 変更

| 対象 | 整理した処理 | 保持した条件 |
| --- | --- | --- |
| Dynamic Segtree | `update` と `write` で同じ葉を二度二分探索していた。求めた位置を渡し、分割時も位置を補正して再利用する | 既存要素の上書き・追加、非可換な集約、64 bit座標、葉の再確保 |
| Sparse Table | 各 `build` で全段を破棄して確保し直す処理を除いた。`op` が const 参照を受け取れる場合は、引数用の要素コピーも省く | `bool`、デフォルト構築できない型、一時値だけを受け取る演算、引数への参照を返す演算 |
| Disjoint Sparse Table | 再構築の直前に全段を破棄する処理を除き、既存の容量を再利用する | 非可換演算、`bool`、デフォルト構築できない型、空入力、サイズの増減 |
| Rerooting | 全頂点の答えを一旦破棄して作り直す処理と、後で全て上書きする配列への単位元の代入を減らす | 非可換な結合順、木・森・孤立点、頂点と辺の異なる型、デフォルト構築できない型 |
| Weighted Union Find | `diff` で連結性確認後に再度 `weight` を呼ぶ経路を整理し、各端点で一度だけ経路圧縮する | 連結性の assert、modint・128 bit整数、経路圧縮による重み更新 |
| Fraction | 既約な値の逆数を作る際の再約分を除く。分子・分母の交換と符号調整だけを行う | 負数、定数評価、整数型の境界、正の分母、表現できる結果に限定する条件 |

Sparse Table の一時値向けの経路では、演算が引数への参照を返しても、その一時値の生存中に結果を値へ変換する。参照のまま補助関数の外へ返さない。

全方位木 DP では、後行順で子から親への値、先行順で親から子への値を計算する。参照する辺の寄与は必ず先に計算済みになるため、未計算の要素を単位元で埋める必要はない。型をデフォルト構築できない場合は、従来どおり単位元から要素を作る。

Sparse Table 系では、再構築後にも過去の配列容量を保持する場合がある。使い方と領域を解放する方法は各説明ページへ追記した。計算量と利用者向けの関数名は変えていない。Fraction の逆数は、説明にある定数時間の処理を実装でも満たすようになった。

## 変更しなかった処理

- 無向 CSR の格納位置の配列削減は、[前回の測定](cross_library_optimization.md)で悪化があり、繰り返して導入していない。
- LIS は要素型によって値そのものと添字の保持を使い分ける。二つの経路には型とコピーコストの理由があるため、見た目だけで一つにしていない。
- Aho–Corasick などの境界検査・構築済み判定の assert は維持した。`NDEBUG` で除かれる検査を、単に行数を減らすためには削除していない。
- Fraction の四則演算は交差約分済みの結果を作る経路を既に共有している。今回はそこを組み直さず、逆数で残っていた GCD だけを除いた。
- FPS・畳み込みの作業領域のゼロ埋めやコピーは、変換長への拡張・出力の未使用部分・別名参照などの条件がある。不要と証明できないものは削除していない。
- Li Chao Tree の狭い整数／128 bit演算の切り替え、ModPow の素因数表などは意図した最適化として残した。

## 参考提出

Library Checker 公式 API で AC 実行時間順の上位20件を取得し、C++ の先頭1件ずつを確認した。一覧・取得時刻・各ソースの SHA-256 は [references.json](results/refactoring/references.json)。汎用 API を問題専用の方式に置き換えるための調査ではない。

| 問題・提出 | 取得時の最大時間 | 確認内容と今回の扱い |
| --- | ---: | --- |
| Static RMQ [402567](https://judge.yosupo.jp/submission/402567) | 19 ms | ブロック Sparse Table、AVX2、複数問い合わせの一括処理。今回は一般の値型を扱う表の再構築とコピーに絞る |
| Static Range Sum [356315](https://judge.yosupo.jp/submission/356315) | 17 ms | 累積和と専用入出力。逆元を要求しない Disjoint Sparse Table を累積和には置き換えない |
| Point Add Range Sum [403014](https://judge.yosupo.jp/submission/403014) | 23 ms | 多分木の prefix sum 構造。任意の64 bit座標を扱う Dynamic Segtree は維持する |
| Point Set Range Composite [402223](https://judge.yosupo.jp/submission/402223) | 49 ms | 逆数をまとめて求める前処理と多分木。逆元やクエリ先読みを一般の集約には要求しない |
| Tree Path Composite Sum [346214](https://judge.yosupo.jp/submission/346214) | 38 ms | 根付き木で部分木と親側の寄与を分ける二回の DP。今回は非可換な演算にも使える全方位 DP の初期化を整理する |
| Unionfind with Potential [224108](https://judge.yosupo.jp/submission/224108) | 8 ms | 両端点を一度ずつ根まで圧縮して差分を読む。今回も経路圧縮の重複を除く |
| Rational Approximation [290530](https://judge.yosupo.jp/submission/290530) | 32 ms | Stern–Brocot 上の探索と交差積による比較。汎用分数の逆数を直接比較する問題ではない |

上位提出とのローカルな速度比較やオンライン提出は行っていない。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 固定。
- `-std=c++23 -O2 -ftrivial-auto-var-init=zero`。native は `-march=native` を追加。通常は `-DNDEBUG`、Weighted Union Find は assert を有効にした版も測る。
- 1回のウォームアップ後に7回。各ラウンドの実行順を固定 seed でシャッフルし、中央値を使う。コンパイル・テスト・別の測定とは同時に実行しない。
- 入力生成・入出力・グラフへの辺の登録は計測外。表と全方位木 DP は `build` を計測し、呼び出し内部の確保・解放を含める。
- 「初回」は反復ごとに空のオブジェクトから構築。「再構築」は一つのオブジェクトを繰り返し使い、最初の1回も時間に含める。表の元配列は同じものを使う。
- Sparse Table の文字列はランダムな64文字。問い合わせは1構築あたり20万区間で、結果のハッシュも問い合わせ時間に含む。
- Dynamic Segtree は20万点を登録し、ランダムに選んだ既存の点への加算を80万回行う。ランダム・狭い座標範囲・昇順・降順を比較する。
- 全方位木 DP は頂点番号と辺の順序をシャッフルした20万頂点の木。全頂点数を求める DP を使い、構築の開始頂点を変えて5回実行する。
- Weighted Union Find は20万頂点を段階的に併合し、100万回の `diff` を計測。Fraction は事前に正規化した正負の100万分数の逆数を計測する。
- 全ての入力で変更前後の checksum を照合する。表の問い合わせ・Weighted Union Find・Fraction は checksum の計算も測定内、その他は測定外。
- 環境・コマンド引数・ソースハッシュ・全試行は [native.json](results/refactoring/native.json)・[generic.json](results/refactoring/generic.json)、逆数は `fraction_native.json`・`fraction_generic.json`。AtCoder 実機と macOS 実機では未測定。

次は反復全体の中央値、ms。

| 対象・操作 | native 前 → 後 | generic 前 → 後 |
| --- | ---: | ---: |
| Sparse Table、整数20万要素、初回構築5回 | 27.830 → 27.383 | 27.978 → 28.516 |
| 同、再構築5回 | 29.945 → 10.903 | 29.681 → 11.420 |
| Disjoint Sparse Table、20万要素、初回構築5回 | 29.838 → 29.906 | 29.905 → 29.801 |
| 同、再構築5回 | 32.348 → 13.855 | 33.304 → 12.881 |
| Sparse Table、文字列2万要素、初回構築5回 | 53.846 → 27.948 | 54.406 → 27.468 |
| 同、再構築5回 | 64.705 → 17.343 | 67.955 → 17.984 |
| 同、初回構築版の問い合わせ100万回 | 79.513 → 55.315 | 79.559 → 55.419 |
| Dynamic Segtree、ランダム座標の更新80万回 | 111.402 → 107.170 | 138.426 → 133.970 |
| 同、狭い座標範囲の更新80万回 | 94.591 → 90.920 | 119.203 → 113.607 |
| Weighted Union Find、assert なしの diff 100万回 | 5.968 → 5.821 | 5.815 → 5.560 |
| 同、assert あり | 10.835 → 6.921 | 10.387 → 7.008 |
| Fraction、100万分数の逆数 | 134.478 → 1.778 | 138.118 → 1.855 |

全方位木 DP は初回で大きく短縮する結果ではなく、再構築も形によって差があった。

| 木・5回の構築 | native 前 → 後 | generic 前 → 後 |
| --- | ---: | ---: |
| ランダム、初回 | 76.074 → 79.322 | 74.670 → 75.107 |
| ランダム、再構築 | 71.159 → 70.245 | 72.910 → 68.645 |
| パス、初回 | 57.921 → 57.043 | 63.232 → 62.888 |
| パス、再構築 | 56.949 → 55.745 | 60.908 → 60.093 |
| 星、初回 | 35.115 → 36.484 | 38.133 → 35.760 |
| 星、再構築 | 37.298 → 31.676 | 32.977 → 34.413 |

Dynamic Segtree の降順登録の構築は native で6.561 → 7.458 ms、generic で7.305 → 6.972 msだった。探索回数を減らしても全ての入力・コンパイル設定で速くなるとはしていない。今回は処理の重複を除く整理として採用し、この悪化も残す。

## 検証と再現

変更箇所と推移的な利用先だけを選ぶ。[selection.json](results/refactoring/selection.json) に対象を保存する。単体テストはサイズの増減・空配列への再構築・非可換演算・特殊な値型・木の開始頂点の変更を含む。Fraction は64 bitの境界値で表現できる逆数を128 bitの参照実装と比較する。

結果は [validation.json](results/refactoring/validation.json)、`unit.log`・`validation.log`・`fraction_validation.log` に保存する。

GCC・Clang の関連16単体テストと、Clang の ASan・UBSan に成功した。Library Checker の17ドライバーは設定済みの GCC・Clang の両環境で成功した。全180ライブラリのドキュメント情報を確認し、影響を受ける12ヘッダーと17提出ドライバーの展開コードをコンパイルした。

```sh
python3 test/benchmark/refactoring.py --output test/benchmark/results/refactoring/native.json
python3 test/benchmark/refactoring.py --generic --output test/benchmark/results/refactoring/generic.json
python3 test/benchmark/refactoring.py --fraction-only --output test/benchmark/results/refactoring/fraction_native.json
python3 test/benchmark/refactoring.py --fraction-only --generic --output test/benchmark/results/refactoring/fraction_generic.json
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/refactoring/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/refactoring/selection.json
```

比較対象のヘッダーは Git の `7a67c6e` から取得して固定する。測定用のブランチは作っていない。
