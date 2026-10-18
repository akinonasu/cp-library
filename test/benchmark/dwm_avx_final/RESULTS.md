# Dynamic Bit Vector の祖先更新

2026-10-18。比較元は `986b5b17c622a2c999ec20e2bf5dfb1c8ecd0cb9`。

`move_rank` の祖先にある累積サイズ・1 の個数の区間更新だけを、
AVX2 / x86_64 / Branch=8 の場合に 8 要素まとめて処理する。
`node.count` より後の非アクティブ要素は変更しない。
子の探索、葉の移動・分割・併合、API、既定のテンプレート引数、メモリ配置は変更しない。
AVX2 がない環境、ARM、その他の Branch はスカラー処理を使う。

## 内部での候補選別

GCC 14.2、AMD EPYC 9V74、CPU 0 固定。
`-O2 -std=gnu++23 -march=native -ftrivial-auto-var-init=zero
-DONLINE_JUDGE -DATCODER -fno-lto`、assert 有効。
uint32_t / Bits=20、乱数種 20261017、一様な値・位置、Q=20,000。
構築は各反復で空から insert し、構築・結果ハッシュ・最終配列の検査は計測外。
AAAA / ABBA / BAAB の固定順序、各プロセス 4 反復。下表は 2 ブロックの B/A 時間比の平均。

| n | set | 4 操作混合 | access 対照 |
|---:|---:|---:|---:|
| 10,000 | -19.09% | -9.31% | -7.94% |
| 100,000 | -20.77% | -8.49% | -1.16% |

混合は access / set / kth_smallest / range_freq の 4 操作。
同一バイナリ A/A の最大・最小比は約 1.02–1.09、個々の計測は約 0.025–0.29 秒と短い。
access の差は実装上の改善とは判断していない。これは候補選別であり、AtCoder の性能や最速性の証明ではない。
各ケースの全出力・最終配列は独立した vector oracle と一致した。
既存の Bit Vector テストと追加した境界移動テストは native / 明示的 scalar で成功した。
ローカルの sanitizer は既知の LeakSanitizer 実行環境制限があるため、成功扱いにせず CI で確認する。

## 最終比較と互換性

同じディレクトリの `run.py` と README の固定条件で、GCC 15.2 の native / scalar を
同じマシンで比較する。n=100,000 の set / 混合には通常の一括構築も含める。
結果は Actions の通常ログに残し、小さな悪化や計測下限未達の行も削除しない。
この資料の内部測定とは Q・反復数・コンパイラが異なるので、数値を直接合算しない。

通常の必要な GCC / Clang テスト、ASan / UBSan、Apple Clang ARM64、native / scalar、
関連 judge とドキュメント検査は PR の正確な最新版で確認する。
追加費用を避ける opt-in CI では永続キャッシュ・artifact を使わず、ドキュメントは
ビルド・検査するが Pages への自動公開は行わない。最終測定・CI の結論は PR に記録する。
