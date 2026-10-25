import copy
import json
import os
from pathlib import Path
import signal
import subprocess
import tempfile
import unittest
from unittest.mock import Mock, patch

import affected_test
from affected import SourceTree, canonical
import intrinsic_gate as gate
import no_cache_route as route
import no_cache_route_test


MACROS = (b"#define assert(expr) check(expr)\n#define ONLINE_JUDGE 1\n#define ATCODER 1\n"
          b"#define __GNUC__ 15\n#define __GNUC_MINOR__ 2\n#define __GNUC_PATCHLEVEL__ 0\n"
          b"#define __x86_64__ 1\n")
NATIVE_MACROS = MACROS + b"#define __AVX2__ 1\n#define __BMI2__ 1\n"


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
        self.assertEqual(gate.scope_mode(plan), "dwm")
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
        base, native = MACROS, NATIVE_MACROS
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
        for mode in gate.MODES:
            self.assertEqual(gate.mode_flags(mode), gate.COMMON + gate.MODES[mode])
            self.assertNotIn("-DMATRIX_DISABLE_AVX2", gate.mode_flags(mode))

    def test_reduced_or_oversized_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        for group, change in (("unit", []), ("headers", []),
                              ("unit", plan["unit"] + ["test/unit/extra.cpp"] * 4)):
            altered = copy.deepcopy(plan)
            altered[group] = change
            with patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())


class MatrixIntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.git("checkout", "--quiet", self.base)
        self.write("components/matrix_arithmetic.hpp", "#pragma once\n")
        self.write("math/matrix.hpp", '#include "../components/matrix_arithmetic.hpp"\n')
        self.write("math/mint.hpp", "#pragma once\n")
        for name in ("matrix", "matrix_characteristic"):
            self.write("test/unit/" + name + ".cpp", '#include "math/matrix.hpp"\n#include "math/mint.hpp"\n')
        self.write("test/unit/standalone.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n'
                   '#include "math/matrix.hpp"\n')
        self.base = self.commit("matrix baseline", bind=False)
        tree = SourceTree(self.root, self.base)
        evidence = {
            "version": 1, "base": self.base,
            "baseline_source_sha256": route.source_hash(tree, route.BASELINE_INPUTS),
            "reviewed_control_sha256": route.source_hash(tree, route.CONTROL_PATHS),
            "policy": "exact-baseline-ci-and-reviewed-source-keys-v1",
            "runs": {name: {"id": number, "attempt": 1}
                     for number, name in enumerate(route.BASELINE_JOBS, 1)},
        }
        self.write(route.EVIDENCE_PATH, (canonical(evidence) + b"\n").decode())
        self.write("components/matrix_arithmetic.hpp", "#pragma once\nusing matrix_changed = int;\n")
        self.head = self.commit("matrix candidate\n\n" + route.TRAILER)

    def test_exact_matrix_push_pr_and_main_select_transitive_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.scope_mode(plan), "matrix")
        self.assertEqual(set(plan["unit"]), gate.MATRIX_REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.MATRIX_PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(gate.scope_mode(plan), "matrix")
            self.assertEqual(set(plan["unit"]), gate.MATRIX_REQUIRED_UNITS)
            self.assertEqual(set(plan["headers"]), gate.MATRIX_PRODUCTS)

    def test_missing_matrix_units_or_headers_fail_closed(self):
        head = self.head
        for name in sorted(gate.MATRIX_REQUIRED_UNITS | gate.MATRIX_PRODUCTS):
            with self.subTest(name=name):
                self.git("checkout", "--quiet", head)
                (self.root / name).unlink()
                self.head = self.commit("remove matrix input\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "(Required MATRIX units|Production MATRIX headers|Include escapes source tree)"):
                    gate.selected(self.root, *self.push())

    def test_reduced_extra_or_mixed_scope_cannot_silently_pass(self):
        _, plan = gate.selected(self.root, *self.push())
        variants = []
        for group in ("headers", "unit"):
            for target in plan[group]:
                altered = copy.deepcopy(plan)
                altered[group].remove(target)
                variants.append(altered)
            for extra in ([plan[group][0]], ["test/unit/extra.cpp" if group == "unit" else "math/extra.hpp"]):
                altered = copy.deepcopy(plan)
                altered[group].extend(extra)
                variants.append(altered)
        for field, values in (("changed_paths", []), ("changed_paths", ["math/unrelated.hpp"]),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.PRODUCTS))):
            altered = copy.deepcopy(plan)
            altered[field] = values
            variants.append(altered)
        for altered in variants:
            with self.subTest(plan=altered), patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())

    def test_actual_extra_dependent_is_rejected_not_removed(self):
        self.write("test/unit/matrix_extra.cpp", '#include "math/matrix.hpp"\n')
        self.head = self.commit("new dependent requires review\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exceeds reviewed gate budget"):
            gate.selected(self.root, *self.push())

    def test_actual_mixed_production_changes_fail_closed(self):
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing other_changed = int;\n")
        self.head = self.commit("mixed intrinsic changes\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exactly one reviewed intrinsic mode"):
            gate.selected(self.root, *self.push())

    def test_matrix_dependency_proofs_include_exact_production_bytes(self):
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="matrix-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            for target in sorted(gate.MATRIX_REQUIRED_UNITS | gate.MATRIX_PRODUCTS):
                generated = target in gate.MATRIX_PRODUCTS
                source = Path(directory) / "header.cpp" if generated else self.root / target
                if generated:
                    source.write_text(gate.header_source(target))
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source)} | {str(self.root / name) for name in names}
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                proof = gate.dependency_proof(self.root, tree, depfile, source, target, generated, "matrix")
                self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, names)
                self.assertEqual(set(proof["production_headers"]), gate.MATRIX_PRODUCTS & names)
                for product in gate.MATRIX_PRODUCTS & names:
                    with self.subTest(target=target, omitted=product):
                        depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(self.root / product)})) + "\n")
                        with self.assertRaisesRegex(ValueError, "current production MATRIX headers"):
                            gate.dependency_proof(self.root, tree, depfile, source, target, generated, "matrix")

    def test_matrix_baseline_foreign_unresolved_and_changed_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/matrix.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="matrix-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "matrix_arithmetic.hpp"
            old.write_bytes(tree.content("components/matrix_arithmetic.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            paths = {str(self.root / name) for name in {target} | gate.MATRIX_PRODUCTS}
            depfile.write_text("gate-output: " + " ".join(sorted(paths | {str(self.root / 'math/unrelated.hpp')})) + "\n")
            with self.assertRaisesRegex(ValueError, "not resolved by reviewed source selector"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(source)})) + "\n")
            with self.assertRaisesRegex(ValueError, "actual source"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")
            depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
            self.write("components/matrix_arithmetic.hpp", "wrong source bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="matrix")

    def test_matrix_scalar_flags_and_runtime_dispatch_macro_are_required(self):
        scalar = MACROS + b"#define MATRIX_DISABLE_AVX2 1\n"
        self.assertEqual(gate.macro_proof(scalar, "scalar", "matrix")["MATRIX_DISABLE_AVX2"], "1")
        self.assertIsNone(gate.macro_proof(NATIVE_MACROS, "native", "matrix")["MATRIX_DISABLE_AVX2"])
        for raw, mode in ((MACROS, "scalar"), (scalar.replace(b"AVX2 1", b"AVX2 0"), "scalar"),
                          (scalar + b"#define __AVX2__ 1\n", "scalar"),
                          (scalar + b"#define NDEBUG 1\n", "scalar"),
                          (NATIVE_MACROS + b"#define MATRIX_DISABLE_AVX2 1\n", "native"),
                          (NATIVE_MACROS + b"#define NDEBUG 1\n", "native"),
                          (scalar.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "scalar")):
            with self.subTest(raw=raw, mode=mode), self.assertRaises(ValueError):
                gate.macro_proof(raw, mode, "matrix")
        self.assertEqual(gate.mode_flags("native", "matrix"), gate.COMMON + gate.MODES["native"])
        self.assertEqual(gate.mode_flags("scalar", "matrix"),
                         gate.COMMON + gate.MODES["scalar"] + ["-DMATRIX_DISABLE_AVX2"])

    def test_matrix_driver_runs_every_target_with_checked_mode_flags(self):
        selected, plan = gate.selected(self.root, *self.push())
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="matrix-driver-") as directory:
            directory = Path(directory)
            compiler, cc1 = directory / "g++-15", directory / "cc1plus"
            compiler.write_bytes(b"compiler fixture")
            cc1.write_bytes(b"cc1 fixture")
            commands = gate.Commands(directory)
            compiled, executed, probes = [], [], []

            def run(command, *, source=b"", seconds=gate.COMMAND_SECONDS):
                command = [str(value) for value in command]
                commands.count += 1
                if command == [str(compiler), "-dumpfullversion"]:
                    return b"15.2.0\n"
                if command == [str(compiler), "-print-prog-name=cc1plus"]:
                    return str(cc1).encode() + b"\n"
                if command[0] != str(compiler):
                    self.assertEqual(len(command), 1)
                    executed.append(command[0])
                    return b""
                mode = "native" if "-march=native" in command else "scalar"
                flags = gate.mode_flags(mode, "matrix")
                self.assertEqual(command[1:1 + len(flags)], flags)
                if "-dM" in command:
                    self.assertEqual(source, b"#include <cassert>\n")
                    probes.append(mode)
                    return NATIVE_MACROS if mode == "native" else MACROS + b"#define MATRIX_DISABLE_AVX2 1\n"
                self.assertEqual(command[1 + len(flags):3 + len(flags)], ["-I", str(self.root)])
                source_path, binary = Path(command[-3]), Path(command[-1])
                if source_path.is_relative_to(self.root):
                    target = source_path.relative_to(self.root).as_posix()
                else:
                    target = source_path.read_text().split('"')[1]
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source_path)} | {str(self.root / name) for name in names}
                depfile = Path(command[command.index("-MF") + 1])
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                binary.write_bytes(b"test binary fixture")
                compiled.append((mode, target, str(binary)))
                return b""

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands), \
                 patch.object(commands, "run", side_effect=run), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "emit") as emit:
                gate.execute(self.root, selected, plan, directory)
            self.assertEqual(probes, ["native", "scalar"])
            self.assertEqual([(mode, target) for mode, target, _ in compiled],
                             [(mode, target) for mode in ("native", "scalar")
                              for group in ("headers", "unit") for target in plan[group]])
            self.assertEqual(executed, [binary for _, _, binary in compiled])
            self.assertEqual(commands.count, 24)
            self.assertEqual(emit.call_args.args, ("gate_completed",))

    def test_normal_matrix_event_does_not_plan_or_run_cpp(self):
        self.git("checkout", "--quiet", "-B", "main", self.base)
        self.write("components/matrix_arithmetic.hpp", "#pragma once\nusing normal = int;\n")
        head = self.commit("ordinary matrix event", bind=False)
        event, context = self.push(head, route.MAIN_REF)
        with tempfile.TemporaryDirectory(prefix="matrix-event-") as directory:
            event_path = Path(directory) / "event.json"
            event_path.write_text(json.dumps(event))
            with patch.dict(os.environ, {**context, "GITHUB_EVENT_PATH": str(event_path)}), \
                 patch.object(gate, "__file__", str(self.root / ".verify-helper/intrinsic_gate.py")), \
                 patch.object(gate, "build_affected_plan", side_effect=AssertionError("unexpected planner")), \
                 patch.object(gate, "execute", side_effect=AssertionError("unexpected C++")), \
                 patch.object(gate, "emit") as emit:
                gate.main()
            emit.assert_called_once_with("gate_skipped", reason="normal event; no extra C++ commands")


class EditDistanceIntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.git("checkout", "--quiet", self.base)
        self.write("string/edit_distance.hpp", "#pragma once\n")
        self.write("test/unit/edit_distance.cpp", '#include "string/edit_distance.hpp"\n')
        self.write("test/unit/standalone.cpp", '#include "data_structure/dynamic_wavelet_matrix.hpp"\n'
                   '#include "string/edit_distance.hpp"\n')
        self.base = self.commit("edit-distance baseline", bind=False)
        self.write("string/edit_distance.hpp", "#pragma once\nusing edit_distance_changed = int;\n")
        self.head = self.commit("edit-distance candidate\n\n" + route.TRAILER)

    def test_exact_push_pr_and_main_select_header_and_both_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.scope_mode(plan), "edit_distance")
        self.assertEqual(set(plan["unit"]), gate.EDIT_DISTANCE_REQUIRED_UNITS)
        self.assertEqual(set(plan["headers"]), gate.EDIT_DISTANCE_PRODUCTS)
        self.assertEqual(selected["source_head"], self.head)
        self.assertEqual(gate.SCOPES["edit_distance"][2:], (1, 2))
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(gate.scope_mode(plan), "edit_distance")
            self.assertEqual(set(plan["unit"]), gate.EDIT_DISTANCE_REQUIRED_UNITS)
            self.assertEqual(set(plan["headers"]), gate.EDIT_DISTANCE_PRODUCTS)

    def test_unknown_missing_extra_and_mixed_scope_fail_closed(self):
        _, plan = gate.selected(self.root, *self.push())
        variants = []
        for group in ("headers", "unit"):
            for target in plan[group]:
                altered = copy.deepcopy(plan)
                altered[group].remove(target)
                variants.append(altered)
            for extra in ([plan[group][0]], ["test/unit/extra.cpp" if group == "unit" else "string/extra.hpp"]):
                altered = copy.deepcopy(plan)
                altered[group].extend(extra)
                variants.append(altered)
        for field, values in (("changed_paths", []), ("changed_paths", ["string/unknown.hpp"]),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.PRODUCTS)),
                              ("changed_paths", plan["changed_paths"] + sorted(gate.MATRIX_PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.PRODUCTS)),
                              ("headers", plan["headers"] + sorted(gate.MATRIX_PRODUCTS))):
            altered = copy.deepcopy(plan)
            altered[field] = values
            variants.append(altered)
        for altered in variants:
            with self.subTest(plan=altered), patch.object(gate, "build_affected_plan", return_value=altered):
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())

    def test_actual_missing_inputs_extra_dependents_and_mixed_changes_fail(self):
        original = self.head
        for name in sorted(gate.EDIT_DISTANCE_REQUIRED_UNITS | gate.EDIT_DISTANCE_PRODUCTS):
            with self.subTest(missing=name):
                self.git("checkout", "--quiet", original)
                (self.root / name).unlink()
                self.head = self.commit("remove edit-distance input\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "(Required EDIT_DISTANCE units|Production EDIT_DISTANCE headers)"):
                    gate.selected(self.root, *self.push())
        for name in ("test/unit/edit_distance_extra.cpp", "string/edit_distance_extra.hpp"):
            with self.subTest(extra=name):
                self.git("checkout", "--quiet", original)
                self.write(name, '#include "string/edit_distance.hpp"\n')
                self.head = self.commit("extra dependent requires review\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "exceeds reviewed gate budget"):
                    gate.selected(self.root, *self.push())
        for name in ("components/dynamic_bit_vector.hpp", "components/matrix_arithmetic.hpp"):
            with self.subTest(mixed=name):
                self.git("checkout", "--quiet", original)
                self.write(name, "#pragma once\nusing mixed_changed = int;\n")
                self.head = self.commit("mixed intrinsic changes\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "exactly one reviewed intrinsic mode"):
                    gate.selected(self.root, *self.push())

    def test_dependencies_require_exact_current_header_for_every_target(self):
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="edit-distance-deps-") as directory:
            depfile = Path(directory) / "unit.d"
            for target in sorted(gate.EDIT_DISTANCE_REQUIRED_UNITS | gate.EDIT_DISTANCE_PRODUCTS):
                generated = target in gate.EDIT_DISTANCE_PRODUCTS
                source = Path(directory) / "header.cpp" if generated else self.root / target
                if generated:
                    source.write_text(gate.header_source(target))
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source)} | {str(self.root / name) for name in names}
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                proof = gate.dependency_proof(self.root, tree, depfile, source, target, generated, "edit_distance")
                self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, names)
                self.assertEqual(set(proof["production_headers"]), gate.EDIT_DISTANCE_PRODUCTS)
                paths.remove(str(self.root / "string/edit_distance.hpp"))
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                with self.subTest(target=target), self.assertRaisesRegex(ValueError, "current production EDIT_DISTANCE headers"):
                    gate.dependency_proof(self.root, tree, depfile, source, target, generated, "edit_distance")

    def test_external_unknown_missing_source_and_changed_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/edit_distance.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="edit-distance-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "edit_distance.hpp"
            old.write_bytes(tree.content("string/edit_distance.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="edit_distance")
            paths = {str(self.root / name) for name in {target} | gate.EDIT_DISTANCE_PRODUCTS}
            depfile.write_text("gate-output: " + " ".join(sorted(paths | {str(self.root / 'math/unrelated.hpp')})) + "\n")
            with self.assertRaisesRegex(ValueError, "not resolved by reviewed source selector"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="edit_distance")
            depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(source)})) + "\n")
            with self.assertRaisesRegex(ValueError, "actual source"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="edit_distance")
            depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
            self.write("string/edit_distance.hpp", "wrong source bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="edit_distance")

    def test_generic_target_keeps_runtime_dispatch_and_native_guards(self):
        self.assertEqual(list(gate.family_modes("edit_distance")), ["native", "generic"])
        self.assertEqual(gate.mode_flags("native", "edit_distance"), gate.COMMON + gate.MODES["native"])
        self.assertEqual(gate.mode_flags("generic", "edit_distance"), gate.COMMON + [
            "-march=x86-64", "-mtune=generic", "-mno-avx", "-mno-avx2", "-mno-avx512f", "-mno-bmi", "-mno-bmi2"])
        for mode in ("native", "generic"):
            self.assertEqual([flag for flag in gate.mode_flags(mode, "edit_distance") if flag.startswith("-D")],
                             ["-DONLINE_JUDGE", "-DATCODER"])
        self.assertEqual(gate.macro_proof(MACROS, "generic", "edit_distance")["isa"], {})
        self.assertTrue(gate.macro_proof(NATIVE_MACROS, "native", "edit_distance")["assertions"])
        for raw, mode in ((MACROS, "native"), (NATIVE_MACROS, "generic"), (MACROS, "scalar"),
                          (MACROS + b"#define __AVX512F__ 1\n", "generic"),
                          (MACROS + b"#define __BMI__ 1\n", "generic"),
                          (MACROS + b"#define NDEBUG 1\n", "generic"),
                          (MACROS.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "generic"),
                          (MACROS.replace(b"#define assert(expr) check(expr)\n", b""), "generic"),
                          (NATIVE_MACROS.replace(b"#define __BMI2__ 1\n", b""), "native")):
            with self.subTest(raw=raw, mode=mode), self.assertRaises(ValueError):
                gate.macro_proof(raw, mode, "edit_distance")
        for family, mode in (("edit_distance", "scalar"), ("dwm", "generic"),
                             ("matrix", "generic"), ("unknown", "native")):
            with self.subTest(family=family, mode=mode):
                with self.assertRaisesRegex(ValueError, "Unknown intrinsic mode"):
                    gate.mode_flags(mode, family)
                with self.assertRaisesRegex(ValueError, "Unknown intrinsic mode"):
                    gate.macro_proof(MACROS, mode, family)

    def test_driver_runs_every_target_with_native_and_generic_flags(self):
        selected, plan = gate.selected(self.root, *self.push())
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="edit-distance-driver-") as directory:
            directory = Path(directory)
            compiler, cc1 = directory / "g++-15", directory / "cc1plus"
            compiler.write_bytes(b"compiler fixture")
            cc1.write_bytes(b"cc1 fixture")
            commands = gate.Commands(directory)
            compiled, executed, probes = [], [], []

            def run(command, *, source=b"", seconds=gate.COMMAND_SECONDS):
                command = [str(value) for value in command]
                commands.count += 1
                if command == [str(compiler), "-dumpfullversion"]:
                    return b"15.2.0\n"
                if command == [str(compiler), "-print-prog-name=cc1plus"]:
                    return str(cc1).encode() + b"\n"
                if command[0] != str(compiler):
                    self.assertEqual(len(command), 1)
                    executed.append(command[0])
                    return b""
                mode = "native" if "-march=native" in command else "generic"
                flags = gate.mode_flags(mode, "edit_distance")
                self.assertEqual(command[1:1 + len(flags)], flags)
                if "-dM" in command:
                    self.assertEqual(source, b"#include <cassert>\n")
                    probes.append(mode)
                    return NATIVE_MACROS if mode == "native" else MACROS
                self.assertEqual(command[1 + len(flags):3 + len(flags)], ["-I", str(self.root)])
                source_path, binary = Path(command[-3]), Path(command[-1])
                target = (source_path.relative_to(self.root).as_posix() if source_path.is_relative_to(self.root)
                          else source_path.read_text().split('"')[1])
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source_path)} | {str(self.root / name) for name in names}
                depfile = Path(command[command.index("-MF") + 1])
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                binary.write_bytes(b"test binary fixture")
                compiled.append((mode, target, str(binary)))
                return b""

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands), \
                 patch.object(commands, "run", side_effect=run), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "emit") as emit:
                gate.execute(self.root, selected, plan, directory)
            self.assertEqual(probes, ["native", "generic"])
            self.assertEqual([(mode, target) for mode, target, _ in compiled],
                             [(mode, target) for mode in ("native", "generic")
                              for group in ("headers", "unit") for target in plan[group]])
            self.assertEqual(executed, [binary for _, _, binary in compiled])
            self.assertEqual(commands.count, 16)
            self.assertEqual(emit.call_args.args, ("gate_completed",))


