# 軽量 FPS を使う畳み込み

2026-10-22。通常版の七つの畳み込みを、通常 FPS の内部 API に依存せず使えるようにした。各 `convolution/*_light.hpp` は直接 include でき、通常版と併用できる。コードは通常の ASCII の C++ で、圧縮マクロを使わない。

係数法が 998244353 の畳み込みは軽量 FPS を使う。一般の係数型は従来どおり CRT・FFT・環の積へ振り分ける。巡回・乗法・順序制約付きの畳み込みはこの入口を使う。オンライン・多変数畳み込みは、軽量 NTT の変換と周波数上の積を使う。軽量 NTT の逆変換は正規化済みなので、通常 NTT の後処理の係数を重ねて掛けない。

手法は対応する通常版を引き継いでいる。オンライン計算と順序制約付き計算の参考実装は、既存の [オンライン畳み込みの調査](online_convolution.md)・[順序制約付き畳み込みの調査](ordered_convolution.md) にある。通常の積の変換は [軽量 FPS の高速化資料](fps_light_speed.md) の実装を使う。

乗法畳み込みの添字法は 32 bit なので、原始根探索に使う因数分解は試し割りとした。64 bit 整数全般を扱う因数分解への依存を除け、元の入力範囲を保ちながらコードを小さくできる。原始根を求めるための因数分解には別途 $$O(\sqrt P)$$ 時間を使う。

公式入力は通常畳み込み53件、オンライン53件、乗法40件、多変数17件を GCC・Clang で実行し、326実行すべて成功した。巡回と順序制約付き畳み込みは独立した愚直計算にも照合した。単体テストは空列、異なる長さ、二乗、負の値、複素数、別の modint、一般の環、2冪の前後を含む。オンラインの更新と `pending()` も照合した。

通常版を先に include する場合と後に include する場合、ACL 型とグローバル名前空間に定義した係数型の併用も確認した。内部呼び出しを名前空間で修飾し、ADL による通常版との競合を避ける。GCC・Clang の単独 include、Clang ASan/UBSan も成功した。

検証後の最終ヘッダーのハッシュとコマンドは次の記録に保存した。

- [最終ヘッダー・単独検証](results/fps_light_natural/convolution-final-checks.json)
- [単体・sanitizer](results/fps_light_natural/convolution-validation.json)
- [通常版との併用](results/fps_light_natural/convolution-coexist.json)
- [公式問題](results/fps_light_natural/convolution-official.json)
- [提出コード](results/fps_light_natural/convolution-drivers.json)
- [標準展開とサイズ](results/fps_light_natural/convolution-bundle.json)

サイズと FPS 全体の依存検査は [全体の資料](fps_light_natural.md) にまとめた。ここでは正しさとコード長を検証しており、judge の順位や通常版との速度差は測定していない。
