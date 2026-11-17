---
title: "Suffix Automaton（接尾辞オートマトン）"
documentation_of: //string/suffix_automaton.hpp
library_category: string/substring
---

末尾へ文字を追加しながら、部分文字列の検索、出現回数、相異なる部分文字列の数を求めます。NUL・128 以上の値を含む全 256 種類のバイトに対応します。

```cpp
#include "lib/string/suffix_automaton.hpp"

int main() {
    SuffixAutomaton sam("ababa");
    bool found = sam.contains("bab"); // true
    auto count = sam.count("aba"); // 2
    auto distinct = sam.distinct_count(); // 9
    sam.push_back('b');
    count = sam.count("ab"); // 3
    auto match = sam.longest_common_substring("zzbaba");
    // match.text_pos = 1, query_pos = 2, length = 4
}
```

| 操作 | 内容・返り値 |
| --- | --- |
| `SuffixAutomaton()` / `SuffixAutomaton(s)` | 空の文字列／`std::string_view` から構築 |
| `clear()` | 空へ戻す。確保済みの領域は保持 |
| `reserve(n)` | 長さ `n` の文字列に必要な領域を予約 |
| `push_back(c)` | `char` を末尾に追加し、追加後の `last()` を `int` で返す |
| `append(s)` | `std::string_view` を末尾へ追加。返り値は `void` |
| `size()` / `empty()` | 追加したバイト数を `int`／空かどうかを `bool` で返す |
| `distinct_count()` | 異なる空でない部分文字列の数を `std::uint64_t` で返す |
| `find(s)` / `contains(s)` | 部分文字列の到達状態を `int`／存在するかを `bool` で返す。不在の状態番号は `-1` |
| `count(s)` | 重なりを含む出現回数を `std::uint64_t` で返す。不在なら 0 |
| `longest_common_substring(s)` | 構築した文字列と `s` の最長共通部分文字列の位置・長さを `Match` で返す |

`Match` は `int text_pos, query_pos, length` を持ちます。構築した文字列の `[text_pos,text_pos+length)` と、引数の `[query_pos,query_pos+length)` が一致します。最長の候補が複数ある場合は、引数での出現位置が最小のものを返します。その部分文字列の構築文字列での出現位置も最小です。一致がなければ全フィールドが 0 です。

空文字列は常に存在し、`find("")` は `root`、`count("")` は `size()+1` です。`distinct_count()` には空文字列を含めません。

状態を使った DP や出現回数の伝播には次の API を使えます。

| 操作・定数 | 内容・返り値 |
| --- | --- |
| `root` | 初期状態の番号 0 |
| `node_count()` | 初期状態を含む状態数を `int` で返す |
| `last()` | 現在の文字列全体の状態を `int` で返す。空では `root` |
| `length(v)` / `min_length(v)` | 状態 `v` が表す部分文字列の最大長／最小長を `int` で返す |
| `suffix_link(v)` | その状態の最長文字列について、異なる状態に属する最長の接尾辞の状態番号を `int` で返す。初期状態では `-1` |
| `transition(v,c)` | 1 バイトの遷移先を `int` で返す。存在しなければ `-1` |
| `transitions(v)` | `(バイト,遷移先)` の `std::vector<std::pair<unsigned char,int>>` を返す。順序は未規定 |
| `walk(v,s)` | `v` から `s` を読んだ状態を `int` で返す。途中で遷移できなければ `-1` |
| `first_occurrence(v)` | 状態の最大長の文字列が最初に現れる `[l,r)` を `std::pair<int,int>` で返す。初期状態では `(0,0)` |
| `occurrences()` | 各状態の出現回数を `std::vector<std::uint64_t>` で返す。初期状態は `size()+1` |
| `prefix_nodes()` | 各非空接頭辞の状態番号を `const std::vector<int>&` で返す。要素 `i` は長さ `i+1` の接頭辞 |
| `order_by_length()` | `length` の昇順に状態番号を並べた `std::vector<int>` を返す |

状態は、本文のどの位置で出現が終わるかが全て同じ部分文字列をまとめたものです。例えば `ababa` の部分文字列 `b` と `ab` は、ともに0始まりの位置1・3で終わるため同じ状態になります。状態 `v` の出現回数は、長さが `min_length(v)` 以上 `length(v)` 以下の、その状態に到達するどの部分文字列にも共通です。

初期状態以外の suffix link は最大長がより短い状態を指し、文字の遷移は最大長がより長い状態を指します。状態番号の大小は suffix link の向きと一致するとは限りません。`order_by_length()` は suffix link の伝播や遷移上の DP に使えます。

