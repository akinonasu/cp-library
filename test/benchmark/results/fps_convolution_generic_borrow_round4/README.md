# 汎用 NTT の結果配列借用の追加検証

root の private `fallback_borrow` を独立レビューした。計時は行っていない。採用後の汎用借用コンストラクタと `run_borrowed_as<ntt::Convolution>` に、新たな API、境界、生存期間、const、alias の問題は見当たらなかった。

- 既存 `Convolution(n,m)` は `nullptr` で委譲し、所有配列の経路を保つ。borrowed 版は A の所有を移さず、B と根表だけを所有する。
- A のアクセスは既存の GCC may-alias SIMD、assembly、`memcpy` に限られる。存命中の `Mint[]` を `uint32_t` scalar 参照として読み書きする経路はない。これは既存 Product と同じ GCC 用条件であり、portable C++ だけによる型 aliasing の主張ではない。
- 色調整は最大 `16*63=1008` words。追加 1024 words の中で B と二つの根表が収まる。alignment は A が 32 bytes、B と根表が 64 bytes を保つ。
- `run_borrowed_as` は `len+23` 個の `Mint` の生存期間を開始し、offset が 0〜7 words なので A には常に `len+16` 個以上が残る。末尾を含めた零初期化を維持する。`memmove` は公開サイズだけを移動し、借用 Plan の破棄は A を解放しない。
- 入力二本のコピーが計算より先に完了し、公開入力を書き換えない。同じ入力を二回渡す場合も安全である。追加した overload と template は内部だけで、公開 signature と Clang / Mac fallback を変更しない。

拡張 unit は既存 Product の guard 検証を保ち、汎用 Plan の変換長 64 から大入力まで、奇数 / 偶数 log、両辺 sparse、A または B が dense、入力交換、同一入力、零、遅延 Montgomery 値、canonical 出力を確認する。借用先は 0 mod 64 と 32 mod 64 の両方を用いる。末尾には guard page を置き、32 mod 64 の場合の最小限の余分な半 cache line は canary を検査する。借用が要求する末尾 16 words の零も確認する。元の入力の内部ページを read-only にして、全 `.v` が保存されることも照合する。

GCC 14 の通常 Werror、ASan / UBSan、GCC 13 Werror がすべて成功した。コンパイルと実行は共有 `/tmp/cp-fps-tuning.lock` 内。コマンド、source / header / binary SHA-256 は `checks.json`。`fps_convolution_borrow.cpp` が最終採用用 unit である。

既存の 53 公式入力の検証結果を再利用した。`fallback_first/result.json` に記録された borrowed の全 109 source 依存、driver、binary の SHA-256 が今回の private tree / binary と一致し、53 件成功の記録も確認した。対応を `official_reuse.json` に保存したため同じ 53 件は再実行していない。

private の検証 header SHA は `9491e2e954485197ff06ebfd6a84847a5d743c3fc2d826de623212ab421e4985`。最終統合との差は先に採用された unbalanced の include と dispatch だけであり、`private_to_integrated.patch` に保存した。最終統合状態の公式全 53 入力は root が別途確認する。

unit SHA は `3c5592e0e78d305cd3284393fc0708586fcfbb162df29ae134520c052a13998f`。production header や unit への変更はこの担当で行っていない。root が配置した unit の SHA が上記と一致することも確認した。
