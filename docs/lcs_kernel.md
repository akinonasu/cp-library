---
title: "LCS の内部計算"
documentation_of: //components/lcs_kernel.hpp
library_category: components/string
---

[LCS](lcs.md) が文字列・整数列の計算で使う内部部品です。通常は `lcs_length`・`lcs` を使ってください。以下の名前は `lcs_detail` 内にあります。

2 列 `s,t` の長さをそれぞれ $$n,m$$ とし、`words = ceil(m/64)` とします。

| 処理 | 内容・返り値 | 時間計算量 |
| --- | --- | --- |
| `near(s,t,out)` | 挿入・削除が合計 8 回以下で同じ列にできる場合に LCS 長を `int` で返す。それ以外は `-1` | $$O(n+m)$$ |
| `MaskPositions(t,workspace,reverse)` | 各番号が現れる位置をまとめる。`reverse=true` では逆順の位置を使う | $$O(m)$$ |
| `profile_dense(s,positions,words)` / `profile_sparse(s,positions,words)` | `s` と `t` の各接頭辞の LCS 長をビット列で返す | $$O(m+n\lceil m/64\rceil)$$ |

`near` は同じ型の `std::string_view` または `std::span<const T>` を受け取り、要素を `==` で比較します。各列の長さは `INT_MAX` 以下にしてください。`out` は省略できます。指定する場合は有効な文字列または同じ要素型の `std::vector<T>` へのポインターとし、成功時に LCS を 1 つ末尾へ追加します。失敗時は変更しません。出力先は入力列と領域を共有しないでください。置換は挿入と削除の 2 回として数えます。空列・重複にも対応し、両方が空なら 0 を返します。出力先の領域を除く追加空間は $$O(1)$$ です。

`MaskPositions` の `t` は `std::span<const std::uint32_t>` です。番号と `workspace` は [整数列の比較用ビットマスク](sequence_bit_masks.md) と同じ条件で用意し、`reverse` は省略せず指定します。作成後は元の `t` を保持する必要はありません。同じ作業領域を使う他のマスクを同時に作らず、作業領域は破棄まで保持してください。`MaskPositions` はコピーできず、作成・保持・破棄には $$O(m)$$ 時間・空間を使います。

行計算の `s` も同じ番号を使う `std::span<const std::uint32_t>` とします。返り値は `std::vector<std::uint64_t>` です。`0 <= j < m` について、返り値の `j/64` 番目の要素の `j%64` ビットが 0 なら、`t` の長さ `j` の接頭辞から長さ `j+1` の接頭辞に伸ばしたときに LCS 長が 1 増えます。列末尾を超えるビットは 1、`m=0` なら空の配列です。`MaskPositions` ごとに行計算は 1 回とし、計算後は破棄してください。行計算の追加空間は $$O(m)$$ です。作業領域そのものの保持量は、扱う番号の種類数を `alphabet` として $$O(\mathrm{alphabet})$$ です。

`profile_dense` のテンプレート引数は `<reverse=false,backend=0>`、`profile_sparse` は `<reverse=false>` です。`reverse=true` は `s` を逆順に読み、`MaskPositions` も `reverse=true` で作ります。`backend` は 0 が自動選択、1 が通常演算、2 が AVX2、3 が AVX-512F です。2・3 を明示する場合は、実行する CPU が対応していることが必要です。通常は自動選択を使ってください。
