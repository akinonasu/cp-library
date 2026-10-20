# 原票の対応

- `final.json` / `final.log`: 採用した private 統合版、固定基準、未編集 409465 の 21 shuffle × 4 入力。warmup を含む全 checker、順序、ns 値、入力 / answer / 出力 / binary SHA-256。
- `initial.*`: 固定変換長 256〜16384 の初期 5 回選別。
- `confirm.*`: `4*bitceil(m)` / `8*bitceil(m)` の 21 回比較。
- `append.*`: 全長 zero/add から overlap/add + append への変更の 9 回比較。
- `measure_{initial,confirm,append,final}.py`: 各 raw JSON が記録する SHA-256 と一致する当時の harness。
- `driver_initial.cpp`: 初期 JSON に記録した SHA-256 と一致する driver。`driver.cpp` は動的変換長、`driver_append.cpp` は append 版。
- `shape*`: 18 形状の同一 process 内の補助 kernel 計測。trial 0 を warmup とし、残り 9 値の中央値を summary に保存。二つの method の出力中央係数も一致することを確認した。公式 whole 時間へ混ぜていない。
- `official_check.json`: private 統合版の公式全 53 入力。checker 順は input / actual / answer。
- `fps_convolution_unbalanced.hpp` / `.cpp`: 採用された component と production 形式 unit。repo に配置したファイルと byte 同一。
- `unit_final.cpp`: private helper のガード境界 / naive / lazy / swapped / sameinput / stride 境界を検証した source。
- `unit_header_before_format.hpp` / `format_only_header.patch`: GCC 14 sanitizer 時の header と、最終版への include 明示・空行だけの差。最終 header は GCC 13 と production 形式の GCC 14 でも確認した。
- `metadata.json`: 固定基準・候補の全 source SHA、コンパイラ / flags / CPU / OS、全 binary SHA、検証条件、historical harness と初期 driver の SHA 照合。
- `409465.cpp.gz`: gzip 展開後の SHA を metadata へ記録した未編集原本。
- `unbalanced.patch`: 固定基準への新 component と公開 dispatch の差分。root の別の高速化を含まない。
- `reproduce.py`: artifact 全 hash 確認、基準 archive、patch 復元、source 照合、同じ flags のビルド、任意の unit / 公式検証 / 21 回計時。
- `manifest.json`: 原票一式の SHA-256。manifest 自身は含めない。

root の後続変更を組み合わせた全体比較は別資料に保存される。このディレクトリの元の raw 値を後から統合状態の値で置き換えない。
