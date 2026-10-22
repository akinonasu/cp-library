# 軽量 FPS の合成・射影・合成逆元

2026-10-22 に追加した通常の C++ 実装。共有実装は `components/fps_composition_light.hpp`、公開ヘッダーは `fps/compose_light.hpp`、`fps/power_projection_light.hpp`、`fps/compositional_inverse_light.hpp`。`LightFormalPowerSeries<T>` とその公開演算だけを使い、高速版の FPS・専用の列 NTT・blocked kernel を読み込まない。

## 手法

重み付き power projection は、逆順の重みを分子、$$1-y(f(x)-f(0))$$ を分母とする二変数 Bostan–Mori で求める。各段で $$x$$ の次数を半分、$$y$$ の次数を約 2 倍にする。二変数の係数を一次元へ埋め込み、軽量 FPS の通常の積を使う。分母の積は $$Q(z)Q(-z)=Q_{\mathrm{even}}(z^2)^2-z^2Q_{\mathrm{odd}}(z^2)^2$$ と分け、必要な変換長を抑える。非零の定数項は最後に階乗を使う二項変換で戻す。

合成はこの線形操作の転置。各段の分母を保持し、逆順に相関を計算して外側の係数を入力の重み側へ戻す。合成逆元は power projection と対数・指数による Lagrange 反転で求める。平方根分割で全ての小べき乗を保持する方法より、大入力に対して計算量を抑えることを優先した。

重み数または出力次数を $$n$$ とし、次の 2 冪を $$S$$ とすると、各段の畳み込みの変換長は高々 $$4S$$。$$n\le2^{21}$$ なら軽量 NTT の上限 $$2^{23}$$ 内に収まる。段 $$j$$ の列幅は高々 $$S/2^j$$、分子の列数は高々 $$2^j$$、分母は高々 $$2^j+1$$。積を取る前に末尾の零を除き、実際に必要な長さで変換する。

時間は $$O(n\log^2(n+1))$$、保持する各段の分母を含む追加空間は $$O(n\log(n+1))$$。定数項が非零の射影では、出力数を $$m$$ としてさらに $$O(m\log(m+1))$$ 時間、$$O(m)$$ 空間を使う。

## 参照

既存の [高速実装の調査](fps_composition_speed.md)・[power projection の調査](power_projection.md)で取得・比較した提出を参照した。今回はその大きな演算別カーネルを複製せず、二変数 Bostan–Mori、合成との転置関係、Lagrange 反転の高位の手順を軽量 FPS 上で実装した。

| 問題 | 参考提出 |
| --- | --- |
| 合成 | [409336](https://judge.yosupo.jp/submission/409336) |
| 合成 Large | [409535](https://judge.yosupo.jp/submission/409535) |
| 合成逆元 | [409399](https://judge.yosupo.jp/submission/409399) |
| 合成逆元 Large | [409549](https://judge.yosupo.jp/submission/409549) |

取得時刻・原本ハッシュ・専用カーネルとの速度比較はリンク先の記録にある。参考提出の表示時間は、この軽量実装の実行時間を示すものではない。

## 検証

`test/unit/fps_light_composition.cpp` で空入力、0・1 係数、2 冪の前後、入力長と出力長の違い、疎な入力、非零の定数項を持つ重み付き射影を愚直な積と照合する。513 係数までは合成と射影の双対性、合成逆元を両側から合成した恒等式、係数版と重み付き版の一致も確認する。

GCC・Clang と `mint`・ACL `modint998244353` の 4 通り、Clang ASan/UBSan の 1 通りを検証した。公式データは合成 27 件・合成 Large 32 件・合成逆元 23 件・合成逆元 Large 28 件を両コンパイラで実行し、計 220 実行が公式 checker を通過した。コマンド、ソース・入力・期待出力・実行ファイルのハッシュ、検証中の経過時間は [原票](results/fps_light_composition/validation.json) に保存した。経過時間は並行した検証中の値であり、judge の順位や他実装との速度比較には使わない。
