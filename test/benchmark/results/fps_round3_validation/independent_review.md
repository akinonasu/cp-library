# 最終差分の読み取りレビュー

2026-10-19 UTC。担当: `evaluation_interpolation`。比較元は `bd951dce6fdcb3ea9db4fcef342147012a8ab0c0`。最終5変更と対応する単体・資料を読み、必須修正となる安全性・公開 API の問題は見つからなかった。

参照した固定原票は [fps_round3_freeze.json](../fps_round3_freeze.json)、SHA-256 は `7f14d6c7e1c2fab6e9405ece92fa0946479e7df7a241826a3d9c3929ec6c576f`。レビュー時の次の10ファイルは、原票の SHA-256 とすべて一致した。

| ファイル | SHA-256 |
| --- | --- |
| `components/fps_bostan.hpp` | `bda7e2f7b2189023cc5da6b695eddc5bcde4ff81eb8407d933c0cf18fb13e258` |
| `components/fps_convolution.hpp` | `c756be0b18cd5435641e568c977e2e71aced49d3a7393bd27fb749372ed8067e` |
| `components/fps_exp_blocked.hpp` | `5c1aaaa34f92f859f55ba245475b4d8ac98832f149c8fe6b1b5323d097dcb4d2` |
| `components/fps_io.hpp` | `0c74a2a33c4073471813c15cdaced24b0613f4f9b05d41e7e789b1dfe190ac6a` |
| `fps/bostan_mori.hpp` | `4177aa77be9db4af684eba23d2ba07cd092e8628e308079c29f81858e18128d6` |
| `math/mint.hpp` | `733ff4a2ef2ac34b0174980ec373570c45429731301dcf55f72e768011d748fc` |
| `test/unit/fps_bostan.cpp` | `5c9c55678434b7e77c77d162596470769ece4b9d715ce051f93938091170e8f1` |
| `test/unit/fps_convolution_borrow.cpp` | `4c801ea17915acea3d7883abe04447229b3de9e80de28209f2bd8bd4b7b5f3ed` |
| `test/unit/fps_exp_tail.cpp` | `e7c49c6e021219bcc576d221a469d52775587bf493599ab9b9c942da0b414d35` |
| `test/unit/fps_io.cpp` | `81a57dc2b803141a2baaf8663c862ac0e814935ad55651e03d7a7c01b6ed6fd8` |

## 確認範囲

- I/O: 4ストリームの検証が完了してから順に callback を呼び、失敗時は入力位置を確定しない。callback の参照受け渡し、64要素の残部、既存出力への追加、作業領域の所有権、従来の fallback を確認した。
- 畳み込み: 借用領域の全 `Mint` の生存、32 byte alignment、変換長と16係数の末尾、B・根表の確保範囲、GCC alias 対応 SIMD アクセス、入力保持、同一配列、重複 `memmove`、破棄時の所有権を確認した。この担当は借用経路の実装も担当しており、今回の確認は固定後の再読である。
- 指数: 完全な8項の入力・逆順出力 load、短入力と端数、64 bit lane あたり最大8積と Montgomery 補正の上限、横和、通常入力と微分済み入力の倍率を確認した。native の非重複条件と公開 API が別の出力を作ること、raw のインプレース経路が不変であることも確認した。
- Bostan–Mori: 8要素・radix-2・radix-4 の境界、配列内アクセス、元の変換済み入力の保存、twist 後の最高次項補正、runtime AVX2 判定と汎用経路を確認した。
- Mint: 初回宣言の `= default`、default member initializer、non-user-provided 性、暗黙の constexpr / noexcept、レイアウトが維持されることを確認した。

## 保存物の監査

round3 の資料と IFMA 資料のローカルリンクはすべて存在した。保存 patch に絶対パスや親ディレクトリへの脱出はなかった。38個の候補 patch を固定比較元の文脈へメモリ上で適用し、対応する convolution・evaluation・exp の候補ヘッダーの記録 hash も照合した。別の原本を対象とする `profile_reference.patch` は未編集の409534へ `patch --dry-run` し、適用可能だった。

convolution・evaluation・exp の `metadata.files`、division の manifest 33点、Mint の manifest 26点は記録 hash と一致した。Bostan の pragma 表記正規化前後は固定原票と総合資料で区別されていた。資料はローカル測定と公式 judge、不採用候補、AVX512 の利用条件、小さい時間差を区別しており、未確認の fastest 達成を主張していなかった。総合測定と最終検証の完了値は、それぞれの実行担当の確定記録を使う。

このレビューで production・既存原票の編集、コンパイル、テスト実行、性能測定は行っていない。静的な確認と保存物の照合であり、別担当の最終検証を置き換えるものではない。
