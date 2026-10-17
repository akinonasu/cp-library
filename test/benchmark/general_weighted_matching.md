# General Weighted Matching

## 実装

`GeneralWeightedMatching` は整数重みの無向一般グラフで最大重みマッチングを求めます。Blossom の縮約・展開と双対変数の更新による決定的な算法で、時間は O(n³)、空間は O(n²) です。組数の最大化は目的に含めません。

入力は 64 bit 符号付き整数、合計は 128 bit です。自己ループと非正の辺は無視し、多重辺は最大の重みだけを保持します。辺を追加して再計算でき、変更がなければ計算済みの結果を返します。

- 元の重み行列、縮約後の代表辺、子の対応を連続配列に配置。
- 探索は CSR の隣接列を使い、存在する辺だけを走査。
- キューは再利用する `vector<int>`。縮約時に内部の元の頂点を追加。
- 頂点数と最大重みによって内部の整数型を 32／64／128 bit から選択。
- 公開 API と内部の Blossom を `graph/` と `components/` に分離。

双対変数は非負で、目的値に対する双対上界から大きさを制限できます。`4*(n+1)*maximum` が収まる整数型を選ぶことで、双対変数・重みの 2 倍・差の計算にも余裕を持たせています。64 bit の最大重みを使う場合は 128 bit へ切り替え、合計も独立に 128 bit で計算します。CPU 固有命令や GCC 専用 pragma は使いません。

## 参考提出

