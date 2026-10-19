# 読み取りレビュー

担当: `compose_reversion`。対象は初回の private `overlap.hpp`。後の append 版は結果の構築だけを既存の `append_fps_words<true>` に替え、入力表現、変換長、ブロックの切り方を維持した。

非空、`len` が 64 以上の二冪、`m<=len`（`stride>0`）、公開 native 経路の `n+m-1<=2^23` を満たす呼び出しでは、数学、範囲、生存期間、const、alias に懸念はない。

`y.val()` は通常剰余、`x.v` は正規化した Montgomery 表現なので出力も Montgomery の剰余であり、`result.v+out<2p` を一回の補正で加算できる。`count+m-1<=len` なので循環 wrap がなく、`p+count+m-2<=n+m-2` の出力境界に収まる。`x` / `y` は const の参照で、同一入力の場合も result / arena は独立している。Source が未使用の入力末尾を零として扱うため dirty padding の読み取りに依存しない。

production 化では変換長を外から無検証に受けず、上記条件、AVX2 実行時判定、platform guard を保証すること。大き過ぎる `bit_ceil` を避ける dispatch が必要になる。

採用版は変換長を内部で `8*bitceil(shorter)` とし、`shorter>32`、`longer>=32768`、`shorter<=longer/32` の条件でだけ公開経路から呼ぶ。全体の出力長上限により短辺は 254200 以下、変換長は最大 $$2^{21}$$ である。結果 append は予約容量に直接書かず、既存のバッチで `Mint` の生存期間を保つ。
