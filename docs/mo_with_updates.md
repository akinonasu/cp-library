---
title: "更新付き Mo 法"
documentation_of: //data_structure/range_query/mo_with_updates.hpp
---

配列の一点更新と区間の問い合わせを入力順に登録し、区間と時刻を動かして一括処理します。値の保持、更新前後の値、答えの計算は呼び出し側で用意します。

```cpp
#include "lib/data_structure/range_query/mo_with_updates.hpp"
#include <vector>

int main() {
    std::vector<int> a{1, 2, 1, 3, 2}, frequency(4), answers(2);
    MoWithUpdates mo(a.size());
    mo.add_query(0, 3);
    mo.add_update(1); // a[1]: 2 -> 1
    mo.add_query(0, 3);
    int distinct = 0;
    auto add = [&](int p) { if (frequency[a[p]]++ == 0) ++distinct; };
    auto erase = [&](int p) { if (--frequency[a[p]] == 0) --distinct; };
    auto change = [&](int id, bool inside, int value) {
        int p = mo.update_position(id);
        if (inside) erase(p);
        a[p] = value;
        if (inside) add(p);
    };
    mo.run(add, erase,
        [&](int id, bool inside) { change(id, inside, 1); },
        [&](int id, bool inside) { change(id, inside, 2); },
        [&](int id) { answers[id] = distinct; });
    // answers = {2, 1}
}
```

この例の更新は 1 件です。複数の更新では、更新 ID ごとに位置・更新直前の値・更新後の値を記録し、適用時に更新後の値、巻き戻し時に更新直前の値を使います。同じ位置への連続した更新も登録できます。

| 操作 | 内容・返り値 |
| --- | --- |
| `MoWithUpdates(n=0)` | 長さ `n` の配列に対する処理を作成 |
| `size()` / `query_count()` / `update_count()` | `int` で配列長・問い合わせ数・更新数を取得 |
| `add_update(p)` | 位置 `p` の更新を登録し、0 始まりの更新 ID を `int` で返す |
| `add_query(l,r)` | 半開区間 `[l,r)` の問い合わせを登録し、0 始まりの問い合わせ ID を `int` で返す |
| `update_position(id)` | 更新 ID の位置を `int` で返す |
| `query(id)` | `Query{l,r,time}` を値で返す。`time` はその問い合わせより前に登録した更新数 |
| `order(block_size=0)` | 処理順の問い合わせ ID を `std::vector<int>` で返す |
| `run(add,erase,apply,rollback,answer,block_size=0)` | コールバックを呼んで処理。返り値は `void` |
| `run(add_left,add_right,erase_left,erase_right,apply,rollback,answer,block_size=0)` | 左端と右端の操作を分けて処理。返り値は `void` |

添字は 0 始まりです。`0 <= n <= INT_MAX`、`0 <= p < n`、`0 <= l <= r <= n` とします。更新数と問い合わせ数はそれぞれ `INT_MAX` 以下です。空区間、問い合わせの重複、問い合わせだけの入力、更新だけの入力に対応します。空の配列には空区間の問い合わせを登録できます。

`block_size` は非負です。0 では配列長・問い合わせ数・更新数から幅を選び、更新がない場合は [通常の Mo 法](mo_order.md) の順序を使います。正数では指定した幅を使います。

`run` の呼び出し前に、配列を更新前の初期状態、集約を空区間の状態にしてください。

- `add(p)` / `erase(p)`：現在の配列の位置 `p` を集約に追加・削除します。
- `apply(id,inside)`：更新 `id` を適用します。
- `rollback(id,inside)`：更新 `id` を取り消します。
- `inside` は、更新位置がその時点の区間に含まれるかを表します。含まれる場合の集約の変更も呼び出し側で行います。
- `answer(id)`：問い合わせに対応する時刻・区間になったときに呼ばれます。呼び出し順は登録順とは限りません。

左右を分ける場合、`add_left`・`add_right` は追加する位置、`erase_left`・`erase_right` は削除する位置を受け取ります。時刻の変更が先、区間の移動が後です。コールバックから登録内容を変更しないでください。

登録内容を保ったまま再実行できます。再実行前に呼び出し側の配列と集約を初期化してください。終了時の配列・集約の状態は初期状態へ戻しません。

配列長を $$N$$、問い合わせ数を $$Q$$、更新数を $$U$$、指定する幅を $$B$$ とします。登録は償却 $$O(1)$$、順序生成は $$O(Q\log Q)$$、保持・作業空間は $$O(Q+U)$$ です。更新がある場合、幅が $$1\le B\le N$$ でコールバックが定数時間なら、処理は $$O(Q\log Q+QB+N^2/B+UN^2/B^2)$$ 時間です。$$N,Q,U$$ が同程度なら、既定の幅はおおむね $$N^{2/3}$$、処理は $$O(N^{5/3})$$ になります。問い合わせがない場合、コールバックは呼びません。