出現回数は必要になったときに一括計算し、文字追加で無効化します。繰り返し取得しても回数を重複加算せず、取得後も文字を追加できます。状態番号は追加しても変わりませんが、suffix link・遷移先・最小長・出現回数は変わることがあります。`clear()` では状態番号もリセットします。コピーは独立します。

文字数・予約長は `(INT_MAX-1)/3` 以下とします。状態を引数に取る操作には `[0,node_count())` の番号を渡してください。最長共通部分文字列の引数は `INT_MAX` バイト以下です。削除や途中への挿入には対応しません。バイト列として処理するため、UTF-8 の文字単位ではありません。返した配列は値のコピーです。ただし `prefix_nodes()` の参照とその要素の有効期間には `std::vector` と同じ規則が適用されます。

文字数を $$N$$、検索する文字列の長さを $$M$$、登場するバイトの種類数を $$\sigma\le256$$ とします。構築・文字追加の合計は $$O(N\sigma)$$ 時間、保持空間は $$O(N)$$ です。`push_back` は、一連の追加にかかる合計時間から評価した1回当たりの時間（償却時間）が $$O(\sigma)$$、`transition` は $$O(\sigma)$$、`find`・`contains`・`walk`・最長共通部分文字列は $$O(M\sigma)$$ です。`count` は文字追加後の初回に追加で $$O(N)$$、以降は $$O(M\sigma)$$ 時間です。`occurrences`・`order_by_length` は $$O(N)$$ 時間・追加空間、`transitions(v)` は出次数に比例する時間・追加空間です。`reserve` は予約長と既存要素数に比例する時間、`clear` は $$O(N)$$ 時間、その他の取得は $$O(1)$$ 時間です。

`BasicSuffixAutomaton<T>` は整数列にも同じ操作を提供します。`T` は `bool` を除く 64 bit 以下の整数型です。負数・最小値・最大値を含め、値を変換せずに扱います。`SuffixAutomaton` は `BasicSuffixAutomaton<char>` の別名なので、既存の型名もそのまま使えます。

整数版の列引数は `span<const T>` で受け取り、同じ要素型の `vector<T>`・`span<T>` も渡せます。`push_back`・`transition` は `T` を受け取り、`transitions(v)` は `vector<pair<T,int>>` を返します。それ以外の操作名・返り値・位置・空列・重複出現・コピーの扱いは文字列版と同じです。`SuffixAutomaton` 自体にも `vector<char>`・`span<char>`・`span<const char>` を渡せます。

```cpp
#include "lib/string/suffix_automaton.hpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main() {
	std::vector<std::int64_t> text{-5, 1000000000000LL, -5};
	BasicSuffixAutomaton<std::int64_t> sam(text);
	std::vector<std::int64_t> pattern{-5};
	assert(sam.count(pattern) == 2);
	sam.push_back(1000000000000LL);
	pattern.push_back(1000000000000LL);
	assert(sam.count(pattern) == 2);
}
```

整数版について、追加した要素数を $$N$$、検索列の長さを $$M$$、追加済みの異なる値の数を $$\Sigma$$ とします。値が 0 種類・1 種類なら、以下の対数時間は定数時間として扱います。

| 整数版の操作 | 時間計算量 |
| --- | --- |
| 空の構築 | $$O(1)$$ |
| 列からの構築・一連の `push_back` / `append` | 合計 $$O(N\log\Sigma)$$ |
| `push_back(c)` | 償却 $$O(\log\Sigma)$$ |
| `transition(v,c)` | $$O(\log\Sigma)$$ |
| `find(s)` / `contains(s)` / `walk(v,s)` / `longest_common_substring(s)` | $$O(M\log\Sigma)$$ |
| `count(s)` | $$O(M\log\Sigma)$$、追加後の初回はさらに $$O(N)$$ |
| `occurrences()` / `order_by_length()` | $$O(N)$$ |
| `transitions(v)` | 出次数に比例 |
| `reserve(n)` / `clear()` | 予約長と既存要素数に比例 / $$O(N)$$ |
| その他の取得操作 | $$O(1)$$ |

整数版の保持空間は $$O(N)$$ です。`occurrences`・`order_by_length` は $$O(N)$$、`transitions` は出次数に比例する追加空間を使います。出現回数の初回計算は $$O(N)$$ 追加空間を使い、結果を次の追加まで保持します。それ以外の検索は $$O(1)$$ 追加空間です。
