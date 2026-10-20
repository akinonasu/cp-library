# Round 4 の固定版の検証

この資料は constant 入力用の追加変更より前に固定した版の検証記録である。検証の比較元は `bd29f70c70e24e27770b9e9a3777b6cf85664b63`。後続の変更にはこの成功記録を無条件に適用せず、変わる依存だけを別途確認する。

合否の結合 gate は `scope-final-audit.json`。GCC 14 と Clang 19 はそれぞれ standalone header 21 件と unit 27 件を通過し、GCC ASan/UBSan は focused unit 4 件を通過した。全ログの対象と順序を照合し、100 回の実 `-MM` scan と controls を含む 173 ファイルの SHA-256 が前後で一致した。private の先行テストは exact flags と compiler binding が一致しなかったため、再利用していない。

元の strict runner の `result.json` と `scope-audit.json` は `failed` のまま保存している。実行済みの 3 command はすべて return code 0 だが、GCC 実行開始後に測定資料内の複製 `convolution.test.cpp` を `.cpp.txt` へ改名し、CI が資料を追加 driver として拾う状態を解消したため、全体 inventory の fingerprint が変わった。これは production や実行対象 unit の変更ではない。

`scope-refinement.json` と `scope-final-audit.json` は、元の selection からこの 1 件だけが減り、headers/unit/sanitize/bundles の全対象・fingerprint・実依存が不変だったことを確認する。`selection.json` は書き換えず、最終 inventory は別名の `selection-final.json` に保存した。成功済みチェックを再実行していない。pragma や C++ source の正規化例外はない。

公式問題の結果は `judge_audit.json`。対象 24 driver のうち Library Checker 22 問・全 596 case が checker を通過した。Yukicoder 2 件は token がないため明示 skip。checker 引数は `input actual answer`。source 180 ファイルと公式 data/checker 1158 ファイルの SHA-256、actual dependency、selection、compiler、flags を前後で拘束し、case 数と final selection の一致も確認した。既存成功結果の reuse は行っていない。

ドキュメント側は `fps_round4_docs*.json/log` を参照。生成 276、library metadata 151、affected bundle 21 が成功し、凍結 source 598 ファイルが前後で一致した。

`baseline-provenance.json` は比較元の full git archive 3131 ファイルを拘束する。新しい作業領域へ同じ commit を `git archive` で展開し、その親へこの provenance を置けば、`test/benchmark/fps_round4_validation.py --work WORK --baseline-tree WORK/baseline --output NEW_OUTPUT --execute` で新しい記録を作れる。再測定時の対象・source・compiler・flags はその記録が拘束する。元の raw を上書きしない。

`core-manifest.json` はこの固定版の core 記録と helper の hash。judge と docs はそれぞれ専用 manifest を持つ。後続の追加変更の検証と、この履歴の原票を混ぜない。
