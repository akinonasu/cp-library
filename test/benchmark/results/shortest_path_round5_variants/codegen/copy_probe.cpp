#include <cstdint>
#include <utility>
#include <algorithm>
using Arc = std::pair<int, std::uint32_t>;
struct Edge { int from, to; std::uint32_t weight; };
void direct(Arc *d, const Edge *e) { *d = {e->to, e->weight}; }
void local(Arc *d, const Edge *e) { Arc x{e->to, e->weight}; *d = x; }
void named(Arc *d, const Edge *e) { d->first = e->to; d->second = e->weight; }
void copy(Arc *d, const Arc *e) { *d = *e; }
void move(Arc *d, Arc *e) { *d = std::move(*e); }
void copyalgo(Arc *d, const Arc *e) { std::copy_n(e, 1, d); }
