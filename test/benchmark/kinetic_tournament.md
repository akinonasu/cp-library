# Kinetic Tournament

2026-10-20。整数時刻の単調な進行、直線の一点変更・削除、区間の最小値・最大値と添字を扱うライブラリを追加した。

## 参考実装

[Eric K. Zhang の実装](https://github.com/ekzhang/library/blob/f553dd59b9f5635351478d6f45a9283fe603c840/kinetic_tournament.cpp)を確認した。各内部頂点に子の勝者と、部分木内の証明が最初に失効する時刻を保持する。時刻を進める際に、失効した部分だけをまとめて再計算する。浮動小数点やイベントの優先度付きキューは不要。この方式を採用した。

直接対応する Library Checker 問題はないため、関連する Line Add Get Min の AC を公式 API の時間順で取得した。

| 提出 | 取得時の時間 | 確認した手法 |
| --- | ---: | --- |
| [362191 / IceKylin](https://judge.yosupo.jp/submission/362191) | 57 ms | 座標を先読みした Li Chao Tree、radix sort による座標圧縮、連続配列 |
| [357336 / IceKylin](https://judge.yosupo.jp/submission/357336) | 58 ms | 同系統の Li Chao Tree、座標配列と葉の値の分離 |
| [357321 / IceKylin](https://judge.yosupo.jp/submission/357321) | 58 ms | 同系統の Li Chao Tree、反復による追加・問い合わせ |

最速提出は Kinetic Tournament ではなく、削除や添字区間の問い合わせも扱わない。時刻を戻せる点では Li Chao Tree が適している。今回は指定された時刻単調の API と点変更・削除・区間問い合わせを優先し、先読みをライブラリの条件にしない。これらの提出との時間比較やジャッジ上の最速性は主張しない。

取得日時・一覧・ソースの SHA-256 は `results/kinetic_tournament/reference.json`、取得スクリプトは `kinetic_tournament_reference.py` に保存した。

## 実装

- 直線は葉の添字ごとに 1 個だけ保持し、内部頂点は勝者の添字と失効時刻を連続配列に持つ。
- 区間問い合わせは通常の反復セグメント木と同じ分解を行い、候補の直線を現在時刻で評価する。
- 非活性の葉を `-1` とし、値の番兵を直線として扱わない。係数・時刻は符号付き型の全域を使える。
- 同値なら小さい添字を選ぶ。交差点が整数かどうかと添字の優先順から、次に勝者が変わる最初の整数時刻を求める。
- 評価値の差は取らない。係数の差を符号と符号なしの絶対値で保持し、64 bit の商・余りを使って交差時刻を求める。係数の差が符号付き 64 bit を超える場合にも対応する。
- 失効時刻は符号付き時刻を順序を保って符号なし 64 bit へ写したもの。0 はイベントなしに使う。構築・更新時の時刻より後にしかイベントはないため、最小時刻に対応する 0 は将来のイベントにならない。最大時刻のイベントは正しく表現する。
- `advance` はイベントを一つずつ処理せず、現在時刻で必要な部分を再計算する。大量の同時交差も 1 回の再帰でまとめて処理する。

## 検証

- GCC 15.2 / Clang 21.1.8 のヘッダー単独コンパイル、既存ライブラリとの同時 include。
- 係数 -2～2 の 2 直線の全 625 組を、時刻 -5～5 で直接比較。次のイベントも整数時刻の全探索と比較。
- `INT64_MIN`・`INT64_MAX` とその近傍など 9 値からなる 2 直線の全 6,561 組を、最小・最大の両方で時刻の端から端まで照合。
- 評価値自体は 128 bit に収まるが差は収まらない入力、最小時刻の直後の交差、最大時刻ちょうどの交差、32 bit 時刻の最大値。
- 中間の積が返り値の型を超えても加算後は収まる入力と、係数型より狭い返り値の型。中間値は係数型に合わせた拡張型で計算する。
- 5 種類の型・最小／最大設定、11 種類の長さで計 110,000 回の操作。点設定・削除・空き位置への追加・同時刻・イベント時刻への進行・区間問い合わせ・全体問い合わせ・空区間・全削除を直接走査と比較。
- コピー・ムーブ・再構築・リセット。ムーブ元の空の状態も検査。
- 20 万直線をまとめて構築して大きく時刻を進めるケース。
- Clang ASan / UBSan。
- Line Add Get Min の公式 15 ケースを GCC・Clang で照合。
- ヘッダーと提出コードの Bundle、ドキュメントの使用例を両コンパイラでコンパイル・実行。

Line Add Get Min の検証コードは、全直線を追加順の添字に置き、各問い合わせに「その時点までに追加された直線数」を記録する。問い合わせを x の昇順に並べ、時刻を進めてその長さの prefix の最小値を求める。元の順序で出力する。この先読みは任意順の x を持つ問題を検証するための変換で、ライブラリの API 自体には不要。点変更・削除は独立した単体テストで検証する。

結果は `check.log`・`sanitize.log`・`validation.log`・`validation.json` に保存した。オンライン提出は行っていない。Mac 実機はローカルでは未検証で、macOS CI の対象。

## 測定条件

- WSL2 / Linux x86_64、Intel Core i7-13700F、GCC 15.2.0、CPU 2 に固定。
- generic: `-std=c++2b -O2 -DNDEBUG -Wall -Wextra -Werror`。
- native: 上記に `-march=native -ftrivial-auto-var-init=zero` を追加。
- 検証・コンパイル・他の測定とは同時に実行しない。
- 長さ 100,000、seed `20261020`。全位置の初期設定・入力生成・破棄・入出力を除き、操作と checksum を測る。初期状態は全方式で同じ。
- `reference` は commit ID で固定した Eric K. Zhang の実装をそのまま使用。`library` は既定の 128 bit 評価、`library64` は 64 bit 評価を指定する。後者と参考実装で全ての積・和・差が収まる入力を使う。
- 参考実装の同値時の優先順は傾き、本実装は添字。比較する返り値は最小値のみで、全試行の checksum が一致することを確認した。
- 順序を seed で入れ替え、各 7 回実行した中央値。AtCoder・Library Checker 実機、Mac 実機での速度は未測定。

| 入力 | 操作 |
| --- | --- |
| random | 非負のランダムな傾き・切片。100,000 操作を点変更・時刻の進行・区間問い合わせで交互に実行 |
| envelope | 傾き `n-1-i`、切片 `i*(i+1)/2`。100,000 回、時刻を 1 ずつ進めて全体の最小値を求める |
| same_slope | 傾き 7、ランダムな切片。100,000 回、時刻を 1 ずつ進めてランダムな区間の最小値を求める |
| jump | envelope と同じ直線で、一度だけ時刻 n へ進めて全体の最小値を求める |
| concurrent | 傾き i、切片 `(n-1-i)*n`。全てが時刻 n で交差し、一度だけその時刻へ進めて全体の最小値を求める |

## 結果

単位は ms。各行は上記の操作全体の合計時間。

| 入力 | generic reference / library / library64 | native reference / library / library64 |
| --- | ---: | ---: |
| random | 13.143 / 11.108 / 8.393 | 13.636 / 11.266 / 8.381 |
| envelope | 7.606 / 11.089 / 9.410 | 7.516 / 11.091 / 9.536 |
| same_slope | 13.991 / 14.380 / 8.001 | 14.111 / 14.075 / 7.995 |
| jump | 0.385 / 0.407 / 0.309 | 0.378 / 0.420 / 0.308 |
| concurrent | 0.393 / 0.400 / 0.329 | 0.387 / 0.424 / 0.331 |

乱数の混合操作では既定版も短い時間になった。一方、envelope は既定版で約 46～48%、64 bit 指定でも約 24～27% 遅い。全ケースでの速度優位性は主張しない。既定版は評価幅が異なり、64 bit 全域・添字の優先順・空の結果・削除も扱うため、参考実装と条件の差がある。

条件とソースの SHA-256 は各 `environment.json`、全試行は `samples.csv`、中央値は `summary.json` に保存した。

## 再現

```sh
python3 test/benchmark/kinetic_tournament_reference.py --cache .verify-helper/cache/kinetic_tournament_references --output test/benchmark/results/kinetic_tournament/reference.json
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament_check.py
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament.py --output test/benchmark/results/kinetic_tournament/reproduced_generic
flock /tmp/cp-wm-opt.lock python3 test/benchmark/kinetic_tournament.py --native --output test/benchmark/results/kinetic_tournament/reproduced_native
```

測定だけを再現する場合は `reference.json` を更新せず、固定したソースを使う。
