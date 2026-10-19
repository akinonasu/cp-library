# Constant 入力の追加検証

この資料は `bd29f70c70e24e27770b9e9a3777b6cf85664b63` を上流基準とした Round 4 の固定版へ、constant 入力の検出・積・専用 unit を追加した時点の記録である。その後の上流取り込みとは別に保持する。

`scope-final-audit.json` と `scope-audit.json` はともに passed。新しい GCC/Clang/Sanitizer 実行は次の範囲に限り、前の全 27 unit・22 公式問題を再実行していない。

| 確認 | 新実行 | exact 証拠の再利用 | 論理上の対象数 |
|---|---:|---:|---:|
| GCC 14 normal | 4 headers + fps.cpp | 新 constant unit 1 | 6 |
| Clang 19 normal | 4 headers + 2 units | 0 | 6 |
| GCC ASan/UBSan | fps.cpp | 新 constant unit 1 | 2 |

4 headers は新 constant helper、fps_convolution、convolution/convolution、fps/fps。専用 unit は public convolution/FPS integration、所有 NTT との比較、lazy 入力、判定境界、末尾の fallback、guard page を確認する。

`selection-full.json` は前の固定版の final state に対する CI の差分 inventory。headers/bundles 21、unit 28、verification 24 である。前回差分にあった unbalanced standalone は今回は変更がなく、新 constant standalone と入れ替わるため bundles は 21 のまま。実行用の `selection-core.json` は理由を記録して 4 headers・2 units・Sanitizer 2 units に絞り、compiler ごとの実実行対象を別 selection に保存した。

再利用した 2 件は compiler 実体 SHA と resolved version、DEBUG=1 を含む flags、unit source、全 24 実依存、元の `.d`、binary、log、compile/run の return code が最終版と一致する。project root と unit source の場所だけを対応づけ、C++ 内容の正規化は行っていない。`reuse-assessment.json` がその binding を保持する。元の private 証拠は `../fps_convolution_constant_round4/private_validation/` にある。

新しい 14 回の実 `-MM` scan と全ログを保存し、対象選択・compiler・source・実依存・再利用証拠の前後 hash を確認した。旧 snapshot の依存 172 ファイルは不変で、fps_convolution から追加 include と dispatch の 2 行を戻すと旧 source SHA に完全一致することも runner が確認している。

ドキュメント側は同じ folder の `docs_*.json/log` を参照。生成 277、metadata 152、affected bundles 21 が成功し、source 601 ファイルが前後で一致した。公式 convolution 全 53 入力と formal 16 問の比較は親の別記録であり、この core runner には含めていない。

保存済み成功の再利用には記録された private artifact が必要となる。独立した環境で両 unit を新しく確認する場合は、`test/verify.py --unit-only --cxx CXX --selection selection-core.json`、Sanitizer は追加の `--sanitize` を使い、新しい出力先へログを保存する。元の raw、過去の gate、selection を上書きしない。

この実行終了後の上流取り込みによる binding は親の別 audit を参照し、この固定版の成功と混ぜない。
