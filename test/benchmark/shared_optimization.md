# 高速化の共有

2026-10-19。基準は `56a7b9b`。ヘッダーを横断して、同じ処理を行う部分と既存の共通部品を比較した。

## 調査結果

| 再利用元 | 適用先・候補 | 判断 |
| --- | --- | --- |
| Run 列挙の前方・後方 AVX2 比較 | LCS の長さ計算・復元、編集距離の共通端の除去 | 採用。SIMD カーネルを `components/sequence_compare.hpp` に分離し、3 ライブラリで共有 |
| mod 平方根の `Montgomery32` | 素数判定・素因数分解の 32 bit 入力 | 試作して撤回。汎用設定で改善が小さく、64 bit 入力の判定では悪化も残った |
| FPS の階乗・逆階乗生成 | 二項係数の前計算 | `fps_factorials.hpp` を既に共有。追加変更なし |
| Fenwick 木の共通祖先までの同時更新 | 重み付き Wavelet Matrix | `FenwickSum::add_pair` を既に利用。追加変更なし |
| 無向 CSR | Low-Link・二辺連結成分・全方位木 DP、LCA の構築 | 前者は共有済み。LCA は出力配列を作業領域にも使うため、単純な置換で速度が上がるとは判断せず変更なし |
| Dijkstra の圧縮 Radix Queue | 最小費用流 | 通常の増加路探索では再利用候補。反復ごとの初期化と、容量スケーリングの 128 bit 距離を扱う設計が必要なため、今回は未適用・未測定 |

## 参考実装

Library Checker 公式 API から、AC 実行時間順の先頭 3 件を取得した。一覧と各ソースの SHA-256 は `results/shared_optimization/references.json` に保存した。ジャッジ時間と下のローカル測定時間は直接比較しない。

