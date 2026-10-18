import copy
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import affected_test
from affected import SourceTree
import intrinsic_gate as gate
import no_cache_route as route
import no_cache_route_test


class IntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.write("test/unit/dynamic_wavelet_core.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n')
        self.head = self.commit("intrinsic fixture\n\n" + route.TRAILER)

    def test_exact_push_pr_and_main_select_all_five_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(set(plan["unit"]), gate.REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(set(plan["unit"]), gate.REQUIRED_UNITS)

    def test_normal_event_does_not_plan_or_execute_cpp(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        head = self.commit("ordinary event", bind=False)
        with patch.object(gate, "build_affected_plan", side_effect=AssertionError("unexpected planner")):
            self.assertEqual(gate.selected(self.root, *self.push(head, route.MAIN_REF)),
                             ({"mode": "normal"}, None))

    def test_missing_required_unit_and_changed_source_fail_closed(self):
        (self.root / "test/unit/dynamic_wavelet_core.cpp").unlink()
        self.head = self.commit("remove required unit\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "Required DWM units"):
            gate.selected(self.root, *self.push())
        self.write("components/dynamic_bit_vector.hpp", "changed but not committed\n")
        with self.assertRaisesRegex(ValueError, "checkout differs"):
            gate.selected(self.root, *self.push())

    def test_dependency_manifest_uses_exact_checkout_bytes(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/dynamic_wavelet_core.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="intrinsic-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            names = sorted({target} | gate.PRODUCTS)
            continuation = " " + chr(92) + "\n "
            depfile.write_text("gate-output: " + continuation.join(str(self.root / name) for name in names) + "\n")
            proof = gate.dependency_proof(self.root, tree, depfile, source, target)
            self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, set(names))
            self.write("components/dynamic_bit_vector.hpp", "mismatched baseline bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target)

    def test_external_baseline_and_missing_production_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/dynamic_wavelet_core.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="intrinsic-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "dynamic_bit_vector.hpp"
            old.write_bytes(tree.content("components/dynamic_bit_vector.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target)
            depfile.write_text(f"gate-output: {source}\n")
            with self.assertRaisesRegex(ValueError, "current production DWM headers"):
                gate.dependency_proof(self.root, tree, depfile, source, target)

    def test_macro_requirements_and_explicit_scalar_flags(self):
        base = (b"#define assert(expr) check(expr)\n#define ONLINE_JUDGE 1\n#define ATCODER 1\n"
                b"#define __GNUC__ 15\n#define __GNUC_MINOR__ 2\n#define __GNUC_PATCHLEVEL__ 0\n"
                b"#define __x86_64__ 1\n")
        native = base + b"#define __AVX2__ 1\n#define __BMI2__ 1\n"
        self.assertTrue(gate.macro_proof(native, "native")["assertions"])
        self.assertEqual(gate.macro_proof(base, "scalar")["isa"], {})
        for raw, mode in ((base, "native"), (native, "scalar"),
                          (native + b"#define NDEBUG 1\n", "native"),
                          (native.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "native")):
            with self.assertRaises(ValueError):
                gate.macro_proof(raw, mode)
        self.assertEqual(gate.MODES["scalar"], ["-march=x86-64", "-mtune=generic", "-mno-avx",
            "-mno-avx2", "-mno-avx512f", "-mno-bmi", "-mno-bmi2",
            "-fno-tree-loop-vectorize", "-fno-tree-slp-vectorize"])
        self.assertIn("-fno-lto", gate.COMMON)
        self.assertNotIn("-DNDEBUG", gate.COMMON)

    def test_reduced_or_oversized_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        for group, change in (("unit", []), ("headers", []),
                              ("unit", plan["unit"] + ["test/unit/extra.cpp"] * 4)):
            altered = copy.deepcopy(plan)
            altered[group] = change
            with patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())


if __name__ == "__main__":
    unittest.main()
