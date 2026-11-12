---
title: コンパイラとアーキテクチャの共通条件
documentation_of: //components/compiler_features.hpp
---

ライブラリ内部のコンパイル対象の判定を共有します。各ライブラリから自動で読み込まれ、実行 CPU の命令対応は別に判定します。

次のマクロは、条件を満たす場合に `1`、それ以外では `0` です。利用者側で定義・変更しないでください。

| マクロ | 条件 |
| --- | --- |
| `CP_X86` | x86 または x86-64 |
| `CP_GCC_X86` | x86 系の GCC。Clang は除く |
| `CP_GNU_X86` | x86 系で GNU 互換の機能を使える GCC または Clang |
| `CP_GCC_LINUX64` | Linux x86-64 の GCC。Clang は除く |
