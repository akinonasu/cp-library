#include "../fixtures/beats.hpp"
#include "../../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n), in.read(q);
	std::vector<division_beats::S> a(n);
	for (auto &node : a) {
		long long value = 0;
		in.read(value);
		node = division_beats::leaf(value);
	}
	division_beats::Tree seg(a);
	while (q--) {
		int t = 0, l = 0, r = 0;
		in.read(t), in.read(l), in.read(r);
		--l;
		if (t == 3) out.println(seg.prod(l, r).sum);
		else {
			long long x = 0;
			in.read(x);
			if (t == 1) seg.apply(l, r, {-1, static_cast<unsigned long long>(x)});
			else seg.apply(l, r, {x, 1});
		}
	}
}
