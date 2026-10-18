---
title: FPS の一括入出力
documentation_of: //components/fps_io.hpp
---

Library Checker ドライバーの係数読み書きに使う内部部品です。Linux x86_64 / GCC 用に定義され、呼び出し側で AVX2 対応を確認します。他の環境では通常の整数入出力を使います。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `read_coefficients(input,out,n)` | 入力から n 個の FPS 係数を読み取る | 入力文字数＋n に比例 |
| `write_coefficients(output,in,n)` | n 個の係数を出力する | $$O(n)$$ |

関数は `fps_detail` 内にあり、FPS の入出力をまとめて処理します。通常は [Fast I/O](https://qandori.github.io/cp-library/components/fast_io.hpp.html) の `read`・`write` を使います。

`fps_detail::read_coefficients(input,out,n)` は `FastInput` から符号なし 32 bit 整数を `n` 個読み、`vector<Mint>` の末尾へ追加します。大きな入力ではトークン境界で 4 ストリームへ分け、各ストリームから 2 トークンずつ並行して変換します。すべての桁・区切り・uint32 の上限を確認してから結果を採用します。入力の末尾や不規則な空白では既存の整数読み込みに戻ります。バッファの外を読まず、利用者の入力に padding は要求しません。変換した整数は 64 個ずつ Montgomery 表現へ変換し、生成済みの `Mint` オブジェクトとして追加します。

`fps_detail::write_coefficients(output,in,n)` は `n` 個の係数を `FastOutput` へ書きます。Montgomery 表現を 8 値ずつ解除し、16 値ずつ SIMD で整形します。各係数を 9 桁分の領域に右詰めし、余白は空白とします。最後は改行し、`n=0` は改行のみです。25600 値ごとの大きなブロックは、先行する出力バッファを必要に応じて flush してから直接書きます。短いブロックは通常の出力バッファを使います。部分的な書き込みは継続し、書き込みに失敗した場合は `std::runtime_error` を送出します。

作業領域は呼び出しごとに所有し、別々の `FastInput`・`FastOutput` の処理は共有する可変バッファを持ちません。同じストリームを複数スレッドで同時に使うことは認めません。

`FastInput`・`FastOutput` の既存 API と係数の値は維持し、通常の `FastOutput::write` の形式は変更しません。これらの関数は通常の整数入出力と同様、各トークンが指定の整数型に収まる入力を前提とします。

時間は $O(n)$、追加の作業領域は $O(1)$ です。出力配列の容量不足時は $O(n)$ の確保が必要です。
