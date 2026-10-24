---
title: 畳み込みの型別振り分け
documentation_of: //components/convolution_dispatch.hpp
---

[畳み込み](convolution.md)と派生する畳み込みが共有する部品です。通常は `convolution/convolution.hpp` を読み込みます。

`convolution(a,b)` は同じ要素型の `vector<T>` を受け取り、積の係数を `vector<T>` で返します。空入力があれば空配列を返し、それ以外の長さは `a.size()+b.size()-1` です。要素型ごとの利用条件は公開側の畳み込みと同じです。

法 998244353、その他の固定法 modint、整数、浮動小数点・複素数、その他の環に応じて演算を選択します。FPS のクラス自体は読み込みません。

出力長を $$N$$ として、NTT・FFT を使う型では $$O(N\log N)$$ 時間・$$O(N)$$ 追加空間です。汎用の環では、入力長を $$n,m$$ として $$O(nm)$$ 時間・$$O(n+m)$$ 出力空間です。
