---
title: FPS の一括入出力
documentation_of: //components/fps_io.hpp
---

法 998244353 の共有 `mint` の係数列をまとめて読み書きする部品です。出力には [FPS の一括出力](fps_write.md) を読み込みます。出力だけが必要なら `components/fps_write.hpp` を直接 include すると、入力処理を読み込まずに使えます。

`read_coefficients` は Linux x86_64 / GCC 用に定義されます。呼び出し側で AVX2 対応を確認してから使ってください。他の環境では `FastInput::read` と `mint::raw` などで係数を読みます。`write_coefficients` は Mac を含むすべての対応環境で使え、CPU の選択は自動です。

| 操作 | 内容・返り値 | 計算量 |
| --- | --- | --- |
| `fps_detail::read_coefficients(input,out,n)` | `FastInput& input` から `size_t n` 個を読み、`vector<mint>& out` の末尾へ追加する。返り値は `void` | $$O(L+m)$$ |
| `fps_detail::write_coefficients(output,in,n)` | `FastOutput& output` へ、`const mint* in` の先頭 `size_t n` 係数を出力する。返り値は `void` | $$O(n)$$ |

既定引数はありません。表の $$L$$ は読み進める文字数、$$m$$ は呼び出し前の `out.size()` です。容量が足りている場合、読み込みは $$O(L)$$ 時間です。通常の整数入出力には [Fast I/O](fast_io.md) の `read`・`write` も使えます。

`read_coefficients(input,out,n)` は、入力・追加先・係数数の順に指定します。空白区切りの十進整数が `n` 個以上存在し、各値が `uint32_t` に収まることを前提とします。各整数を法 998244353 の値として読み、既存の `out` の要素を保持して末尾へ追加します。零や同じ係数があっても入力順を保ちます。`n=0` は何も追加せず、入力も消費しません。指定した個数の後のトークンは、続けて `input.read(x)` などで読めます。

大きな入力ではトークン境界で 4 ストリームへ分け、各ストリームから 2 トークンずつ並行して変換します。すべての桁・区切り・`uint32_t` の上限を確認してから結果を採用します。入力の末尾や不規則な空白では既存の整数読み込みに戻ります。バッファの外を読まず、利用者の入力に padding は要求しません。変換した整数は 64 個ずつ Montgomery 表現へ変換して追加します。

`n > out.max_size()-out.size()` なら、読み進める前に `std::length_error` を送出します。入力元の寿命は読み込みが終わるまで保ち、同じ `FastInput` を呼び出し中に別の処理から使わないでください。

`write_coefficients(output,in,n)` の引数順序は出力先・係数列の先頭・係数数です。入力を変更せず、アラインメントや末尾の padding も要求しません。`n=0` は `in=nullptr` でもよく、改行だけを出力します。出力書式・エラー処理・使用例は [FPS の一括出力](fps_write.md) にまとめています。

入力・出力ともに、配列以外の追加作業空間は $$O(1)$$ です。入力先の再確保が必要な場合は $$O(m+n)$$ の空間を使います。作業領域は呼び出しごとに所有し、共有する可変バッファはありません。
