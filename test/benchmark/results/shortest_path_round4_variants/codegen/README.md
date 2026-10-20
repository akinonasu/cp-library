# CSR 候補の予備測定

GCC 15.2、Linux x86_64、Core i7-13700F、CPU 2 固定、共有 lock `/tmp/cp-wm-opt.lock` の下で比較しました。フラグ・環境・各候補の hash と元 commit は [manifest.json](manifest.json) にあります。`old` は `f4aa743`、`current` と他の候補は `bd29f70` が基準です。

実提出コードの比較は各ケース 1 回 warmup、順序を変えた 7 回の別プロセスの測定です。入力・出力・バイナリは Linux `/tmp` に置き、全体時間を測ります。warmup の距離一致を確認しています。最終版の経路 checker と型・境界の検証は正式測定の資料を参照してください。

[コード生成の確認](codegen.json)では、入力・構築・探索の分離と旧キューへの差し戻しも調べました。main・入力・探索の stack frame の大きさは同じでした。inline の変化はありましたが、差し戻し・noinline 候補は安定した改善を示さず、前回の退行の原因とは判断していません。

| 記録 | 比較 |
| --- | --- |
| [measure.json](measure.json) | 入次数判定の後回し、入力・構築・探索の noinline、旧キュー |
| [memory.json](memory.json) | Arc 確保順序、CSR scatter の 8・32・64 辺先の先読み |
| [broader.json](broader.json) | 先読みの他の代表 8 ケースへの影響 |
| [combo.json](combo.json) | 小さいグラフで先読みを省く版、入次数判定との組合せ、適応 32 bit offset |
| [boundary.json](boundary.json) | 頂点数 4096–65536、平均出次数 2・16 の合成グラフ |
| [boundary_lower.json](boundary_lower.json) | 頂点数 512・1024・2048 の合成グラフ |

`prefetch_*_offset` は先の辺の出発点に対応する offset へ書き込み用先読みを行います。`prefetch_*_arc` は同じ offset の現在値から Arc の位置も先読みします。実際の配置は常に現在の offset を減らして行います。未来の辺が残っているため先読みする index は Arc 配列内にあります。重複する出発点があっても正答に影響しません。

`guard32` は頂点数 8192 以上の通常 CSR 配置だけに 32 辺先の先読みを使います。最後の 32 辺と、それ未満のグラフは元のループです。一本道の直接配置は変えません。`deferred_guard32` は入次数の重複判定も入力 callback から配置前の走査へ移します。`narrow` は offset が収まる場合だけ 32 bit にし、超える場合は `size_t` を使います。後者は Dijkstra の行参照にも一度だけ幅分岐を入れる、大きな変更を伴う候補です。

合成グラフは seed `20261019` の一様乱数で出発点・到着点・重みを生成します。入力生成を計時外にし、CSR の span 構築を同じプロセス内で反復します。各プロセス 2 回 warmup、31 回計時し、順序を変えた 3 プロセスを使います。破棄・辺数の checksum は計時外です。allocator と cache が温まった条件なので、公式入力の初回構築時間とは直接比較しません。小さいグラフで先読みを省く条件は性能上の目安です。頂点数だけで全 CPU・全密度の cache 境界が定まるという主張はしていません。

候補の全依存先を重複して保存する代わりに、[patches](patches) と commit・対応表・hash から、コンパイルしたソースを再構成できます。第三者の提出ソースは含みません。再構成には Python、Git、GNU patch を使います。

```sh
python3 test/benchmark/results/shortest_path_round4_variants/codegen/reproduce.py \
  --output .verify-helper/cache/shortest-path/round4/codegen-replay
python3 .verify-helper/cache/shortest-path/round4/codegen-replay/measure.py combo
python3 .verify-helper/cache/shortest-path/round4/codegen-replay/boundary.py
```

再構成時は元のソースと flatten 後のソースの SHA-256 を両方照合します。`scripts/prepare.py`・`scripts/variants.py` は元の候補生成スクリプトです。保存後の再構成には `reproduce.py` を使います。
