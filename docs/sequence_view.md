---
title: Sequence View
documentation_of: //components/sequence_view.hpp
library_category: components/string
---

`vector<char>` や `span<char>` を、長さを保ったまま `string_view` で参照する内部部品です。文字列アルゴリズムが、文字列用の操作名のまま NUL を含む配列も受け取るために使います。

`sequence_view_detail::CharSequence<R>` は、`const R&` から `span<const char>` を作れ、`string_view` へ暗黙変換できない型であることをコンパイル時に判定します。`string`・`string_view`・文字列リテラルは従来の `string_view` 引数へ渡すため、対象から除きます。

`sequence_view_detail::char_view(const R& s)` は、同じ記憶領域と長さを参照する `string_view` を返します。`R` は上の条件を満たす型とします。`char_view(string_view s)` は文字列の参照をそのまま返し、`string`・文字列リテラルも渡せます。引数の既定値はありません。空列も受け付け、重複した値と NUL は変更しません。返り値は元の配列を参照するため、その配列の要素の移動・再確保・破棄後には使えません。

コピー・メモリ確保は行いません。`char_view` の時間・追加空間はともに $$O(1)$$ です。型の条件判定に実行時の時間・空間は使いません。
