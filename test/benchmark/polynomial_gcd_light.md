# 軽量多項式 GCD の実装・検証

2026-10-22。通常版の [調査記録](polynomial_gcd.md) にある Library Checker
[409589](https://judge.yosupo.jp/submission/409589)、
[406000](https://judge.yosupo.jp/submission/406000) と、通常版の
`PolynomialHalfGCD::half` を参照した。多項式 GCD・拡張 GCD そのものに直接対応する
Library Checker の問題は見つからず、既存の調査と同様に Inv of Polynomials で検証した。

軽量版は、上位係数について再帰した変換行列を元の多項式に作用させる Half-GCD を使う。
行列は通常の多項式演算で表した 2×2 行列で、64 係数以下の区間だけ通常の互除法を使う。
専用 SIMD 互除法、変換キャッシュ、巡回畳み込みによる次数の切り詰めは含めず、
軽量 FPS の畳み込みを使う。計算量は O(M(n) log n) 時間、O(n) 追加空間。

新しい二つのヘッダーの合計は 3,827 B。通常の ASCII 識別子とタブ整形を使い、
ソースを圧縮するマクロは追加していない。通常版 FPS のヘッダーは読み込まない。

Linux x86_64 の GCC 14.2 と Clang 19.1.7 で単体テストと standalone コンパイルを実施。
単体テストでは独立した通常の多項式除算・互除法との一致、最高次係数の正規化、
入力の対称性、Bézout 恒等式、逆元の積、Half-GCD の変換行列と逐次互除法の行列の一致、
行列式 ±1 を確認した。零・定数・末尾の 0・共通因子・整除関係・疎な入力・
2 冪前後の係数数・別の modint 型を含む。Clang の ASan/UBSan でも確認した。

公式 Inv of Polynomials の 18 ケースについて、GCD、拡張 GCD、剰余環の逆元の各 API を
通る三つのドライバーを、両コンパイラーで公式チェッカーに照合した。合計 108 ケース。
記録は `results/fps_light_natural/polynomial_gcd_validation.json`。

検証中は `/tmp/cp-fps-tuning.lock`、`/tmp/cp-wm-opt.lock` の順で両方に共有ロックを取得し、
速度測定との同時実行を避けた。ここでは正当性とソースサイズを検証し、速度比較は未実施。
