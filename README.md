# Cloakwork

Cloakwork is a header-only C++20 obfuscation library for Windows. It provides comprehensive protections against static and dynamic analysis -- string encryption, value obfuscation, control flow flattening, anti-debug, anti-VM, import hiding, direct syscalls, and more. No dependencies, no build step: drop in a single header and go. Supports both user mode and kernel mode drivers.

> Inspired by [obfusheader.h](https://github.com/ac3ss0r/obfusheader.h), Zapcrash's nimrodhide.h, and qengine.

**Author:** ck0i on Discord | **License:** MIT

---

## Quick Start

```cpp
#include "cloakwork.h"
```

```cpp
// encrypted at compile-time, decrypted at runtime
const char* secret = CW_STR("my secret string");
```

```cpp
// compile-time FNV-1a hash for API name hiding
constexpr uint32_t hash = CW_HASH("kernel32.dll");
constexpr uint32_t hash_ci = CW_HASH_CI("ntdll.dll");
```

```cpp
// obfuscated integer with random key encoding
int key = CW_INT(0xDEAD);
```

```cpp
// resolve API without import table entry
auto pVirtualAlloc = CW_IMPORT("kernel32.dll", VirtualAlloc);
```

```cpp
// crash if debugger detected, or check as bool
CW_ANTI_DEBUG();
if (CW_CHECK_DEBUG()) { /* debugger present */ }
```

```cpp
// crash if VM/sandbox detected, or check as bool
CW_ANTI_VM();
if (CW_CHECK_VM()) { /* virtualized */ }
```

```cpp
// wrap code in an encrypted state machine
int result = CW_PROTECT(int, {
    if (x > 10) return x * 2;
    return x + 5;
});
```

```cpp
// indirect syscall via ntdll gadget (x64)
NTSTATUS status = CW_SYSCALL(NtClose, handle);
```

---

## Configuration

Define feature macros **before** including the header. All features are enabled by default.

| Macro | Description | Default |
|-------|-------------|---------|
| `CW_ENABLE_ALL` | Master on/off switch | `1` |
| `CW_ENABLE_STRING_ENCRYPTION` | XTEA compile-time string encryption | `1` |
| `CW_ENABLE_VALUE_OBFUSCATION` | Integer/value obfuscation and MBA | `1` |
| `CW_ENABLE_CONTROL_FLOW` | Control flow obfuscation | `1` |
| `CW_ENABLE_ANTI_DEBUG` | Anti-debugging features | `1` |
| `CW_ENABLE_FUNCTION_OBFUSCATION` | Function pointer obfuscation | `1` |
| `CW_ENABLE_DATA_HIDING` | Scattered/polymorphic values | `1` |
| `CW_ENABLE_METAMORPHIC` | Metamorphic code generation | `1` |
| `CW_ENABLE_COMPILE_TIME_RANDOM` | Compile-time random generation | `1` |
| `CW_ENABLE_IMPORT_HIDING` | Dynamic API resolution | `1` |
| `CW_ENABLE_SYSCALLS` | Direct syscall invocation | `1` |
| `CW_ENABLE_ANTI_VM` | Anti-VM/sandbox detection | `1` |
| `CW_ENABLE_INTEGRITY_CHECKS` | Code integrity verification | `1` |
| `CW_ANTI_DEBUG_RESPONSE` | Debugger response: 0=ignore, 1=crash, 2=fake data | `1` |

If you disable `CW_ENABLE_ALL` and selectively re-enable features, note that
`CW_ENABLE_ANTI_DEBUG` depends on both `CW_ENABLE_COMPILE_TIME_RANDOM` and
`CW_ENABLE_STRING_ENCRYPTION`. Cloakwork now emits a compile-time error when
that dependency contract is violated.

---

## API Reference

### String Encryption

| Macro | Description |
|-------|-------------|
| `CW_STR(s)` | XTEA-encrypted string, decrypts at runtime |
| `CW_STR_LAYERED(s)` | Multi-layer encryption with polymorphic re-encryption |
| `CW_STR_STACK(s)` | Stack-based encryption with automatic secure wipe on scope exit |
| `CW_WSTR(s)` | Wide string (wchar_t) encryption |
| `CW_STACK_STR(name, ...)` | Char-by-char stack builder, no string literal in binary |

### String Hashing

| Macro | Description |
|-------|-------------|
| `CW_HASH(s)` | Compile-time FNV-1a hash (case-sensitive) |
| `CW_HASH_CI(s)` | Compile-time FNV-1a hash (case-insensitive) |
| `CW_HASH_WIDE(s)` | Compile-time wide string hash |
| `CW_HASH_RT(str)` | Runtime FNV-1a hash (case-sensitive) |
| `CW_HASH_RT_CI(str)` | Runtime FNV-1a hash (case-insensitive) |

### Value Obfuscation

| Macro | Description |
|-------|-------------|
| `CW_INT(x)` | Obfuscated integer with random key encoding |
| `CW_MBA(x)` | Mixed Boolean Arithmetic obfuscation |
| `CW_CONST(x)` | Encrypted compile-time constant |
| `CW_ADD(a, b)` | Obfuscated addition via MBA |
| `CW_SUB(a, b)` | Obfuscated subtraction via MBA |
| `CW_AND(a, b)` | Obfuscated bitwise AND via MBA |
| `CW_OR(a, b)` | Obfuscated bitwise OR via MBA |
| `CW_XOR(a, b)` | Obfuscated bitwise XOR via MBA |
| `CW_NEG(a)` | Obfuscated negation via MBA |

### Comparisons

| Macro | Description |
|-------|-------------|
| `CW_EQ(a, b)` | Obfuscated equality (==) |
| `CW_NE(a, b)` | Obfuscated not-equals (!=) |
| `CW_LT(a, b)` | Obfuscated less-than (<) |
| `CW_GT(a, b)` | Obfuscated greater-than (>) |
| `CW_LE(a, b)` | Obfuscated less-or-equal (<=) |
| `CW_GE(a, b)` | Obfuscated greater-or-equal (>=) |

### Booleans

| Macro | Description |
|-------|-------------|
| `CW_TRUE` | Opaque predicate that evaluates to true |
| `CW_FALSE` | Opaque predicate that evaluates to false |
| `CW_BOOL(expr)` | Obfuscate any boolean expression |

### Control Flow

| Macro | Description |
|-------|-------------|
| `CW_IF(cond)` | Obfuscated branching with opaque predicates |
| `CW_ELSE` | Obfuscated else clause |
| `CW_BRANCH(cond)` | Indirect branching with obfuscation |
| `CW_FLATTEN(func, ...)` | Control flow flattening via state machine |
| `CW_PROTECT(ret_type, body)` | Wrap code in an encrypted state machine dispatcher |
| `CW_PROTECT_VOID(body)` | Void variant of `CW_PROTECT` |
| `CW_JUNK()` | Insert junk computation |
| `CW_JUNK_FLOW()` | Insert junk with fake control flow |

### Function Protection

| Macro | Description |
|-------|-------------|
| `CW_CALL(func)` | XTEA-encrypted function pointer with decoy arrays |
| `CW_SPOOF_CALL(func)` | Call with spoofed return address |
| `CW_RET_GADGET()` | Cached ret gadget in ntdll for return address spoofing |

### Import Hiding

| Macro | Description |
|-------|-------------|
| `CW_IMPORT(mod, func)` | Dynamic resolution without import table entry |
| `CW_IMPORT_WIDE(mod, func)` | Wide string module variant |
| `CW_GET_MODULE(name)` | Get module base via PEB walk |
| `CW_GET_PROC(mod, func)` | Get export address by hash |

### Direct Syscalls

| Macro | Description |
|-------|-------------|
| `CW_SYSCALL_NUMBER(func)` | Extract syscall number with Halo's Gate fallback |
| `CW_SYSCALL(func, ...)` | Indirect invocation via ntdll gadget (x64 only) |

### Data Hiding

| Macro | Description |
|-------|-------------|
| `CW_SCATTER(x)` | Heap-scattered data across multiple allocations |
| `CW_POLY(x)` | Polymorphic mutating wrapper |

### Anti-Debug

| Macro | Description |
|-------|-------------|
| `CW_ANTI_DEBUG()` | Crashes if debugger detected (multi-technique) |
| `CW_CHECK_DEBUG()` | Returns bool, comprehensive multi-layer detection |
| `CW_HIDE_THREAD()` | Hide thread from debugger (ThreadHideFromDebugger) |

For granular checks, use the `cloakwork::anti_debug` namespace directly: `is_debugger_present()`, `has_hardware_breakpoints()`, `comprehensive_check()`, `timing_check()`, and the `enhanced` sub-namespace for debug port checks, parent process analysis, anti-anti-debug plugin detection, kernel debugger detection, and registry artifact scanning.

### Anti-VM / Sandbox

| Macro | Description |
|-------|-------------|
| `CW_ANTI_VM()` | Crashes if VM or sandbox detected |
| `CW_CHECK_VM()` | Returns bool |

For individual checks, use `cloakwork::anti_debug::anti_vm`: hypervisor detection (CPUID), VM vendor string matching (VMware, VirtualBox, Hyper-V, KVM, Xen, Parallels, QEMU), low resource detection, sandbox DLL detection, VM registry keys, VM MAC prefixes, and sandbox username/computer name detection.

### Integrity

| Macro | Description |
|-------|-------------|
| `CW_DETECT_HOOK(func)` | Check for hook patterns (jmp, push/ret, int3) at entry point |
| `CW_INTEGRITY_CHECK(func, size)` | Integrity-checked function wrapper |
| `CW_COMPUTE_HASH(ptr, size)` | Hash a memory region |
| `CW_VERIFY_FUNCS(...)` | Verify multiple functions are not hooked |

### PE / IAT

| Macro | Description |
|-------|-------------|
| `CW_ERASE_PE_HEADER()` | Zero DOS/NT headers and section table to prevent dumping |
| `CW_SCRUB_DEBUG_IMPORTS()` | Stub debug-related IAT entries (IsDebuggerPresent, etc.) |

### Random

| Macro | Description |
|-------|-------------|
| `CW_RANDOM_CT()` | Compile-time random value (unique per build) |
| `CW_RAND_CT(min, max)` | Compile-time random in range |
| `CW_RANDOM_RT()` | Runtime random value (multi-source entropy) |
| `CW_RAND_RT(min, max)` | Runtime random in range |

### Template Classes

- `cloakwork::obfuscated_value<T>` -- generic value obfuscation
- `cloakwork::mba_obfuscated<T>` -- MBA-based obfuscation
- `cloakwork::obfuscated_call<Func>` -- function pointer obfuscation
- `cloakwork::meta_func<Sig>` -- metamorphic function wrapper (alias for `metamorphic_function<Sig>`)
- `cloakwork::data_hiding::scattered_value<T, Chunks>` -- heap data scattering
- `cloakwork::data_hiding::polymorphic_value<T>` -- polymorphic mutating value
- `cloakwork::constants::runtime_constant<T>` -- runtime-keyed constant (alias: `cloakwork::rt_const<T>`)
- `cloakwork::integrity::integrity_checked<Func>` -- integrity-checked function wrapper
- `cloakwork::obf_bool` -- obfuscated boolean (multi-byte storage with opaque predicates)

---

## Kernel Mode

I'd recommend you use my other library [Kernelcloak](https://github.com/ck0i/Kernelcloak) for kernel work, it is much more in depth and Cloakwork doesn't really suit kernel work as much as other libaries do. However, if you choose to still use Cloakwork, here you go:

Kernel mode is auto-detected when WDK headers are present (`_KERNEL_MODE`, `NTDDI_VERSION`, `_NTDDK_`, `_WDMDDK_`), or forced with `#define CW_KERNEL_MODE 1`.

### Feature Availability

| Feature | Kernel Mode | Reason |
|---------|-------------|--------|
| Compile-time random | Enabled | Pure consteval |
| String hashing | Enabled | Pure consteval |
| Anti-debug | Enabled | Kernel-specific techniques |
| String encryption | No-op | Requires `atexit` for static destructors |
| Value obfuscation | No-op | Requires C++20 concepts / `std::bit_cast` |
| Control flow | No-op | Depends on value obfuscation |
| Function obfuscation | No-op | Requires C++20 concepts |
| Data hiding | No-op | Requires `std::unique_ptr` |
| Metamorphic | No-op | Requires `std::initializer_list` |
| Import hiding | No-op | PEB walking is usermode-only |
| Anti-VM | No-op | Uses usermode APIs |
| Integrity checks | No-op | Requires `VirtualQuery` |
| Syscalls | No-op | Already in kernel |

### Example

```cpp
#include <ntddk.h>
#define CW_KERNEL_MODE 1
#include "cloakwork.h"

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    UNREFERENCED_PARAMETER(RegistryPath);

    constexpr uint32_t hash = CW_HASH("NtClose");
    constexpr uint32_t key = CW_RANDOM_CT();

    if (cloakwork::anti_debug::comprehensive_check()) {
        KeBugCheckEx(0xDEAD, 0, 0, 0, 0);
    }

    return STATUS_SUCCESS;
}
```

### Kernel Anti-Debug Techniques

- **KdDebuggerEnabled** -- global flag set when kernel debugger is attached
- **KdDebuggerNotPresent** -- inverse flag (false = debugger present)
- **PsIsProcessBeingDebugged** -- per-process debug port check (dynamically resolved)
- **Debug registers** -- direct `__readdr()` intrinsic for DR0-DR3 hardware breakpoints
- **Timing analysis** -- `KeQueryPerformanceCounter` vs RDTSC for single-step detection

### Kernel Entropy Sources

Runtime random in kernel mode combines: `__rdtsc()`, `PsGetCurrentProcess()`/`PsGetCurrentThread()` (KASLR), process/thread IDs, `KeQueryPerformanceCounter()`, `KeQuerySystemTime()`, `KeQueryInterruptTime()`, pool allocation addresses, and stack addresses. Mixed via xorshift64*.

---

## Credits & License

- Inspired by [obfusheader.h](https://github.com/ac3ss0r/obfusheader.h), nimrodhide.h, qengine, and the anti-reverse-engineering community on unknowncheats.
- Created by helz.dev/Helzky | Discord: `ck0i`
- MIT License -- do what you want, no warranty.