class FPSScalarReturnIntrinsicGateTests(unittest.TestCase):
    git = affected_test.AffectedVerification.git
    write = affected_test.AffectedVerification.write
    commit = affected_test.AffectedVerification.commit
    push = no_cache_route_test.NoCacheRouteTests.push
    merge = no_cache_route_test.NoCacheRouteTests.merge
    pr = no_cache_route_test.NoCacheRouteTests.pr
    headers = {
        'components/fps_product_tree.hpp',
        'components/polynomial_half_gcd.hpp',
        'convolution/convolution.hpp',
        'convolution/cyclic_convolution.hpp',
        'convolution/max_min_convolution.hpp',
        'convolution/multiplicative_convolution.hpp',
        'convolution/multivariate_convolution.hpp',
        'convolution/ordered_convolution.hpp',
        'fps/bostan_mori.hpp',
        'fps/compose.hpp',
        'fps/compositional_inverse.hpp',
        'fps/fps.hpp',
        'fps/interpolate.hpp',
        'fps/linear_recurrence.hpp',
        'fps/multipoint_eval.hpp',
        'fps/polynomial_gcd.hpp',
        'fps/power_projection.hpp',
        'fps/taylor_shift.hpp',
    }
    units = {
        'test/unit/convolution.cpp',
        'test/unit/fps.cpp',
        'test/unit/fps_algorithms.cpp',
        'test/unit/fps_tuning_advanced.cpp',
        'test/unit/fps_tuning_basic.cpp',
        'test/unit/fps_tuning_polynomial.cpp',
        'test/unit/multipoint_tree.cpp',
        'test/unit/online_convolution.cpp',
        'test/unit/ordered_convolution.cpp',
        'test/unit/polynomial_gcd.cpp',
        'test/unit/power_projection.cpp',
        'test/unit/standalone.cpp',
    }

    def setUp(self):
        no_cache_route_test.NoCacheRouteTests.setUp(self)
        self.git("checkout", "--quiet", self.base)
        self.write("fps/fps.hpp", "#pragma once\n")
        for target in self.headers - {"fps/fps.hpp"}:
            self.write(target, '#include "fps/fps.hpp"\n')
        for target in self.units:
            self.write(target, '#include "fps/fps.hpp"\n')
        self.base = self.commit("FPS baseline", bind=False)
        self.write("fps/fps.hpp", "#pragma once\nusing fps_changed = int;\n")
        self.head = self.commit("FPS candidate\n\n" + route.TRAILER)

    def test_exact_push_pr_and_main_select_all_eighteen_headers_and_twelve_units(self):
        selected, plan = gate.selected(self.root, *self.push())
        self.assertEqual(gate.FPS_PRODUCTS, {"fps/fps.hpp"})
        self.assertEqual(gate.FPS_REQUIRED_HEADERS, self.headers)
        self.assertEqual(gate.FPS_REQUIRED_UNITS, self.units)
        self.assertEqual(gate.SCOPES["fps_scalar_return"][2:], (18, 12))
        self.assertEqual(gate.scope_mode(plan), "fps_scalar_return")
        self.assertEqual(plan["headers"], sorted(self.headers))
        self.assertEqual(plan["unit"], sorted(self.units))
        self.assertEqual(selected["source_head"], self.head)
        merged = self.merge()
        for event, context in (self.pr(merged), self.push(merged, route.MAIN_REF)):
            selected, plan = gate.selected(self.root, event, context)
            self.assertEqual(selected["head"], merged)
            self.assertEqual(selected["source_head"], self.head)
            self.assertEqual(gate.scope_mode(plan), "fps_scalar_return")
            self.assertEqual(plan["headers"], sorted(self.headers))
            self.assertEqual(plan["unit"], sorted(self.units))

    def test_missing_extra_duplicate_and_mixed_scopes_fail_before_command_allocation(self):
        _, plan = gate.selected(self.root, *self.push())
        variants = []
        for group in ("headers", "unit"):
            for target in plan[group]:
                altered = copy.deepcopy(plan)
                altered[group].remove(target)
                variants.append(altered)
            extra = "test/unit/extra.cpp" if group == "unit" else "fps/extra.hpp"
            for values in (plan[group] + [extra], plan[group] + [plan[group][0]],
                           plan[group][:-1] + [extra], plan[group][:-1] + [plan[group][0]]):
                altered = copy.deepcopy(plan)
                altered[group] = values
                variants.append(altered)
        for field, values in (("changed_paths", []), ("changed_paths", ["fps/unknown.hpp"]),
                              ("changed_paths", plan["changed_paths"] + ["fps/compose.hpp"]),
                              ("changed_paths", plan["changed_paths"] + ["math/unrelated.hpp"])):
            altered = copy.deepcopy(plan)
            altered[field] = values
            variants.append(altered)
        for products in (gate.PRODUCTS, gate.MATRIX_PRODUCTS, gate.EDIT_DISTANCE_PRODUCTS):
            for group in ("changed_paths", "headers"):
                altered = copy.deepcopy(plan)
                altered[group] += sorted(products)
                variants.append(altered)
        for altered in variants:
            with self.subTest(plan=altered), patch.object(gate, "Commands") as commands:
                with self.assertRaises(ValueError):
                    gate.scope_mode(altered)
                with self.assertRaises(ValueError):
                    gate.execute(self.root, {}, altered, self.root)
                commands.assert_not_called()

    def test_actual_missing_extra_and_mixed_targets_fail_closed(self):
        original = self.head
        for name in ("fps/fps.hpp", "components/fps_product_tree.hpp", "test/unit/fps.cpp"):
            with self.subTest(missing=name):
                self.git("checkout", "--quiet", original)
                (self.root / name).unlink()
                self.head = self.commit("remove FPS input\n\n" + route.TRAILER)
                with self.assertRaisesRegex(ValueError, "(exact unique reviewed|may change only fps/fps.hpp)"):
                    gate.selected(self.root, *self.push())
        for name in ("test/unit/fps_extra.cpp", "fps/extra.hpp"):
            with self.subTest(extra=name):
                self.git("checkout", "--quiet", original)
                self.write(name, '#include "fps/fps.hpp"\n')
                self.head = self.commit("extra FPS dependent\n\n" + route.TRAILER)
                with self.assertRaises(ValueError):
                    gate.selected(self.root, *self.push())
        self.git("checkout", "--quiet", original)
        self.write("components/dynamic_bit_vector.hpp", "#pragma once\nusing mixed = int;\n")
        self.head = self.commit("mixed FPS production changes\n\n" + route.TRAILER)
        with self.assertRaisesRegex(ValueError, "exactly one reviewed intrinsic mode"):
            gate.selected(self.root, *self.push())

    def test_every_target_requires_exact_current_production_dependency_bytes(self):
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="fps-deps-") as directory:
            depfile = Path(directory) / "target.d"
            for target in sorted(self.headers | self.units):
                generated = target in self.headers
                source = Path(directory) / "header.cpp" if generated else self.root / target
                if generated:
                    source.write_text(gate.header_source(target))
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source)} | {str(self.root / name) for name in names}
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                proof = gate.dependency_proof(self.root, tree, depfile, source, target, generated, "fps_scalar_return")
                self.assertEqual({item["project_path"] for item in proof["project_dependencies"]}, names)
                self.assertEqual(proof["production_headers"], ["fps/fps.hpp"])
                for item in proof["project_dependencies"]:
                    self.assertEqual(item["sha256"], gate.digest(tree.content(item["project_path"])))
                paths.remove(str(self.root / "fps/fps.hpp"))
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                with self.subTest(target=target), self.assertRaisesRegex(ValueError, "current production FPS_SCALAR_RETURN headers"):
                    gate.dependency_proof(self.root, tree, depfile, source, target, generated, "fps_scalar_return")

    def test_foreign_unknown_missing_source_and_changed_dependencies_fail(self):
        tree = SourceTree(self.root, self.head)
        target = "test/unit/fps.cpp"
        source = self.root / target
        with tempfile.TemporaryDirectory(prefix="fps-baseline-") as directory:
            depfile, old = Path(directory) / "unit.d", Path(directory) / "fps.hpp"
            old.write_bytes(tree.content("fps/fps.hpp"))
            depfile.write_text(f"gate-output: {source} {old}\n")
            with self.assertRaisesRegex(ValueError, "escaped production checkout"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="fps_scalar_return")
            paths = {str(self.root / name) for name in {target, "fps/fps.hpp"}}
            depfile.write_text("gate-output: " + " ".join(sorted(paths | {str(self.root / 'math/unrelated.hpp')})) + "\n")
            with self.assertRaisesRegex(ValueError, "not resolved by reviewed source selector"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="fps_scalar_return")
            depfile.write_text("gate-output: " + " ".join(sorted(paths - {str(source)})) + "\n")
            with self.assertRaisesRegex(ValueError, "actual source"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="fps_scalar_return")
            depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
            self.write("fps/fps.hpp", "wrong source bytes\n")
            with self.assertRaisesRegex(ValueError, "differ from exact head"):
                gate.dependency_proof(self.root, tree, depfile, source, target, family="fps_scalar_return")

    def test_generic_native_labels_and_compiler_assertion_lto_guards(self):
        family = "fps_scalar_return"
        self.assertEqual(list(gate.family_modes(family)), ["native", "generic"])
        self.assertEqual(gate.COMMON, ["-O2", "-std=gnu++23", "-ftrivial-auto-var-init=zero", "-fmodules",
                                      "-DONLINE_JUDGE", "-DATCODER", "-fno-lto"])
        self.assertEqual(gate.mode_flags("native", family), gate.COMMON + ["-march=native"])
        self.assertEqual(gate.mode_flags("generic", family), gate.COMMON + [
            "-march=x86-64", "-mtune=generic", "-mno-avx", "-mno-avx2", "-mno-avx512f", "-mno-bmi", "-mno-bmi2"])
        for mode, raw in (("native", NATIVE_MACROS), ("generic", MACROS)):
            proof = gate.macro_proof(raw, mode, family)
            self.assertEqual(proof["runtime_ntt_simd"], "unchanged; may execute in native and generic modes")
            self.assertTrue(proof["assertions"])
            self.assertEqual(proof["gcc"], "15.2.0")
            self.assertEqual([flag for flag in gate.mode_flags(mode, family) if flag.startswith("-D")],
                             ["-DONLINE_JUDGE", "-DATCODER"])
        for raw, mode in ((MACROS, "native"), (NATIVE_MACROS, "generic"), (MACROS, "scalar"),
                          (MACROS + b"#define __AVX512F__ 1\n", "generic"),
                          (MACROS + b"#define __BMI__ 1\n", "generic"),
                          (MACROS + b"#define NDEBUG 1\n", "generic"),
                          (MACROS.replace(b"__GNUC_MINOR__ 2", b"__GNUC_MINOR__ 3"), "generic"),
                          (MACROS.replace(b"#define assert(expr) check(expr)\n", b""), "generic"),
                          (MACROS.replace(b"__x86_64__ 1", b"__x86_64__ 0"), "generic"),
                          (NATIVE_MACROS.replace(b"#define __BMI2__ 1\n", b""), "native")):
            with self.subTest(raw=raw, mode=mode), self.assertRaises(ValueError):
                gate.macro_proof(raw, mode, family)
        with self.assertRaisesRegex(ValueError, "Unknown intrinsic mode"):
            gate.mode_flags("scalar", family)

    def test_mock_driver_uses_all_124_commands_and_reports_effective_budget(self):
        selected, plan = gate.selected(self.root, *self.push())
        tree = SourceTree(self.root, self.head)
        with tempfile.TemporaryDirectory(prefix="fps-driver-") as directory:
            directory = Path(directory)
            compiler, cc1 = directory / "g++-15", directory / "cc1plus"
            compiler.write_bytes(b"compiler fixture")
            cc1.write_bytes(b"cc1 fixture")
            commands = gate.Commands(directory, max_commands=gate.FPS_MAX_COMMANDS)
            compiled, executed, probes = [], [], []

            def run(command, *, source=b"", seconds=gate.COMMAND_SECONDS):
                command = [str(value) for value in command]
                commands.count += 1
                self.assertLessEqual(commands.count, commands.max_commands)
                if command == [str(compiler), "-dumpfullversion"]:
                    return b"15.2.0\n"
                if command == [str(compiler), "-print-prog-name=cc1plus"]:
                    return str(cc1).encode() + b"\n"
                if command[0] != str(compiler):
                    self.assertEqual(len(command), 1)
                    executed.append(command[0])
                    return b""
                mode = "native" if "-march=native" in command else "generic"
                flags = gate.mode_flags(mode, "fps_scalar_return")
                self.assertEqual(command[1:1 + len(flags)], flags)
                if "-dM" in command:
                    self.assertEqual(source, b"#include <cassert>\n")
                    probes.append(mode)
                    return NATIVE_MACROS if mode == "native" else MACROS
                self.assertEqual(command[1 + len(flags):3 + len(flags)], ["-I", str(self.root)])
                source_path, binary = Path(command[-3]), Path(command[-1])
                target = (source_path.relative_to(self.root).as_posix() if source_path.is_relative_to(self.root)
                          else source_path.read_text().split('"')[1])
                names = {name for name in tree.closure(target) if tree.content(name) is not None}
                paths = {str(source_path)} | {str(self.root / name) for name in names}
                depfile = Path(command[command.index("-MF") + 1])
                depfile.write_text("gate-output: " + " ".join(sorted(paths)) + "\n")
                binary.write_bytes(b"test binary fixture")
                compiled.append((mode, target, str(binary)))
                return b""

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands) as constructor, \
                 patch.object(commands, "run", side_effect=run), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "emit") as emit:
                gate.execute(self.root, selected, plan, directory)
            constructor.assert_called_once_with(directory, max_commands=124)
            self.assertEqual(probes, ["native", "generic"])
            self.assertEqual([(mode, target) for mode, target, _ in compiled],
                             [(mode, target) for mode in ("native", "generic")
                              for group in ("headers", "unit") for target in plan[group]])
            self.assertEqual(executed, [binary for _, _, binary in compiled])
            self.assertEqual(commands.count, 124)
            receipt = next(call.kwargs for call in emit.call_args_list if call.args == ("scope",))
            self.assertEqual(receipt["max_commands"], 124)
            self.assertEqual((receipt["max_seconds"], receipt["max_log_bytes"]), (18 * 60, 1024 * 1024))
            labels = [call.kwargs["mode"] for call in emit.call_args_list if call.args == ("macro_proof",)]
            self.assertEqual(labels, ["native", "generic"])
            self.assertEqual(emit.call_args.args, ("gate_completed",))
            self.assertEqual(emit.call_args.kwargs["commands"], 124)
            self.assertFalse(emit.call_args.kwargs["cache_written"])
            self.assertFalse(emit.call_args.kwargs["artifacts_uploaded"])

    def test_wrong_compiler_version_stops_without_substitution_or_retry(self):
        selected, plan = gate.selected(self.root, *self.push())
        with tempfile.TemporaryDirectory(prefix="fps-compiler-") as directory:
            directory = Path(directory)
            compiler = directory / "g++-15"
            compiler.write_bytes(b"compiler fixture")
            commands = Mock()
            commands.run.return_value = b"15.3.0\n"

            def fixture_path(value):
                return compiler if value == "/usr/bin/g++-15" else Path(value)

            with patch.object(gate, "Commands", return_value=commands), \
                 patch.object(gate, "Path", side_effect=fixture_path), patch.object(gate, "SourceTree") as tree:
                with self.assertRaisesRegex(ValueError, "Exact GCC 15.2.0 is required"):
                    gate.execute(self.root, selected, plan, directory)
            commands.run.assert_called_once_with([compiler, "-dumpfullversion"], seconds=30)
            tree.assert_not_called()

    def test_other_families_keep_default_64_after_scope_validation(self):
        self.assertEqual(gate.MAX_COMMANDS, 64)
        self.assertEqual(gate.Commands(self.root).max_commands, 64)
        for family in ("dwm", "matrix", "edit_distance"):
            products, units, _, _ = gate.SCOPES[family]
            plan = {"changed_paths": sorted(products), "headers": sorted(products), "unit": sorted(units)}
            with self.subTest(family=family), patch.object(gate, "Commands", side_effect=RuntimeError("stop before compiler")) as commands:
                with self.assertRaisesRegex(RuntimeError, "stop before compiler"):
                    gate.execute(self.root, {}, plan, self.root)
                commands.assert_called_once_with(self.root, max_commands=64)


