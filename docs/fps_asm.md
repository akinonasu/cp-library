---
title: FPS の命令文字列
documentation_of: //components/fps_asm.hpp
---

FPS 内部のインラインアセンブリを短く記述するためのプリプロセッサ部品です。公開の FPS ヘッダーから自動的に読み込まれるため、利用者が直接呼び出す必要はありません。

`CP_FPS_ASM_` で始まる内部マクロは、命令名と文字列化したオペランドから従来と同じ命令文字列を作ります。各カーネル内の `CP_FPS_ASM_BLOCK` には短いオペランド名と命令の組み立て方を渡し、使用後に `undef` します。命令の順序、`volatile`、入出力制約、clobber、ラベル、alignment は変えません。

`oj-bundle` が生成するソースを短くするための処理で、実行時の関数・分岐・作業領域は追加しません。API と FPS の計算量も変わりません。

サイズと生成コードの同一性は [測定資料](https://github.com/qandori/cp-library/blob/main/test/benchmark/fps_source_size.md) にあります。実行時の計算量・追加空間は増えません。
