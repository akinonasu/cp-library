# Dynamic Wavelet Matrix comparison

The Japanese report is [../dynamic_wavelet_quaternary.md](../dynamic_wavelet_quaternary.md).

`cases.py` generates reproducible ABC343 F inputs and independent answers.
It validates its top-two segment tree oracle against Counter + sorting on every
small-case query. Large cases use N=Q=200000 and seeds 20261020 through20261025.
The emitted JSON records input/output hashes and operation counts.

`abc343f.cpp` uses the public `DynamicWaveletMatrix<int>` (signed32-bit) API.
Compile the identical source against frozen baseline and final include roots.

| Macro | Default | Meaning |
| --- | ---: | --- |
| `DWM_FAST_IO` | 0 | 0: original unsynchronized iostream; 1: FastInput/FastOutput |
| `DWM_COUNT_DIRECT` | 0 | 0: original `range_freq(v,v+1)`; 1: `count(v)` |
| `DWM_INSTRUMENT` | 0 | 0: streaming solver; 1: separate construction/operation timers |
| `DWM_VALUE_TYPE` | int | Optional value-type control |
| `DWM_BITS` | sizeof(Value)*8 | Optional bit-width control |
| `DWM_HEADER` | public header | Optional header binding |

Instrumented mode loads operations and allocates answers before the timer,
reports construction/operation wall and process CPU times on stderr, then
writes every answer after the timer. Streaming mode reads and executes online.
The fast-I/O + direct-count consumer is a separate control.

`fetch_nim_reference.py` downloads and hashes seven fixed CC0-1.0 library files
and LICENSE from kemuniku/cplib. It bundles the supplied main with the original
lexical check settings. Both streaming and API-only source hashes are checked.
Supply an existing Nim2.2.4 compiler; the helper downloads no toolchain.
The primary reference uses QuadVector[32,256] and unsigned H=30, as supplied.

All compilation/validation uses a shared flock at `/tmp/cp-fps-tuning.lock`.
Timing runners take its exclusive lock, pin CPU2 and use blocking waits.
Do not run unrelated compilation or benchmarks during these measurements.