2026-10-18 JST に [General Weighted Matching](https://judge.yosupo.jp/problem/general_weighted_matching) の AC 提出を実行時間順に調査しました。取得結果とソースの SHA-256 は [general_weighted_matching_reference.json](results/general_weighted_matching_reference.json) に保存しています。

| 提出 | 言語・表示時間 | 調査内容 |
| --- | --- | --- |
| [404902](https://judge.yosupo.jp/submission/404902) | C++23・0.240 s | O(n³) の Blossom。探索木・slack の対象をリストで保持し、花の回転・反転を添字で処理 |
| [353300](https://judge.yosupo.jp/submission/353300) | C++23・0.240 s | 404902 と同一ソースであることをハッシュで確認 |
| [353331](https://judge.yosupo.jp/submission/353331) | C++23・0.243 s | 同系統の Blossom の実装を確認 |
| [324220](https://judge.yosupo.jp/submission/324220) | C++20・0.248 s | 全頂点の走査と `reverse`／`rotate` による素朴な O(n³) の Blossom |

404902 と 324220 をソースのままローカルで測定しました。両者とも固定サイズの行列と 32 bit の重み・双対変数を使い、探索時は元の頂点すべてを走査します。現在の実装では動的な頂点数、疎な隣接列、可変の整数幅を使い、花の順序は配列の反転・回転で管理します。

## 測定条件

- WSL2 Ubuntu、Intel Core i7-13700F、Linux x86_64、GCC 15.2.0。
- `-std=c++23 -O2 -DEVAL -DONLINE_JUDGE -march=native -ftrivial-auto-var-init=zero`。assert は有効。
- CPU 2 に固定。各条件でウォームアップ 1 回と測定 7 回、中央値を掲載。
- 実行順 seed は 404902。各試行で比較対象の順番をシャッフル。
- 全バイナリのコンパイルを終えてから測定し、計時中にはテストや文書生成を実行しない。
- バイナリ・入力・出力は Linux の `/tmp`。全体時間は新しいプロセスの起動から終了までで、通常ファイルに接続した入出力・メモリ確保・構築・計算を含む。checker とハッシュ計算は計時外。
- 別途ファイルの解析を計時外とし、グラフ構築とマッチング計算を分けて測る。

比較版は、現在の整数幅の自動選択、内部を 64 bit に固定、128 bit に固定、および初期の双対変数に対して tight な最大重み辺を貪欲にマッチさせる試行版です。固定幅の版も入力・合計・アルゴリズム・入出力は同じです。今回の公式ケースはすべて自動選択が 32 bit になる範囲です。

環境・ソース・生成した比較版のハッシュは [environment.json](results/general_weighted_matching/environment.json)、全試行は [results.json](results/general_weighted_matching/results.json) に記録しています。Library Checker のサーバー上での提出・測定ではありません。

## 計算部分

単位は ms。入力解析と出力を除き、`graph` は公開オブジェクトの構築と辺追加、`build` は作業領域の確保・計算・解放と結果の集計を含みます。各版の重みの合計を照合し、返されたマッチングの実行可能性も確認しています。

| ケース | 現在の graph | 現在の build | 64 bit 固定の build | 128 bit 固定の build |
| --- | ---: | ---: | ---: | ---: |
| `max_random_00` | 0.455 | 25.414 | 27.588 | 45.684 |
| `sparse_04` | 0.360 | 4.105 | 5.324 | 10.885 |
| `complete_00` | 0.947 | 121.093 | 123.458 | 191.522 |
| `complete_plane_01` | 0.870 | 177.004 | 179.455 | 281.976 |
| `complete_mandist_01` | 0.810 | 133.803 | 136.664 | 213.353 |
| `cycles_00` | 0.368 | 4.260 | 5.352 | 10.783 |

必要な整数幅を選ぶことで、常に広い型を使う場合の負荷を避けられました。貪欲な初期化の試行版は `complete_00` の build が 119.979 ms ですが、`sparse_04` は 4.486 ms、`cycles_00` は 4.594 ms でした。効果が一様ではないため採用していません。

## 入出力を含む公式ケース

全 24 ケースを全比較版で公式 checker に通しました。代表ケースの中央値です。単位は ms。

| ケース | 現在 | 64 bit 固定 | 128 bit 固定 | 404902 | 324220 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `max_random_00` | 27.638 | 29.837 | 47.527 | 65.170 | 64.993 |
| `max_random_01` | 76.567 | 78.936 | 120.125 | 144.330 | 140.849 |
| `sparse_04` | 6.245 | 7.628 | 12.382 | 14.197 | 14.889 |
| `binary_tree_00` | 5.914 | 6.835 | 12.095 | 12.955 | 13.938 |
| `cycles_00` | 5.651 | 6.859 | 12.292 | 14.499 | 15.495 |
| `complete_00` | 127.436 | 127.843 | 198.562 | 109.475 | 114.324 |
| `complete_plane_00` | 179.045 | 182.382 | 278.015 | 155.048 | 162.806 |
| `complete_plane_01` | 176.680 | 181.498 | 279.130 | 155.810 | 162.705 |
| `complete_mandist_01` | 137.985 | 140.670 | 216.478 | 118.931 | 125.276 |

疎なグラフやランダムケースでは上位提出より短くなりました。一方、完全グラフ系では 404902 より約 13〜16% 長く、全ケースでの最速は達成していません。参照提出とは入出力・記憶域・花の管理も異なるため、差のすべてを CSR や整数幅に帰属させることはできません。

## 検証

- GCC 15.2・13.4 と Clang 21 で単独 include・単体テスト。
- Clang ASan/UBSan と `NDEBUG`。
- 4 頂点までの重み 0..3 の全グラフと、18 頂点までのランダムグラフを部分集合 DP と照合。
- 負の重み・自己ループ・多重辺・再計算・キャッシュ・clear・コピー・ムーブを確認。
- 32／64／128 bit の切り替え境界、`INT64_MAX` の辺、64 bit を超える合計を確認。
- 公式 24 ケースを GCC・Clang で確認。比較測定の各版も公式 checker で照合。
- Clang の `NDEBUG` で結合した提出コードも全 24 ケースを確認。

## 再現

```sh
python3 test/benchmark/general_weighted_matching_reference.py \
  --cache .verify-helper/cache/weighted-matching-reference \
  --output test/benchmark/results/general_weighted_matching_reference.json
python3 test/benchmark/general_weighted_matching.py \
  --references .verify-helper/cache/weighted-matching-reference \
  --official ~/.cache/online-judge-tools/library-checker-problems/graph/general_weighted_matching \
  --output test/benchmark/results/general_weighted_matching --cpu 2 --trials 7
```

GCC 15 と、生成済みの公式入力・出力・checker が必要です。
