from pathlib import Path
p=Path('/tmp/fps-division-round3/candidate/test/unit/fps_division_blocked.cpp');s=p.read_text();pos=s.index('static void check_alias()')
s=s[:pos]+'''static void check_divisor_input() {
	using fps_detail::blocked_poly::native_divide;
	const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
	for (int m : {1, 2, 7, 8, 9, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257, 513, 1025, 4097, 65537}) {
		const std::size_t bytes = std::size_t(m) * sizeof(fps_mint), readable = (bytes + page - 1) / page * page;
		for (bool end : {false, true}) {
			void* mapping = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			assert(mapping != MAP_FAILED);
			auto* begin = static_cast<char*>(mapping) + page;
			auto* guard = begin + readable;
			assert(mprotect(mapping, page, PROT_NONE) == 0 && mprotect(guard, page, PROT_NONE) == 0);
			auto* values = ::new (end ? guard - bytes : begin) fps_mint[m];
			for (int i = 0; i < m; ++i) values[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 11);
			const fps b(values, values + m);
			assert(mprotect(begin, readable, PROT_READ) == 0);
			for (int n : {m, m + 1, 2 * m - 1, 2 * m, 2 * m + 1, 4 * m + 17}) {
				fps a(n);
				for (int i = 0; i < n; ++i) a[i] = fps_mint::word(2 * fps_mint::mod() - 1 - i % 7);
				fps q, r;
				native_divide(a.data(), a.size(), values, m, q, r);
				auto restored = q * b + r;
				fps_detail::Operations<fps_mint>::trim(restored);
				assert(restored == a);
				for (int i = 0; i < m; ++i) assert(values[i].v == b[i].v);
			}
			assert(munmap(mapping, readable + 2 * page) == 0);
		}
	}
}

'''+s[pos:];s=s.replace('check_output(), check_input(), check_alias()','check_output(), check_input(), check_divisor_input(), check_alias()');p.write_text(s)