class IntrinsicCommandTests(unittest.TestCase):
    def test_fps_124th_command_succeeds_125th_fails_and_default_stays_64(self):
        directory = Path("/tmp/intrinsic-command-fixture")
        for budget in (64, 124):
            commands = gate.Commands(directory, max_commands=budget)
            process = Mock(returncode=0)
            process.communicate.return_value = (b"", None)
            with self.subTest(budget=budget), patch.object(gate.subprocess, "Popen") as popen, patch.object(gate, "emit"):
                popen.return_value.__enter__.return_value = process
                for _ in range(budget):
                    self.assertEqual(commands.run(["fixture-binary"]), b"")
                self.assertEqual(commands.count, budget)
                self.assertEqual(popen.call_count, budget)
                with self.assertRaisesRegex(ValueError, "Command budget exceeded"):
                    commands.run(["fixture-binary"])
                self.assertEqual(popen.call_count, budget)
        self.assertEqual(gate.Commands(directory).max_commands, 64)
        for budget in (0, 63, 65, 123, 125, 1000):
            with self.subTest(unreviewed_budget=budget), self.assertRaisesRegex(ValueError, "Unreviewed command budget"):
                gate.Commands(directory, max_commands=budget)

    def test_fps_budget_retains_environment_timeout_total_time_and_log_limits(self):
        self.assertEqual((gate.JOB_SECONDS, gate.COMMAND_SECONDS, gate.MAX_LOG_BYTES), (1080, 120, 1024 * 1024))
        directory = Path("/tmp/intrinsic-command-fixture")
        with patch.object(gate.time, "monotonic", return_value=100):
            commands = gate.Commands(directory, max_commands=124)
            self.assertEqual(commands.deadline, 1180)
            self.assertEqual(commands.environment, {"PATH": "/usr/bin:/bin", "HOME": str(directory),
                             "TMPDIR": str(directory), "LC_ALL": "C", "TZ": "UTC"})
            process = Mock(returncode=0)
            process.communicate.return_value = (b"", None)
            with patch.object(gate.subprocess, "Popen") as popen, patch.object(gate, "emit"):
                popen.return_value.__enter__.return_value = process
                commands.run(["fixture-binary"])
                process.communicate.assert_called_once_with(b"", timeout=120)
                self.assertEqual(popen.call_args.kwargs["env"], commands.environment)
                self.assertTrue(popen.call_args.kwargs["start_new_session"])
                commands.deadline = 99
                with self.assertRaisesRegex(ValueError, "Total runtime budget exceeded"):
                    commands.run(["fixture-binary"])
                self.assertEqual(popen.call_count, 1)
        line = json.dumps({"event": "fixture"}, sort_keys=True, ensure_ascii=True)
        with patch.object(gate, "LOG_BYTES", gate.MAX_LOG_BYTES - len(line) - 1), patch("builtins.print") as output:
            gate.emit("fixture")
            self.assertEqual(gate.LOG_BYTES, gate.MAX_LOG_BYTES)
            with self.assertRaisesRegex(ValueError, "Normal log budget exceeded"):
                gate.emit("fixture")
            output.assert_called_once()

    def test_clean_environment_and_nonzero_exit_fail_closed(self):
        directory = Path("/tmp/intrinsic-command-fixture")
        commands = gate.Commands(directory)
        self.assertEqual(commands.environment, {"PATH": "/usr/bin:/bin", "HOME": str(directory),
                         "TMPDIR": str(directory), "LC_ALL": "C", "TZ": "UTC"})
        process = Mock(returncode=3)
        process.communicate.return_value = (b"assertion failed", None)
        with patch.object(gate.subprocess, "Popen") as popen, patch.object(gate, "emit"):
            popen.return_value.__enter__.return_value = process
            with self.assertRaisesRegex(ValueError, "Command failed: 1"):
                commands.run(["fixture-binary"])
            self.assertEqual(popen.call_args.kwargs["env"], commands.environment)
            self.assertTrue(popen.call_args.kwargs["start_new_session"])

    def test_timeout_kills_group_and_cannot_be_reported_as_success(self):
        commands = gate.Commands(Path("/tmp/intrinsic-command-fixture"))
        process = Mock(pid=1234, returncode=0)
        process.communicate.side_effect = [subprocess.TimeoutExpired(["fixture-binary"], 1), (b"", None)]
        with patch.object(gate.subprocess, "Popen") as popen, patch.object(gate.os, "killpg") as kill, \
             patch.object(gate, "emit") as emit:
            popen.return_value.__enter__.return_value = process
            with self.assertRaisesRegex(ValueError, "Command failed: 1"):
                commands.run(["fixture-binary"], seconds=1)
            kill.assert_called_once_with(1234, signal.SIGKILL)
            self.assertTrue(emit.call_args.kwargs["timed_out"])

    def test_command_and_total_time_budgets_fail_before_launch(self):
        commands = gate.Commands(Path("/tmp/intrinsic-command-fixture"))
        with patch.object(gate.subprocess, "Popen") as popen:
            commands.count = gate.MAX_COMMANDS
            with self.assertRaisesRegex(ValueError, "Command budget exceeded"):
                commands.run(["fixture-binary"])
            commands.count, commands.deadline = 0, 0
            with self.assertRaisesRegex(ValueError, "Total runtime budget exceeded"):
                commands.run(["fixture-binary"])
            popen.assert_not_called()


if __name__ == "__main__":
    unittest.main()