- Run Enumerate: [406234](https://judge.yosupo.jp/submission/406234)・[403379](https://judge.yosupo.jp/submission/403379) は取得時 12 ms。[191550](https://judge.yosupo.jp/submission/191550) は 22 ms。先頭の実装は Lyndon に基づく探索と分割統治を持ち、上限を明示した 8 byte 比較と AVX2 の前方・後方比較を共有する。短い不一致を先に除外し、長い一致だけ SIMD で処理する方針を比較した。既存の Run 列挙にも同じ方針があり、今回はカーネルを別ライブラリへ展開した。
- Primality Test: [405740](https://judge.yosupo.jp/submission/405740) は取得時 34 ms。ハッシュで選ぶ底の表と 2 系列の累乗を使う。表に基づく判定への変更は行わず、既存の Montgomery 演算を幅別に再利用する候補だけを比較した。
- Factorize: [405743](https://judge.yosupo.jp/submission/405743) は取得時 13 ms。遅延 Montgomery 演算、複数系列の Pollard Rho、まとめた GCD を使う。法を $$2^{62}$$ 未満に制限するため、64 bit 全域の API をそのまま置換しない。

LCS・編集距離は対応する AOJ の公式ケースでも確認した。

## 採用した変更

`sequence_compare.hpp` の `prefix`・`suffix` は整数列の 64 bit 比較と、CPU 判定付き AVX2 を持つ。一般の要素は `==` で比較する。読み込みは指定範囲に収め、パディングを要求しない。非 x86_64 CPU と `<false>` 指定では AVX2 を使わない。

LCS と編集距離はこの部品で共通の接頭辞・接尾辞を除去する。LCS は端の 1 文字が違う場合に早期終了する。LCS の小さい呼び出しで余分な関数呼び出しが残らないよう、`trim` のインライン展開を指定した。

Run 列挙の `Matcher` 全体を共通関数への委譲に変える案も測定したが、GCC のインライン展開と生成コードが変わり、長く測定したケースで約 3～8% 遅くなった。最終版では既存の型付き比較ループを残し、AVX2 カーネルの共有に絞った。GCC のインライン展開ログも調べたが、時間差の全てを特定の命令に帰属させてはいない。

公開 API・Run 列挙のアルゴリズム・編集距離の wavefront 探索と打ち切り条件は維持した。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0。AtCoder 実機と macOS 実機では未測定。
- generic: `-std=c++23 -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- CPU 2 に固定。コンパイル・テスト・他のベンチマークと同時に実行しない。
- seed は `20261019`。入力生成と正解確認の 1 回を計測外とし、同じ関数・入力での反復を計測する。各組の実行順を固定 seed で入れ替え、9 回の中央値を採る。
- 各入力種類では生成した同じ組を反復する。小さいランダム文字列の結果は、多数の異なる文字列を連続して処理した結果ではない。
- 全反復の checksum が変更前後で一致することを確認。復元には結果文字列全体の checksum 計算、Run 列挙には出力区間の checksum 計算も含む。入出力は計測に含まない。
- コマンド・CPU 情報・ソース SHA-256・全測定値・要約は各 JSON と同名の CSV に保存。

`identical` は同一文字列、`near` は中央の 1 文字を置換、`prefix` は末尾 128 文字、`suffix` は先頭 128 文字を乱数で置き換える。後者 2 つにも先頭・末尾の偶然の一致を許す。長い文字列は 262,144 文字、ランダム入力は 26 文字種。

以下は反復全体の時間。単位は ms。最終版は `adopted_generic`・`adopted_native`。

| 操作・入力 | 反復数 | generic 前 → 後 | native 前 → 後 |
| --- | ---: | ---: | ---: |
| LCS 長さ・同一 | 2048 | 106.112 → 10.924 | 106.420 → 10.668 |
| LCS 長さ・中央 1 文字置換 | 2048 | 106.847 → 11.029 | 106.472 → 11.010 |
| LCS 長さ・長い共通接頭辞 | 2048 | 108.008 → 12.255 | 107.379 → 12.020 |
| LCS 長さ・長い共通接尾辞 | 2048 | 108.080 → 11.950 | 107.963 → 11.935 |
| LCS 復元・同一 | 256 | 54.910 → 42.739 | 54.670 → 42.599 |
| 編集距離・同一 | 2048 | 18.106 → 11.007 | 17.961 → 11.011 |
| 編集距離・中央 1 文字置換 | 2048 | 16.321 → 10.882 | 16.575 → 10.825 |
| 編集距離・長い共通接頭辞 | 2048 | 19.135 → 12.635 | 18.458 → 12.392 |
| 編集距離・長い共通接尾辞 | 2048 | 19.709 → 12.329 | 19.551 → 12.169 |
| LCS 長さ・ランダム 4096 文字 | 128 | 10.482 → 10.429 | 10.229 → 10.061 |
| 編集距離・ランダム 4096 文字 | 128 | 31.634 → 31.754 | 32.420 → 32.404 |
| LCS 長さ・ランダム 16 文字 | 400000 | 78.497 → 78.915 | 73.512 → 74.034 |
| 編集距離・ランダム 16 文字 | 400000 | 26.655 → 26.849 | 20.091 → 20.058 |
| Run 列挙・ランダム 100000 文字 | 48 | 89.785 → 89.320 | 88.487 → 90.731 |
| Run 列挙・周期列 100000 文字 | 48 | 69.866 → 71.000 | 70.138 → 71.846 |

長い共通端がある場合は LCS の長さ計算が約 9～10 倍、編集距離が約 1.5～1.6 倍。ランダム文字列の中心処理そのものの高速化ではない。対照の Run 列挙では最大約 2.5% 遅い測定も残っており、全入力で同等以上とは主張しない。

## 不採用の記録

最初の候補と幅別 Montgomery 演算は `candidate_generic`、型付き比較へ変更した途中版は `generic`、インライン指定を試した途中版は `final_generic` に記録した。いずれも最終版の測定ではない。

Montgomery 候補は 32 bit ランダム判定が 0.915 → 0.912 ms、32 bit 素数のみが 8.721 → 8.643 ms、32 bit 分解が 4.060 → 3.930 ms。64 bit ランダム判定は 1.756 → 1.788 ms、64 bit 素数のみは 37.144 → 37.467 ms となった。過去の `primality_factorize.md` にも同様の候補の悪化記録があり、今回の小さい差だけで安定した改善と判断せず撤回した。試作差分は `montgomery32_candidate.patch` に保存し、ライブラリの変更には含めない。

## 検証

- GCC 15.2 / Clang 21.1.8 のヘッダー単体、単体テスト、全ヘッダーとの同時 include。
- 共通比較の全不一致位置・非整列アドレス・8 / 32 / 64 byte 境界・NUL・128 bit 整数・浮動小数点の正負のゼロ・独自の等価比較。読み込み禁止ページで囲った領域でも検証。
- LCS・編集距離・Run 列挙の既存の全探索と乱数比較、Clang ASan / UBSan。
- 両コンパイラで Run Enumerate 公式 24 ケース、LCS の AOJ システム 5 ケースとサンプル 1 ケースを長さ計算・復元の双方で確認、編集距離の AOJ システム 50 ケース。
- 影響を受ける 5 ヘッダーの Bundle と、対応するドキュメントの最初の使用例を両コンパイラで確認。

`validation.json` と各ログに検証結果を保存した。macOS 実機は未検証で、macOS CI の対象。`<false>` の通常経路はローカルでも検証した。

## 再現

```sh
python3 test/benchmark/shared_optimization_prepare.py --revision 56a7b9b
flock /tmp/cp-wm-opt.lock python3 test/benchmark/shared_optimization.py --baseline .verify-helper/cache/shared_optimization/baseline --strings-only --long --output test/benchmark/results/shared_optimization/reproduced_generic.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/shared_optimization.py --baseline .verify-helper/cache/shared_optimization/baseline --strings-only --long --native --output test/benchmark/results/shared_optimization/reproduced_native.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/shared_optimization_check.py
```

公式入力は `oj download --system` で取得した URL の MD5 を `.verify-helper/cache/<MD5>/test` として保存する。LCS はサンプルも含めた。新しい一覧とソースを取り直す場合は prepare に `--references` を追加する。取得日時が変わるため、保存済みの参照一覧を当時の記録として扱う。
