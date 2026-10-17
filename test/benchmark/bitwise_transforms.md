# Bitwise Zeta / Walsh–Hadamard Transform

## 変更

`components/bitwise_zeta.hpp` と `components/walsh_hadamard.hpp` の API、計算量 `O(n log n)`、追加領域 `O(1)` は維持する。

- 隣接する 2 段を融合し、中間結果を再利用する。
- 下位 10 bit の段を 1024 要素ずつ処理してから、上位の段を処理する。
- GCC / x86 のみ、push/pop で囲んだ `O3` を使う。`math/mint.hpp` の演算子は既に `O3` で、元の `O2` 変換からはインライン化されず、バタフライ内に演算子の呼び出しが残ることを逆アセンブルで確認した。Clang と非 x86 では pragma を使わない。ISA の固定指定や追加の SIMD 命令は使わず、自動ベクトル化に任せる。

各要素の演算順と、各段の `T` への代入を変えていない。例えば Walsh の中間値を `T x = a + b` とすると、元は代入しか要求していなかった式に変換コンストラクタが必要になるため、この書き方は採用していない。ゼータ変換も新たなコピー・デフォルト構築を要求しない。浮動小数点の再結合、modint の内部表現への依存、fast-math はない。

探索では 256 / 1024 / 4096 要素の blocking、融合なし / 2 段融合、GCC `O3` の有無を比較した。今回は新たな型別 SIMD 実装は追加せず、汎用的な 2 段融合と 1024 要素 blocking に絞った。

## 再現方法

基準は変更前 `0a96edcbfda903acd7dcadd0e788be263c9589cf` の 2 ヘッダーで、`bitwise_transforms_baseline.hpp` に関数名を namespace で隔離して保存した。

```sh
g++ -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -I . \
  test/benchmark/bitwise_transforms.cpp -o /tmp/bitwise-transforms
/tmp/bitwise-transforms 6 15
/tmp/bitwise-transforms 10 15
/tmp/bitwise-transforms 18 15
/tmp/bitwise-transforms 20 15
```

引数は `log2(n)` と標本数。各標本内で新旧の測定順を交互にし、各中央値を出す。入力コピー・生成・新旧の全要素一致確認は計測外。入力は固定 seed、値 `[0,1000]`。標本ごとに最大 `max(1, 262144/n)` 回測り、1 回あたりの ms を使う。計時は `steady_clock`。出力の checksum によって結果を使用し、double はビット列を checksum に入れる。

2026-10-17、Linux x86_64、Intel Xeon Platinum 8573C、GCC 14.2.0。AtCoder の公開フラグから `-O2 -march=native -std=gnu++23 -ftrivial-auto-var-init=zero` を採用し、`NDEBUG` は定義しない。実機は AtCoder と異なり、GCC 15.2 / Zen 系 CPU / macOS の速度は未測定。クラウド環境では外部負荷による wall time のばらつきがあるため、別セッションで得た絶対時間同士は比較しない。

他のローカル測定・コンパイルを止めた時間帯に、15 標本で計測した。結果の全 60 ケースは `bitwise_transforms_results.csv` に保存。主な結果は以下のとおり（ms）。

| 型・処理 | n = 2^18 旧 | 新 | 倍率 | n = 2^20 旧 | 新 | 倍率 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| mint / Walsh | 6.428 | 1.216 | 5.29x | 30.939 | 6.481 | 4.77x |
| mint / subset zeta | 3.949 | 0.804 | 4.91x | 33.249 | 7.178 | 4.63x |
| mint / subset Möbius | 4.064 | 0.881 | 4.61x | 37.214 | 7.087 | 5.25x |
| mint / superset zeta | 4.105 | 0.992 | 4.14x | 30.295 | 7.980 | 3.80x |
| mint / superset Möbius | 4.403 | 1.607 | 2.74x | 23.459 | 8.046 | 2.92x |
| uint64 / Walsh | 2.881 | 1.391 | 2.07x | 14.260 | 6.894 | 2.07x |
| double / Walsh | 2.499 | 1.189 | 2.10x | 19.872 | 8.833 | 2.25x |

`n=64` と `n=1024` も確認した。最初の `n=1024` 測定では double の superset Möbius が 4.350 → 4.795 μs（9.3% 遅い）だった。31 標本、各 256 回で追加確認すると 5.339 → 2.977 μs（1.79x）となり、この退行は再現しなかった。追加測定も `bitwise_transforms_small_repeat.csv` にそのまま保存した。小サイズでは計時・呼び出しの固定費や環境負荷の影響が大きく、すべての型・サイズでの高速化は保証しない。

## Library Checker の AC 実装との比較

2026-10-17 に live API の `status=AC&order=+time` で取得した先頭提出を調査した。

- [AND #409492 / Aiyiyi](https://judge.yosupo.jp/submission/409492): AC、11 ms。AVX2 の 3 段融合、L1 用の小区画、列方向処理と band の配置調整を使用し、入力処理・畳み込み・出力をまとめて最適化している。
- [XOR #409698 / Aiyiyi](https://judge.yosupo.jp/submission/409698): AC、13 ms。符号付き 64 bit の中間値、AVX2 の 4 段融合、手書きの radix-16 asm、転置、遅延剰余計算を組み合わせている。
- [XOR #352927 / adamant](https://judge.yosupo.jp/submission/352927): AC、29 ms の既存実装も確認した。コンパイル時サイズによる再帰的な変換と AVX2 指定を使っている。検索結果で先頭に出たが、live API では現時点の最速ではなかった。

今回も stage fusion と cache locality の方向性を採った。一方、上記最速提出の mod 998244353 専用処理・Linux I/O・配列の再配置・畳み込み全体の融合は、汎用 `vector<T>` の in-place 変換 API にそのまま導入できない。提出時間は入出力込みの Library Checker の値であり、今回のカーネル測定とは比較しない。これらの提出そのものとのローカル速度比較や、新実装のオンライン提出は行っていない。

最速 2 提出の live API 応答の AC 状態・提出情報・ソース SHA-256 は `bitwise_transforms_reference.json` に保存。ソースを再取得するには以下を使う。

```sh
python3 test/benchmark/bitwise_transforms_reference.py /tmp/bitwise-reference \
  --metadata /tmp/bitwise-reference.json
```

[AtCoder の公開コンパイル設定](https://img.atcoder.jp/file/language-update/2025-10/language-list.html)

## 検証

`test/unit/bitwise_transforms.cpp` は空配列、長さ 1、block 境界の両側、`2^16` までの全 2 冪を新旧で比較する。4 種類の zeta / Möbius と Walsh を、64 bit 整数、8 bit 整数、mint、float / double、非可換な模擬演算型で確認する。float / double はビット完全一致。さらにコピー不可の zeta 要素、default constructor を持たない型、演算結果を代入できるがそこから構築できない Walsh 要素、`vector<bool>`、signed zero / subnormal を確認する。AND / OR / XOR 畳み込みは 1025 要素まで愚直解と比較する。

```sh
g++ -std=gnu++23 -O2 -march=native -ftrivial-auto-var-init=zero -I . \
  test/unit/bitwise_transforms.cpp -o /tmp/bitwise-transforms-test
/tmp/bitwise-transforms-test
```

上記の focused test は成功。推移的な利用先 `test/unit/transforms.cpp` と `test/unit/convolution.cpp` も、通常ビルドと ASan / UBSan の aggregate 検証で成功した。検証ログは `results/portable_optimization/all-sanitize.log`。macOS / Clang の実行確認はこの環境では行っていない。
