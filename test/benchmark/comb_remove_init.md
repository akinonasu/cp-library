# 二項係数の手動初期化の削除

2026-10-20。`math/combinatorics/comb.hpp` の `init_comb()` を削除し、自動初期化から階乗・逆階乗の共通生成処理を直接呼ぶ形に整理した。include 後の `comb`・`fact`・`finv` の使い方と、プログラム全体で1回だけ前計算する仕組みは維持する。

生成アルゴリズムと通常の二項係数の計算は変更していない。速度の比較測定は行っていない。[既存の手法比較と測定](comb_factorials.md)、[自動初期化の設計](comb_auto_init.md)を参照。

## 検証

WSL2 / Linux x86_64、GCC 15.2.0・Clang 21.1.8、`-std=c++2b -O2`。

- 関連する単体テスト4本。上限0・200・70000で、自動初期化した表と二項係数を照合。
- Clang ASan/UBSanで関連3本。
- 2つの翻訳単位を両方の順序でリンクし、初期化が1回だけであることと、`main` 前の利用を確認。
- 既定上限502022、C配列の型・大きさ、GCCのAtCoder相当のconstexpr制限・`NDEBUG`でも確認。
- ヘッダー・提出コードのBundle、説明内コード、yukicoder 3172の公開サンプル4件を両コンパイラで確認。
- 変更した説明ページを生成し、二項係数のBundleをコンパイル。
- 既存の階乗生成ベンチマークを両コンパイラで実行。旧実装との比較用には従来の初期化関数を指定できるようにした。

結果は [results/comb_remove_init](results/comb_remove_init) に保存。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --selection test/benchmark/results/comb_remove_init/selection.json
python3 test/verify.py --unit-only --sanitize --cxx clang++ --selection test/benchmark/results/comb_remove_init/selection.json
python3 test/benchmark/comb_auto_init_check.py --output test/benchmark/results/comb_remove_init
```
