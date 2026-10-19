# Independent read-only review

Reviewer: `/root/inverse_sqrt/pow_native_probe`, 2026-10-19 UTC. No compilation or production edits by the reviewer.

The proposed declaration remains `= default` at its first declaration. It remains explicitly defaulted and non-user-provided. The GNU function attribute does not change implicit inline, implicit constexpr eligibility, the implicit exception specification (`noexcept(true)` here), overload resolution, access, object layout, or the default member initializer `v=0`.

The default constructor is already non-trivial because of the default member initializer. Copy/move triviality, trivial copyability, and standard layout are unchanged by the attribute. The constructor is defined inside the class and is already inline, satisfying GCC's inline prerequisite for `always_inline`.

GCC and Clang recognize `gnu::always_inline` on functions/constructors. Their actual optimization and diagnostics are implementation behavior, so the GCC O3/O2 pragma boundary and Clang compatibility require compilation/codegen confirmation. GCC can diagnose a mandatory inline that is impossible; this constructor has only an unsigned member initialization and no target feature requirement, so the review found no concrete new concern. Future additions of target requirements or a body that cannot inline would need a new check.
