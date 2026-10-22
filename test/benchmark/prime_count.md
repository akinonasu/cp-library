# 素数計数

2026-10-22。`prime_count(n)` は `n` 以下の素数の個数を返します。

## 参考実装

[Library Checker Counting Primes](https://judge.yosupo.jp/problem/counting_primes) は `N <= 10^11` です。[AC 時間順一覧](https://v3.api.judge.yosupo.jp/submissions?problem=counting_primes&status=AC&order=%2Btime&limit=5)と先頭 5 件のソースを確認しました。取得時の順位・言語・時間・ソースの SHA256 は `results/prime_count/references.json` に保存しています。

- [377988 / wery0](https://judge.yosupo.jp/submission/377988)：6 ms、C++。約 1 MB の文字列に区間の素数個数を埋め込み、復元した累積値と端の区間篩を組み合わせます。任意の入力に対する計算量・ソース長を優先し、この大きな事前計算表は採用していません。
- [222872 / sortA0329](https://judge.yosupo.jp/submission/222872)：12 ms、C++。奇数だけの部分篩、小さい引数・大きい引数の個数表、篩われていない候補、保存した `N / candidate` を組み合わせます。繰り返す除算の逆数乗算への置換、範囲をまとめた小さい個数表の更新、最後の素数の積の補正を参考にしました。
- [411722 / GordonBGood](https://judge.yosupo.jp/submission/411722)：10 ms、Haskell。[344887](https://judge.yosupo.jp/submission/344887)・[356164](https://judge.yosupo.jp/submission/356164) は 11〜12 ms、Nim。Meissel 法・wheel 篩・popcount による実装です。参照した範囲ではおよそ `N^(2/3)` の篩領域を使い、今回採用した平方根サイズの商テーブル法とは構成が異なります。これらの言語版はローカルで実行していません。

今回の実装は 64 bit の個数表を使い、戻り値が 32 bit を超える入力にも対応します。参照 C++ の浮動小数点による商の初期化を整数除算にし、逆数乗算にも厳密な整数補正を入れました。平方根を 128 bit の積で補正し、最終補正の中間値にも 128 bit を使います。領域の重ね合わせやグローバルなコンパイラ設定は使っていません。

## 測定

Linux x86_64、AMD EPYC 9V74、GCC 14.2、`-std=c++23 -O2`。CPU 0 に固定し、他のコンパイル・検証と測定を重ねず、ウォームアップ 1 回後に順序を固定 seed で入れ替えて 7 回測定した中央値です。全試行の答えを既知値と照合しました。全試行・コンパイラ・CPU・フラグ・ソースのハッシュは `results/prime_count/measurements.json` に保存しています。

まず、呼び出し内の確保・初期化・計算・解放を含む時間です。入出力とプロセス起動は含みません。「標準商テーブル」は `test/benchmark/prime_count.cpp` の、整数商に対する通常の篩 DP です。奇数への圧縮、候補の削減、逆数乗算を使いません。

| N | 標準商テーブル | 今回 | 速度比 |
| ---: | ---: | ---: | ---: |
| 10^6 | 0.061 ms | 0.016 ms | 3.7 倍 |
| 10^8 | 1.305 ms | 0.158 ms | 8.2 倍 |
| 10^10 | 31.794 ms | 2.770 ms | 11.5 倍 |
| 10^11 | 161.707 ms | 12.080 ms | 13.4 倍 |
| 10^12 | 845.823 ms | 55.401 ms | 15.3 倍 |
| 10^13 | 4484.200 ms | 265.689 ms | 16.9 倍 |

次はプロセスの起動・入出力・終了も含む時間です。参考提出は取得したソースをそのままコンパイルしました。222872 のソース内の `Ofast`・ループ展開・AVX2 指定も維持しています。ライブラリ側は同じ `-O2` のドライバを使い、コンパイラ設定を上書きしていません。

| N | 今回 | 222872（計算法） | 377988（大きな表） |
| ---: | ---: | ---: | ---: |
| 10^6 | 1.277 ms | 1.231 ms | 3.594 ms |
| 10^10 | 3.931 ms | 3.577 ms | 3.373 ms |
| 10^11 | 13.511 ms | 12.048 ms | 3.535 ms |

参考計算法と近い時間で動作し、より大きい個数にも対応します。大きな表を使う最速提出より速いとは主張しません。Library Checker の表示時間とこの環境の時間は直接比較できません。

## 検証

- 独立した Eratosthenes の篩で 200 万以下の累積個数を作り、0〜30000 の全入力、ランダム 2000 件、平方数・4 乗数の直前・一致・直後を比較しました。
- `10^0`〜`10^13` の既知の個数を確認しました。`10^12` と `10^13` は 32 bit を超える戻り値です。
- 10^9〜10^12 付近の 4 区間で、連続 256 整数の決定的 Miller–Rabin 素数判定と、2 回の計数の差を比較しました。
- 浮動小数点の丸め方向 4 通り、GCC・Clang の単独・二重 include とユニットテスト、Clang ASan・UBSan、GCC の `NDEBUG` ドライバを確認しました。コンパイル時の constexpr 上限は通常 CI と同じです。
- Library Checker の公開 31 ケースを生成し、GCC・Clang の出力を公式の正解と比較しました。公開ジャッジへの提出は行っていません。AtCoder 用テストは追加していません。

```sh
python3 test/verify.py --unit-only --cxx g++ clang++ --jobs 2 --selection test/benchmark/prime_count_selection.json
python3 test/verify.py --unit-only --cxx clang++ --sanitize --selection test/benchmark/prime_count_selection.json
python3 test/benchmark/prime_count_reference.py --cache /tmp/prime-count-refs --output /tmp/prime-count-refs/references.json
python3 "$LC_ROOT/generate.py" "$LC_ROOT/number_theory/counting_primes/info.toml"
python3 test/benchmark/prime_count.py --references /tmp/prime-count-refs --problem "$LC_ROOT/number_theory/counting_primes" --output /tmp/prime-count-measurements.json
```

`LC_ROOT` は Library Checker の問題リポジトリのパスです。生成したケースは同リポジトリ側に保持します。
