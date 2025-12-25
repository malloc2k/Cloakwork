#ifndef CLOAKWORK_H
#define CLOAKWORK_H

// Cloakwork advanced obfuscation library - header-only c++20 implementation
// Comprehensive protection against static and dynamic analysis

// ██████╗██╗      ██████╗  █████╗ ██╗  ██╗██╗    ██╗ ██████╗ ██████╗ ██╗  ██╗
//██╔════╝██║     ██╔═══██╗██╔══██╗██║ ██╔╝██║    ██║██╔═══██╗██╔══██╗██║ ██╔╝
//██║     ██║     ██║   ██║███████║█████╔╝ ██║ █╗ ██║██║   ██║██████╔╝█████╔╝
//██║     ██║     ██║   ██║██╔══██║██╔═██╗ ██║███╗██║██║   ██║██╔══██╗██╔═██╗
//╚██████╗███████╗╚██████╔╝██║  ██║██║  ██╗╚███╔███╔╝╚██████╔╝██║  ██║██║  ██╗
// ╚═════╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝ ╚══╝╚══╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝

// Created by @helz.dev on Discord.
// Inspiration from obfusheader.h and Zapcrash's nimrodhide.h

// =================================================================
// COMPILE-TIME CONFIGURATION
// =================================================================
//
// Cloakwork can be configured at compile-time to include only the features you need.
// This reduces binary size and compilation time significantly.
//
// To disable a feature, define the corresponding macro as 0 before including this header:
//   #define CW_ENABLE_STRING_ENCRYPTION 0
//   #include "cloakwork.h"
//
// Alternatively, you can define them as compiler flags:
//   -DCW_ENABLE_METAMORPHIC=0
//
// By default, all features are enabled for backwards compatibility.
//
// Configuration options:
// ----------------------
// CW_ENABLE_ALL                    - master switch to enable/disable everything (default: 1)
// CW_ENABLE_STRING_ENCRYPTION      - compile-time string encryption (default: 1)
// CW_ENABLE_VALUE_OBFUSCATION      - integer/value obfuscation (default: 1)
// CW_ENABLE_CONTROL_FLOW           - control flow obfuscation (default: 1)
// CW_ENABLE_ANTI_DEBUG             - anti-debugging features (default: 1)
// CW_ENABLE_FUNCTION_OBFUSCATION   - function pointer obfuscation (default: 1)
// CW_ENABLE_DATA_HIDING            - scattered/polymorphic values (default: 1)
// CW_ENABLE_METAMORPHIC            - metamorphic code generation (default: 1)
// CW_ENABLE_COMPILE_TIME_RANDOM    - compile-time random generation (default: 1)
// CW_ENABLE_IMPORT_HIDING          - dynamic API resolution / import hiding (default: 1)
// CW_ENABLE_SYSCALLS               - direct syscall invocation (default: 1)
// CW_ENABLE_ANTI_VM                - anti-VM/sandbox detection (default: 1)
// CW_ENABLE_INTEGRITY_CHECKS       - self-integrity verification (default: 1)
// CW_ANTI_DEBUG_RESPONSE           - response to debugger detection: 0=ignore, 1=crash, 2=fake (default: 1)
//
// KERNEL MODE SUPPORT:
// --------------------
// CW_KERNEL_MODE                   - enable kernel mode compatibility (default: auto-detect)
//
// Kernel mode is auto-detected via _KERNEL_MODE or NTDDI_VERSION macros.
// You can also force kernel mode with: #define CW_KERNEL_MODE 1
//
// In kernel mode:
//   - STL containers (vector, mutex, atomic) are replaced with kernel-safe alternatives
//   - Heap operations use ExAllocatePool2/ExFreePool instead of new/malloc
//   - Anti-debug uses kernel structures (EPROCESS, ETHREAD)
//   - Import hiding walks DriverSection linked list
//   - Syscalls are disabled (already in kernel)
//   - Thread-safety uses spinlocks instead of std::mutex
//
// Kernel mode usage:
// ------------------
// #include <ntddk.h>
// #define CW_KERNEL_MODE 1  // optional, auto-detected if ntddk.h included
// #include "cloakwork.h"
//
// Minimal configuration example:
// ------------------------------
// #define CW_ENABLE_ALL 0                      // disable everything first
// #define CW_ENABLE_STRING_ENCRYPTION 1        // enable only what you need
// #define CW_ENABLE_VALUE_OBFUSCATION 1
// #include "cloakwork.h"
//
// Performance-focused configuration:
// -----------------------------------
// #define CW_ENABLE_METAMORPHIC 0              // disable heavy features
// #include "cloakwork.h"
//
// =================================================================

// =================================================================
// KERNEL MODE DETECTION
// =================================================================
// auto-detect kernel mode from WDK headers or explicit definition
#ifndef CW_KERNEL_MODE
    #if defined(_KERNEL_MODE) || defined(NTDDI_VERSION) || defined(_NTDDK_) || defined(_WDMDDK_)
        #define CW_KERNEL_MODE 1
    #else
        #define CW_KERNEL_MODE 0
    #endif
#endif

// in kernel mode, disable features that require usermode APIs or STL
#if CW_KERNEL_MODE
    // syscalls don't make sense in kernel mode
    #ifdef CW_ENABLE_SYSCALLS
        #undef CW_ENABLE_SYSCALLS
    #endif
    #define CW_ENABLE_SYSCALLS 0

    // data hiding uses std::unique_ptr - disable in kernel
    #ifdef CW_ENABLE_DATA_HIDING
        #undef CW_ENABLE_DATA_HIDING
    #endif
    #define CW_ENABLE_DATA_HIDING 0

    // metamorphic uses std::initializer_list - disable in kernel
    #ifdef CW_ENABLE_METAMORPHIC
        #undef CW_ENABLE_METAMORPHIC
    #endif
    #define CW_ENABLE_METAMORPHIC 0

    // string encryption uses static destructors which need atexit - disable in kernel
    #ifdef CW_ENABLE_STRING_ENCRYPTION
        #undef CW_ENABLE_STRING_ENCRYPTION
    #endif
    #define CW_ENABLE_STRING_ENCRYPTION 0

    // value obfuscation uses C++20 concepts and std::bit_cast - disable in kernel
    #ifdef CW_ENABLE_VALUE_OBFUSCATION
        #undef CW_ENABLE_VALUE_OBFUSCATION
    #endif
    #define CW_ENABLE_VALUE_OBFUSCATION 0

    // import hiding uses PEB walking which needs different impl in kernel
    #ifdef CW_ENABLE_IMPORT_HIDING
        #undef CW_ENABLE_IMPORT_HIDING
    #endif
    #define CW_ENABLE_IMPORT_HIDING 0

    // anti-VM uses usermode APIs
    #ifdef CW_ENABLE_ANTI_VM
        #undef CW_ENABLE_ANTI_VM
    #endif
    #define CW_ENABLE_ANTI_VM 0

    // integrity checks require usermode VirtualQuery
    #ifdef CW_ENABLE_INTEGRITY_CHECKS
        #undef CW_ENABLE_INTEGRITY_CHECKS
    #endif
    #define CW_ENABLE_INTEGRITY_CHECKS 0

    // function obfuscation uses concepts
    #ifdef CW_ENABLE_FUNCTION_OBFUSCATION
        #undef CW_ENABLE_FUNCTION_OBFUSCATION
    #endif
    #define CW_ENABLE_FUNCTION_OBFUSCATION 0

    // control flow obfuscation uses mba from value obfuscation
    #ifdef CW_ENABLE_CONTROL_FLOW
        #undef CW_ENABLE_CONTROL_FLOW
    #endif
    #define CW_ENABLE_CONTROL_FLOW 0
#endif

// default configuration - all features enabled
#ifndef CW_ENABLE_ALL
    #define CW_ENABLE_ALL 1
#endif

// individual feature flags with defaults based on master switch
#ifndef CW_ENABLE_COMPILE_TIME_RANDOM
    #define CW_ENABLE_COMPILE_TIME_RANDOM CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_STRING_ENCRYPTION
    #define CW_ENABLE_STRING_ENCRYPTION CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_VALUE_OBFUSCATION
    #define CW_ENABLE_VALUE_OBFUSCATION CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_CONTROL_FLOW
    #define CW_ENABLE_CONTROL_FLOW CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_ANTI_DEBUG
    #define CW_ENABLE_ANTI_DEBUG CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_FUNCTION_OBFUSCATION
    #define CW_ENABLE_FUNCTION_OBFUSCATION CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_DATA_HIDING
    #define CW_ENABLE_DATA_HIDING CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_METAMORPHIC
    #define CW_ENABLE_METAMORPHIC CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_IMPORT_HIDING
    #define CW_ENABLE_IMPORT_HIDING CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_SYSCALLS
    #define CW_ENABLE_SYSCALLS CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_ANTI_VM
    #define CW_ENABLE_ANTI_VM CW_ENABLE_ALL
#endif

#ifndef CW_ENABLE_INTEGRITY_CHECKS
    #define CW_ENABLE_INTEGRITY_CHECKS CW_ENABLE_ALL
#endif

#ifndef CW_ANTI_DEBUG_RESPONSE
    #define CW_ANTI_DEBUG_RESPONSE 1  // 0=ignore, 1=crash, 2=fake data
#endif

// validate configuration dependencies
#if CW_ENABLE_DATA_HIDING && !CW_ENABLE_COMPILE_TIME_RANDOM
    #error "CW_ENABLE_DATA_HIDING requires CW_ENABLE_COMPILE_TIME_RANDOM to be enabled"
#endif

#if CW_ENABLE_CONTROL_FLOW && !CW_ENABLE_COMPILE_TIME_RANDOM
    #error "CW_ENABLE_CONTROL_FLOW requires CW_ENABLE_COMPILE_TIME_RANDOM to be enabled"
#endif

// =================================================================
// includes
// =================================================================

#if CW_KERNEL_MODE
    // =================================================================
    // KERNEL MODE INCLUDES
    // =================================================================
    // assume ntddk.h is already included by the driver before cloakwork.h
    // if not, the user needs to include it themselves
    #ifndef _NTDDK_
        #error "In kernel mode, include <ntddk.h> before cloakwork.h"
    #endif

    #include <intrin.h>

    // forward declare ZwQuerySystemInformation if not available
    extern "C" {
        NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
            ULONG SystemInformationClass,
            PVOID SystemInformation,
            ULONG SystemInformationLength,
            PULONG ReturnLength
        );
    }

    // define _fltused to satisfy linker when floating point is used
    // selectany allows multiple definitions across translation units
    extern "C" __declspec(selectany) int _fltused = 0;

    // kernel mode: provide our own type definitions to avoid STL conflicts
    // WDK doesn't support usermode STL headers properly

    // fixed-width integer types for kernel mode (use Windows types)
    using int8_t = signed char;
    using int16_t = short;
    using int32_t = int;
    using int64_t = long long;
    using uint8_t = unsigned char;
    using uint16_t = unsigned short;
    using uint32_t = unsigned int;
    using uint64_t = unsigned long long;
    using size_t = SIZE_T;
    using ptrdiff_t = SSIZE_T;

    namespace std {
        // std::array replacement for kernel mode
        template<typename T, size_t N>
        struct array {
            T _data[N];

            constexpr T& operator[](size_t i) { return _data[i]; }
            constexpr const T& operator[](size_t i) const { return _data[i]; }
            constexpr T* data() { return _data; }
            constexpr const T* data() const { return _data; }
            constexpr size_t size() const { return N; }
        };

        // std::rotl/rotr replacements for kernel mode
        template<typename T>
        constexpr T rotl(T value, int shift) {
            constexpr int bits = sizeof(T) * 8;
            shift &= (bits - 1);
            if (shift == 0) return value;
            return (value << shift) | (value >> (bits - shift));
        }

        template<typename T>
        constexpr T rotr(T value, int shift) {
            constexpr int bits = sizeof(T) * 8;
            shift &= (bits - 1);
            if (shift == 0) return value;
            return (value >> shift) | (value << (bits - shift));
        }

        // std::index_sequence for kernel mode
        template<size_t... Is>
        struct index_sequence {};

        template<size_t N, size_t... Is>
        struct make_index_sequence_impl : make_index_sequence_impl<N - 1, N - 1, Is...> {};

        template<size_t... Is>
        struct make_index_sequence_impl<0, Is...> {
            using type = index_sequence<Is...>;
        };

        template<size_t N>
        using make_index_sequence = typename make_index_sequence_impl<N>::type;

        // type traits replacements for kernel mode
        template<bool B, class T = void>
        struct enable_if {};

        template<class T>
        struct enable_if<true, T> { using type = T; };

        template<bool B, class T = void>
        using enable_if_t = typename enable_if<B, T>::type;

        template<class T, class U>
        struct is_same { static constexpr bool value = false; };

        template<class T>
        struct is_same<T, T> { static constexpr bool value = true; };

        template<class T, class U>
        inline constexpr bool is_same_v = is_same<T, U>::value;

        template<class T>
        struct remove_cv { using type = T; };
        template<class T>
        struct remove_cv<const T> { using type = T; };
        template<class T>
        struct remove_cv<volatile T> { using type = T; };
        template<class T>
        struct remove_cv<const volatile T> { using type = T; };

        template<class T>
        using remove_cv_t = typename remove_cv<T>::type;

        template<class T>
        struct remove_reference { using type = T; };
        template<class T>
        struct remove_reference<T&> { using type = T; };
        template<class T>
        struct remove_reference<T&&> { using type = T; };

        template<class T>
        using remove_reference_t = typename remove_reference<T>::type;

        template<class T>
        struct is_integral { static constexpr bool value = false; };
        template<> struct is_integral<bool> { static constexpr bool value = true; };
        template<> struct is_integral<char> { static constexpr bool value = true; };
        template<> struct is_integral<signed char> { static constexpr bool value = true; };
        template<> struct is_integral<unsigned char> { static constexpr bool value = true; };
        // wchar_t is unsigned short in kernel mode with /Zc:wchar_t-, skip to avoid duplicate
        template<> struct is_integral<short> { static constexpr bool value = true; };
        template<> struct is_integral<unsigned short> { static constexpr bool value = true; };
        template<> struct is_integral<int> { static constexpr bool value = true; };
        template<> struct is_integral<unsigned int> { static constexpr bool value = true; };
        template<> struct is_integral<long> { static constexpr bool value = true; };
        template<> struct is_integral<unsigned long> { static constexpr bool value = true; };
        template<> struct is_integral<long long> { static constexpr bool value = true; };
        template<> struct is_integral<unsigned long long> { static constexpr bool value = true; };

        template<class T>
        inline constexpr bool is_integral_v = is_integral<remove_cv_t<T>>::value;

        template<class T>
        struct is_floating_point { static constexpr bool value = false; };
        template<> struct is_floating_point<float> { static constexpr bool value = true; };
        template<> struct is_floating_point<double> { static constexpr bool value = true; };
        template<> struct is_floating_point<long double> { static constexpr bool value = true; };

        template<class T>
        inline constexpr bool is_floating_point_v = is_floating_point<remove_cv_t<T>>::value;

        template<class T>
        struct is_arithmetic { static constexpr bool value = is_integral_v<T> || is_floating_point_v<T>; };

        template<class T>
        inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

        template<class T>
        struct is_pointer { static constexpr bool value = false; };
        template<class T>
        struct is_pointer<T*> { static constexpr bool value = true; };
        template<class T>
        struct is_pointer<T* const> { static constexpr bool value = true; };
        template<class T>
        struct is_pointer<T* volatile> { static constexpr bool value = true; };
        template<class T>
        struct is_pointer<T* const volatile> { static constexpr bool value = true; };

        template<class T>
        inline constexpr bool is_pointer_v = is_pointer<T>::value;

        template<class T>
        T&& declval() noexcept;

        template<class T>
        constexpr T&& forward(remove_reference_t<T>& t) noexcept {
            return static_cast<T&&>(t);
        }

        template<class T>
        constexpr T&& forward(remove_reference_t<T>&& t) noexcept {
            return static_cast<T&&>(t);
        }

        template<class T>
        constexpr remove_reference_t<T>&& move(T&& t) noexcept {
            return static_cast<remove_reference_t<T>&&>(t);
        }
    }

    namespace cloakwork_internal {
        // kernel spinlock wrapper for thread safety
        class kernel_spinlock {
        private:
            KSPIN_LOCK lock;
            KIRQL old_irql;

        public:
            kernel_spinlock() {
                KeInitializeSpinLock(&lock);
            }

            void acquire() {
                KeAcquireSpinLock(&lock, &old_irql);
            }

            void release() {
                KeReleaseSpinLock(&lock, old_irql);
            }
        };

        // simple spinlock guard (RAII)
        class spinlock_guard {
        private:
            kernel_spinlock& lock;
        public:
            spinlock_guard(kernel_spinlock& l) : lock(l) { lock.acquire(); }
            ~spinlock_guard() { lock.release(); }
            spinlock_guard(const spinlock_guard&) = delete;
            spinlock_guard& operator=(const spinlock_guard&) = delete;
        };

        // minimal atomic replacement for kernel mode
        // uses interlocked operations
        template<typename T>
        class kernel_atomic {
        private:
            volatile T value;

        public:
            kernel_atomic() : value{} {}
            kernel_atomic(T val) : value(val) {}

            T load(int = 0) const {
                // full memory barrier
                MemoryBarrier();
                return value;
            }

            void store(T val, int = 0) {
                value = val;
                MemoryBarrier();
            }

            T fetch_add(T val, int = 0) {
                if constexpr (sizeof(T) == 4) {
                    return static_cast<T>(InterlockedExchangeAdd(
                        reinterpret_cast<volatile LONG*>(&value),
                        static_cast<LONG>(val)));
                } else if constexpr (sizeof(T) == 8) {
                    return static_cast<T>(InterlockedExchangeAdd64(
                        reinterpret_cast<volatile LONG64*>(&value),
                        static_cast<LONG64>(val)));
                } else {
                    T old = value;
                    value += val;
                    return old;
                }
            }

            T operator++() {
                return fetch_add(1) + 1;
            }

            T operator++(int) {
                return fetch_add(1);
            }
        };

        // kernel pool allocation wrapper
        inline void* kernel_alloc(size_t size) {
            // use NonPagedPoolNx for security (no-execute)
            return ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 'kwlC');
        }

        inline void kernel_free(void* ptr) {
            if (ptr) {
                ExFreePoolWithTag(ptr, 'kwlC');
            }
        }

        // LDR_DATA_TABLE_ENTRY for kernel driver enumeration
        // (different structure than user mode PEB version)
        struct KLDR_DATA_TABLE_ENTRY {
            LIST_ENTRY InLoadOrderLinks;
            PVOID ExceptionTable;
            ULONG ExceptionTableSize;
            PVOID GpValue;
            PVOID NonPagedDebugInfo;
            PVOID DllBase;
            PVOID EntryPoint;
            ULONG SizeOfImage;
            UNICODE_STRING FullDllName;
            UNICODE_STRING BaseDllName;
            ULONG Flags;
            USHORT LoadCount;
            USHORT __Unused;
            PVOID SectionPointer;
            ULONG CheckSum;
            ULONG TimeDateStamp;
        };
    }

    // type aliases for cross-mode compatibility
    #define CW_ATOMIC(T) cloakwork_internal::kernel_atomic<T>
    #define CW_MUTEX cloakwork_internal::kernel_spinlock
    #define CW_LOCK_GUARD(m) cloakwork_internal::spinlock_guard _cw_guard(m)

#else
    // =================================================================
    // USER MODE INCLUDES
    // =================================================================
    #include <vector>
    #include <algorithm>
    #include <atomic>
    #include <mutex>
    #include <memory>

    #ifdef _WIN32
        #include <windows.h>
        #include <intrin.h>
        #include <winternl.h>
        #include <tlhelp32.h>
        #include <iphlpapi.h>
        #pragma comment(lib, "iphlpapi.lib")

        // full LDR_DATA_TABLE_ENTRY structure (winternl.h has incomplete definition, fuck those guys)
        namespace cloakwork_internal {
            struct CW_LDR_DATA_TABLE_ENTRY {
                LIST_ENTRY InLoadOrderLinks;
                LIST_ENTRY InMemoryOrderLinks;
                LIST_ENTRY InInitializationOrderLinks;
                PVOID DllBase;
                PVOID EntryPoint;
                ULONG SizeOfImage;
                UNICODE_STRING FullDllName;
                UNICODE_STRING BaseDllName;
                ULONG Flags;
                USHORT LoadCount;
                USHORT TlsIndex;
                union {
                    LIST_ENTRY HashLinks;
                    struct {
                        PVOID SectionPointer;
                        ULONG CheckSum;
                    };
                };
                union {
                    ULONG TimeDateStamp;
                    PVOID LoadedImports;
                };
                PVOID EntryPointActivationContext;
                PVOID PatchInformation;
            };
        }
    #endif

    // type aliases for cross-mode compatibility
    #define CW_ATOMIC(T) std::atomic<T>
    #define CW_MUTEX std::mutex
    #define CW_LOCK_GUARD(m) std::lock_guard<std::mutex> _cw_guard(m)

#endif // CW_KERNEL_MODE

// compiler detection and configuration
#ifdef _MSC_VER
    #define CW_FORCEINLINE __forceinline
    #define CW_NOINLINE __declspec(noinline)
    #define CW_SECTION(x) __declspec(allocate(x))
    #define CW_COMPILER_BARRIER() _ReadWriteBarrier()
    #pragma warning(push)
    #pragma warning(disable: 4996 4244 4267)
#elif defined(__GNUC__) || defined(__clang__)
    #define CW_FORCEINLINE __attribute__((always_inline)) inline
    #define CW_NOINLINE __attribute__((noinline))
    #define CW_SECTION(x) __attribute__((section(x)))
    #define CW_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
    #define CW_FORCEINLINE inline
    #define CW_NOINLINE
    #define CW_SECTION(x)
    #define CW_COMPILER_BARRIER() std::atomic_signal_fence(std::memory_order_seq_cst)
#endif

// =================================================================
// CLOAKWORK QUICK REFERENCE WIKI
// =================================================================
//
// STRING ENCRYPTION
// -----------------
// CW_STR("text")                   - encrypts string at compile-time, decrypts at runtime
//                                    usage: const char* msg = CW_STR("secret message");
//
// CW_STR_LAYERED("text")           - multi-layer encrypted string with polymorphic re-encryption
//                                    usage: const char* msg = CW_STR_LAYERED("secret");
//
// CW_STR_STACK("text")              - stack-based encrypted string (auto-cleanup)
//                                    usage: auto msg = CW_STR_STACK("secret");
//
// INTEGER/VALUE OBFUSCATION
// -------------------------
// CW_INT(value)                    - obfuscates integer/numeric values
//                                    usage: int x = CW_INT(42);
//
// CW_ADD(a, b)                     - obfuscated addition using MBA
//                                    usage: int sum = CW_ADD(x, y);
//
// CW_SUB(a, b)                     - obfuscated subtraction using MBA
//                                    usage: int diff = CW_SUB(x, y);
//
// CW_SCATTER(value)                - scatters data across memory chunks
//                                    usage: auto scattered = CW_SCATTER(myStruct);
//
// CW_POLY(value)                   - creates polymorphic value that mutates internally
//                                    usage: auto poly = CW_POLY(100);
//
// BOOLEAN OBFUSCATION
// -------------------
// CW_TRUE                          - obfuscated true using opaque predicates
//                                    usage: if (CW_TRUE) { /* always executes */ }
//
// CW_FALSE                         - obfuscated false using opaque predicates
//                                    usage: if (CW_FALSE) { /* never executes */ }
//
// CW_BOOL(expr)                    - obfuscates a boolean expression
//                                    usage: bool result = CW_BOOL(x > 0);
//
// obfuscated_bool                  - class for storing obfuscated boolean values
//                                    usage: obfuscated_bool flag(true);
//
// obfuscated_value<T>              - template class for obfuscating any value type
//                                    usage: obfuscated_value<int> val(42);
//
// mba_obfuscated<T>                - mixed boolean arithmetic obfuscation
//                                    usage: mba_obfuscated<int> val(42);
//
// CONTROL FLOW OBFUSCATION
// ------------------------
// CW_IF(condition)                 - obfuscated if statement with opaque predicates
//                                    usage: CW_IF(x > 0) { /* code */ }
//
// CW_ELSE                          - obfuscated else clause
//                                    usage: CW_IF(cond) { } CW_ELSE { }
//
// CW_BRANCH(condition)             - indirect branching with obfuscation
//                                    usage: CW_BRANCH(isValid) { /* code */ }
//
// CW_FLATTEN(func, args...)        - flattens control flow via state machine
//                                    usage: auto result = CW_FLATTEN(myFunc, arg1, arg2);
//
// FUNCTION CALL PROTECTION
// ------------------------
// CW_CALL(function)                - obfuscates function pointer and adds anti-debug
//                                    usage: auto obf_func = CW_CALL(originalFunc);
//                                           obf_func(args);
//
// obfuscated_call<Func>            - template class for function pointer obfuscation
//                                    usage: obfuscated_call<decltype(func)> obf{func};
//
// ANTI-DEBUGGING/ANALYSIS
// -----------------------
// CW_ANTI_DEBUG()                  - crashes if debugger detected (comprehensive checks)
//                                    usage: CW_ANTI_DEBUG();
//
// CW_CHECK_ANALYSIS()              - comprehensive check for debuggers/analysis tools
//                                    usage: CW_CHECK_ANALYSIS();
//
// CW_INLINE_CHECK()                - inline anti-debug check (scatter these throughout code)
//                                    usage: CW_INLINE_CHECK();
//
// anti_debug::is_debugger_present() - returns true if debugger detected (basic checks)
//                                     usage: if(anti_debug::is_debugger_present()) { }
//
// anti_debug::comprehensive_check() - advanced multi-layered debugger detection
//                                     usage: if(anti_debug::comprehensive_check()) { }
//
// anti_debug::timing_check(func)   - detects debuggers via timing analysis
//                                    usage: if(timing_check([](){}, 1000)) { }
//
// anti_debug::verify_code_integrity() - checks if code has been modified
//                                       usage: if(!verify_code_integrity(func, size)) { }
//
// COMPILE-TIME RANDOMIZATION
// --------------------------
// CW_RANDOM_CT()                   - generates compile-time random value (unique per build)
//                                    usage: constexpr auto rand = CW_RANDOM_CT();
//
// CW_RANDOM_RT()                   - generates runtime random value (unique per execution)
//                                    usage: uint64_t rand = CW_RANDOM_RT();
//
// CW_RAND_CT(min, max)             - compile-time random in range [min, max]
//                                    usage: constexpr int x = CW_RAND_CT(1, 100);
//
// CW_RAND_RT(min, max)             - runtime random in range [min, max]
//                                    usage: int x = CW_RAND_RT(1, 100);
//
// WIDE STRING ENCRYPTION
// ----------------------
// CW_WSTR(L"text")                  - encrypts wide string at compile-time
//                                    usage: const wchar_t* msg = CW_WSTR(L"secret");
//
// STRING HASHING
// --------------
// CW_HASH("text")                   - compile-time FNV-1a hash of string (case-sensitive)
//                                    usage: constexpr uint32_t h = CW_HASH("NtClose");
//
// CW_HASH_CI("text")                - compile-time case-insensitive hash (for module names)
//                                    usage: constexpr uint32_t h = CW_HASH_CI("kernel32.dll");
//
// CW_HASH_WIDE(L"text")             - compile-time hash of wide string
//                                    usage: constexpr uint32_t h = CW_HASH_WIDE(L"ntdll.dll");
//
// hash::fnv1a_runtime(str)          - runtime hash of string
//                                    usage: uint32_t h = hash::fnv1a_runtime(dynamicStr);
//
// IMPORT HIDING
// -------------
// CW_IMPORT(mod, func)              - resolve function without import table
//                                    usage: auto pFunc = CW_IMPORT("kernel32.dll", VirtualAlloc);
//
// imports::getModuleBase(hash)      - get module base by hash
//                                    usage: void* ntdll = imports::getModuleBase(CW_HASH("ntdll.dll"));
//
// imports::getProcAddress(mod, hash) - get function by hash
//                                     usage: void* func = imports::getProcAddress(mod, CW_HASH("NtClose"));
//
// DIRECT SYSCALLS
// ---------------
// CW_SYSCALL_NUMBER(func)           - get syscall number for ntdll function
//                                    usage: uint32_t num = CW_SYSCALL_NUMBER(NtClose);
//
// syscall::getSyscallNumber(hash)   - get syscall number by function hash
//                                    usage: uint32_t num = syscall::getSyscallNumber(CW_HASH("NtClose"));
//
// ANTI-VM/SANDBOX DETECTION
// -------------------------
// CW_ANTI_VM()                      - crashes if VM/sandbox detected
//                                    usage: CW_ANTI_VM();
//
// CW_CHECK_VM()                     - returns true if VM/sandbox detected
//                                    usage: if(CW_CHECK_VM()) { /* in VM */ }
//
// anti_vm::comprehensive_check()    - comprehensive VM/sandbox detection
//                                    usage: if(anti_debug::anti_vm::comprehensive_check()) { }
//
// OBFUSCATED COMPARISONS
// ----------------------
// CW_EQ(a, b)                       - obfuscated equality check (a == b)
//                                    usage: if(CW_EQ(x, 42)) { }
//
// CW_NE(a, b)                       - obfuscated not-equals (a != b)
// CW_LT(a, b)                       - obfuscated less-than (a < b)
// CW_GT(a, b)                       - obfuscated greater-than (a > b)
// CW_LE(a, b)                       - obfuscated less-or-equal (a <= b)
// CW_GE(a, b)                       - obfuscated greater-or-equal (a >= b)
//
// ENCRYPTED CONSTANTS
// -------------------
// CW_CONST(value)                   - encrypted compile-time constant
//                                    usage: int x = CW_CONST(0xDEADBEEF);
//
// constants::runtime_constant<T>    - runtime-keyed constant (unique per execution)
//                                    usage: runtime_constant<int> val(42);
//
// JUNK CODE INSERTION
// -------------------
// CW_JUNK()                         - insert junk computation
//                                    usage: CW_JUNK();
//
// CW_JUNK_FLOW()                    - insert junk with fake control flow
//                                    usage: CW_JUNK_FLOW();
//
// RETURN ADDRESS SPOOFING
// -----------------------
// CW_SPOOF_CALL(func)               - call with spoofed return address
//                                    usage: auto spoof = CW_SPOOF_CALL(myFunc);
//
// spoof::getRetGadget()             - get cached ret gadget for spoofing
//                                    usage: void* gadget = spoof::getRetGadget();
//
// INTEGRITY VERIFICATION
// ----------------------
// CW_INTEGRITY_CHECK(func, size)    - wrap function with integrity checking
//                                    usage: auto checked = CW_INTEGRITY_CHECK(myFunc, 64);
//
// CW_DETECT_HOOK(func)              - check if function is hooked
//                                    usage: if(CW_DETECT_HOOK(VirtualAlloc)) { /* hooked */ }
//
// integrity::computeHash(data, size) - compute hash of memory region
//                                     usage: uint32_t h = integrity::computeHash(ptr, 100);
//
// integrity::verifyFunctions(...)   - verify multiple functions at once
//                                    usage: if(!integrity::verifyFunctions(f1, f2)) { }
//
// =================================================================

namespace cloakwork {

    // =================================================================
    // compile-time random number generation
    // =================================================================

#if CW_ENABLE_COMPILE_TIME_RANDOM
    namespace detail {
        template<size_t N>
        consteval uint32_t fnv1a_hash(const char (&str)[N], uint32_t basis = 0x811c9dc5) {
            uint32_t hash = basis;
            for(size_t i = 0; i < N-1; ++i) {
                hash ^= static_cast<uint32_t>(str[i]);
                hash *= 0x01000193;
            }
            return hash;
        }

        // hardware random number support (intel/amd rdseed)
        inline bool try_hardware_random(uint64_t& out) {
#ifdef _WIN32
#ifdef __RDSEED__
            if (_rdseed64_step(reinterpret_cast<unsigned long long*>(&out))) {
                return true;
            }
#endif
#endif
            return false;
        }

        // runtime key derivation - combines multiple entropy sources
        // note: this doesn't provide cryptographic randomness, it just makes
        // runtime keys unique per execution to frustrate static analysis
        inline uint64_t runtime_entropy_seed() {
            uint64_t entropy = 0;

            // try hardware random first
            if (try_hardware_random(entropy)) {
                return entropy;
            }

#if CW_KERNEL_MODE
            // kernel mode entropy sources
            // rdtsc - cpu cycle counter (changes every execution)
            entropy ^= __rdtsc();

            // current process/thread from kernel (kaslr makes these different)
            entropy ^= reinterpret_cast<uint64_t>(PsGetCurrentProcess());
            entropy ^= reinterpret_cast<uint64_t>(PsGetCurrentThread());
            entropy ^= static_cast<uint64_t>(HandleToULong(PsGetCurrentProcessId())) << 32;
            entropy ^= static_cast<uint64_t>(HandleToULong(PsGetCurrentThreadId()));

            // stack address (kaslr randomizes this)
            volatile char stack_var;
            entropy ^= reinterpret_cast<uint64_t>(&stack_var);

            // high-precision kernel performance counter
            LARGE_INTEGER perf_counter;
            perf_counter = KeQueryPerformanceCounter(nullptr);
            entropy ^= static_cast<uint64_t>(perf_counter.QuadPart);

            // kernel system time
            LARGE_INTEGER system_time;
            KeQuerySystemTime(&system_time);
            entropy ^= static_cast<uint64_t>(system_time.QuadPart);

            // current interrupt time (very high resolution)
            entropy ^= static_cast<uint64_t>(KeQueryInterruptTime());

            // pool allocation address (kaslr randomized)
            void* pool_alloc = cloakwork_internal::kernel_alloc(16);
            if (pool_alloc) {
                entropy ^= reinterpret_cast<uint64_t>(pool_alloc);
                cloakwork_internal::kernel_free(pool_alloc);
            }

#elif defined(_WIN32)
            // user mode entropy sources
            // rdtsc - cpu cycle counter (changes every execution)
            entropy ^= __rdtsc();

            // process and thread ids (aslr makes these different per run)
            entropy ^= static_cast<uint64_t>(GetCurrentProcessId()) << 32;
            entropy ^= static_cast<uint64_t>(GetCurrentThreadId());

            // stack address (aslr randomizes this)
            volatile char stack_var;
            entropy ^= reinterpret_cast<uint64_t>(&stack_var);

            // module base address (aslr randomizes this)
            HMODULE module = GetModuleHandleA(nullptr);
            entropy ^= reinterpret_cast<uint64_t>(module);

            // high-precision performance counter
            LARGE_INTEGER perf_counter;
            QueryPerformanceCounter(&perf_counter);
            entropy ^= static_cast<uint64_t>(perf_counter.QuadPart);

            // system time with high precision
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            entropy ^= (static_cast<uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;

            // heap address (also aslr randomized)
            void* heap_alloc = HeapAlloc(GetProcessHeap(), 0, 16);
            if (heap_alloc) {
                entropy ^= reinterpret_cast<uint64_t>(heap_alloc);
                HeapFree(GetProcessHeap(), 0, heap_alloc);
            }
#else
            // fallback for non-windows platforms
            entropy ^= reinterpret_cast<uint64_t>(&entropy);
            entropy ^= static_cast<uint64_t>(time(nullptr));
#endif

            // mix the entropy thoroughly using knuth's multiplicative hash
            entropy ^= std::rotl(entropy, 31);
            entropy *= 0x9e3779b97f4a7c15ULL;
            entropy ^= entropy >> 27;
            entropy *= 0x94d049bb133111ebULL;
            entropy ^= entropy >> 31;

            return entropy;
        }

        // fast runtime random using xorshift64* (seeded once per thread/cpu)
        inline uint64_t runtime_entropy() {
#if CW_KERNEL_MODE
            // kernel mode: use a simple seeded state with interlocked update
            // thread_local doesn't work well in kernel drivers
            static volatile LONG64 state = 0;

            // lazy initialization
            LONG64 current = InterlockedCompareExchange64(&state, 0, 0);
            if (current == 0) {
                LONG64 seed = static_cast<LONG64>(runtime_entropy_seed());
                InterlockedCompareExchange64(&state, seed, 0);
                current = InterlockedCompareExchange64(&state, 0, 0);
                if (current == 0) current = seed;
            }

            // xorshift64* with interlocked update
            LONG64 x = current;
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;
            InterlockedExchange64(&state, x);
            return static_cast<uint64_t>(x) * 0x2545F4914F6CDD1DULL;
#else
            // user mode: use thread_local for per-thread state
            thread_local uint64_t state = runtime_entropy_seed();

            // xorshift64* algorithm - fast and good quality
            uint64_t x = state;
            x ^= x >> 12;
            x ^= x << 25;
            x ^= x >> 27;
            state = x;
            return x * 0x2545F4914F6CDD1DULL;
#endif
        }

#if CW_KERNEL_MODE
        // kernel mode: consteval with __TIME__/__DATE__ doesn't work properly
        // use a simpler constexpr approach with __COUNTER__ and __LINE__
        constexpr uint32_t compile_seed_impl(uint32_t line, uint32_t counter) {
            // lcg mixing with line and counter for per-instantiation uniqueness
            uint32_t seed = 0xDEADBEEF;
            seed ^= line * 0x01000193;
            seed ^= counter * 0x811c9dc5;
            seed *= 0x1664525;
            seed += 0x1013904223;
            return seed;
        }

        #define CW_COMPILE_SEED() (cloakwork::detail::compile_seed_impl(__LINE__, __COUNTER__))

        template<uint32_t Seed>
        struct random_generator {
            static constexpr uint32_t value() {
                return (Seed * 1664525u + 1013904223u);
            }
            static constexpr uint32_t next() {
                return random_generator<value()>::value();
            }
        };
    }

    // kernel mode: use simpler compile-time random without consteval issues
    #define CW_RANDOM_CT() (cloakwork::detail::random_generator<CW_COMPILE_SEED()>::value())
    #define CW_RAND_CT(min, max) ((min) + (CW_RANDOM_CT() % ((max) - (min) + 1)))

#else
        // user mode: full consteval support with __TIME__/__DATE__
        consteval uint32_t compile_seed() {
            // combine multiple compile-time values for entropy
            constexpr uint32_t time_hash = fnv1a_hash(__TIME__);
            constexpr uint32_t date_hash = fnv1a_hash(__DATE__);
            constexpr uint32_t file_hash = fnv1a_hash(__FILE__);
            return time_hash ^ (date_hash << 1) ^ (file_hash >> 1) ^ __LINE__;
        }

        template<uint32_t Seed>
        struct random_generator {
            static consteval uint32_t value() {
                return (Seed * 1664525u + 1013904223u) ^ __COUNTER__;
            }
            static consteval uint32_t next() {
                return random_generator<value()>::value();
            }
        };
    }

    // separate compile-time and runtime random macros
    #define CW_RANDOM_CT() (cloakwork::detail::random_generator<cloakwork::detail::compile_seed() ^ __COUNTER__>::value())
    #define CW_RAND_CT(min, max) ((min) + (CW_RANDOM_CT() % ((max) - (min) + 1)))
#endif

    #define CW_RANDOM_RT() (cloakwork::detail::runtime_entropy())
    #define CW_RAND_RT(min, max) ((min) + (CW_RANDOM_RT() % ((max) - (min) + 1)))

    // backward compatibility - defaults to compile-time
    #define CW_RANDOM() CW_RANDOM_CT()
    #define CW_RAND(min, max) CW_RAND_CT(min, max)
#else
    // fallback to simple runtime random when compile-time random is disabled
    #define CW_RANDOM_CT() (rand())
    #define CW_RAND_CT(min, max) ((min) + (rand() % ((max) - (min) + 1)))
    #define CW_RANDOM_RT() (rand())
    #define CW_RAND_RT(min, max) ((min) + (rand() % ((max) - (min) + 1)))
    #define CW_RANDOM() (rand())
    #define CW_RAND(min, max) ((min) + (rand() % ((max) - (min) + 1)))
#endif

    // =================================================================
    // compile-time string hashing
    // =================================================================

    namespace hash {
        // fnv-1a hash for compile-time string hashing
        consteval uint32_t fnv1a(const char* str, size_t len) {
            uint32_t hash = 0x811c9dc5;
            for (size_t i = 0; i < len; ++i) {
                hash ^= static_cast<uint8_t>(str[i]);
                hash *= 0x01000193;
            }
            return hash;
        }

        // consteval version for string literals
        template<size_t N>
        consteval uint32_t fnv1a(const char (&str)[N]) {
            return fnv1a(str, N - 1);
        }

        // wide string version
        consteval uint32_t fnv1a_wide(const wchar_t* str, size_t len) {
            uint32_t hash = 0x811c9dc5;
            for (size_t i = 0; i < len; ++i) {
                hash ^= static_cast<uint8_t>(str[i] & 0xFF);
                hash *= 0x01000193;
                hash ^= static_cast<uint8_t>((str[i] >> 8) & 0xFF);
                hash *= 0x01000193;
            }
            return hash;
        }

        template<size_t N>
        consteval uint32_t fnv1a_wide(const wchar_t (&str)[N]) {
            return fnv1a_wide(str, N - 1);
        }

        // runtime hash for dynamic strings
        CW_FORCEINLINE uint32_t fnv1a_runtime(const char* str) {
            uint32_t hash = 0x811c9dc5;
            while (*str) {
                hash ^= static_cast<uint8_t>(*str++);
                hash *= 0x01000193;
            }
            return hash;
        }

        CW_FORCEINLINE uint32_t fnv1a_runtime(const wchar_t* str) {
            uint32_t hash = 0x811c9dc5;
            while (*str) {
                hash ^= static_cast<uint8_t>(*str & 0xFF);
                hash *= 0x01000193;
                hash ^= static_cast<uint8_t>((*str >> 8) & 0xFF);
                hash *= 0x01000193;
                ++str;
            }
            return hash;
        }

        // case-insensitive runtime hash
        CW_FORCEINLINE uint32_t fnv1a_runtime_ci(const char* str) {
            uint32_t hash = 0x811c9dc5;
            while (*str) {
                char c = *str++;
                if (c >= 'A' && c <= 'Z') c += 32;
                hash ^= static_cast<uint8_t>(c);
                hash *= 0x01000193;
            }
            return hash;
        }

        CW_FORCEINLINE uint32_t fnv1a_runtime_ci(const wchar_t* str) {
            uint32_t hash = 0x811c9dc5;
            while (*str) {
                wchar_t c = *str++;
                if (c >= L'A' && c <= L'Z') c += 32;
                hash ^= static_cast<uint8_t>(c & 0xFF);
                hash *= 0x01000193;
                hash ^= static_cast<uint8_t>((c >> 8) & 0xFF);
                hash *= 0x01000193;
            }
            return hash;
        }

        // case-insensitive wide-to-ascii runtime hash
        // (hashes wide string as if it were ascii - for comparing against CW_HASH_CI)
        CW_FORCEINLINE uint32_t fnv1a_runtime_ci_w2a(const wchar_t* str) {
            uint32_t hash = 0x811c9dc5;
            while (*str) {
                wchar_t c = *str++;
                if (c >= L'A' && c <= L'Z') c += 32;
                // only use low byte (ascii portion) to match CW_HASH_CI behavior
                hash ^= static_cast<uint8_t>(c & 0xFF);
                hash *= 0x01000193;
            }
            return hash;
        }

        // case-insensitive compile-time hash (for module names)
        consteval uint32_t fnv1a_ci(const char* str, size_t len) {
            uint32_t hash = 0x811c9dc5;
            for (size_t i = 0; i < len; ++i) {
                char c = str[i];
                if (c >= 'A' && c <= 'Z') c += 32;
                hash ^= static_cast<uint8_t>(c);
                hash *= 0x01000193;
            }
            return hash;
        }

        template<size_t N>
        consteval uint32_t fnv1a_ci(const char (&str)[N]) {
            return fnv1a_ci(str, N - 1);
        }
    }

    // compile-time hash macro
    #define CW_HASH(s) ([]() consteval { return cloakwork::hash::fnv1a(s); }())
    #define CW_HASH_WIDE(s) ([]() consteval { return cloakwork::hash::fnv1a_wide(s); }())
    #define CW_HASH_CI(s) ([]() consteval { return cloakwork::hash::fnv1a_ci(s); }())

    // =================================================================
    // anti-debugging and anti-analysis
    // =================================================================

#if CW_ENABLE_ANTI_DEBUG
    namespace anti_debug {

        // code integrity verification - detects hooks and patches
        inline uint32_t compute_crc32(const uint8_t* data, size_t length) {
            uint32_t crc = 0xFFFFFFFF;
            for (size_t i = 0; i < length; ++i) {
                crc ^= data[i];
                for (int j = 0; j < 8; ++j) {
                    crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
                }
            }
            return ~crc;
        }

        template<typename Func>
        inline bool verify_code_integrity(Func func, size_t expected_size, uint32_t expected_hash) {
            const uint8_t* code = reinterpret_cast<const uint8_t*>(func);
            uint32_t actual_hash = compute_crc32(code, expected_size);
            return actual_hash == expected_hash;
        }

        // basic debugger detection - exposed for manual use
        CW_FORCEINLINE bool is_debugger_present() {
#if CW_KERNEL_MODE
            // kernel mode debugger detection

            // technique 1: check for kernel debugger (KdDebuggerEnabled global)
            // this is set when a kernel debugger is attached
            if (*KdDebuggerEnabled) return true;

            // technique 2: check if kernel debugger is not present flag is false
            if (!*KdDebuggerNotPresent) return true;

            // technique 3: check if current process is being debugged
            // PsIsProcessBeingDebugged is documented and available since Windows 8
            PEPROCESS current_process = PsGetCurrentProcess();
            if (current_process) {
                // use the documented API when available
                // PsIsProcessBeingDebugged checks the DebugPort field internally
                typedef BOOLEAN (*PsIsProcessBeingDebuggedFn)(PEPROCESS Process);
                static PsIsProcessBeingDebuggedFn PsIsProcessBeingDebugged = nullptr;
                static bool resolved = false;

                if (!resolved) {
                    UNICODE_STRING func_name;
                    RtlInitUnicodeString(&func_name, L"PsIsProcessBeingDebugged");
                    PsIsProcessBeingDebugged = reinterpret_cast<PsIsProcessBeingDebuggedFn>(
                        MmGetSystemRoutineAddress(&func_name));
                    resolved = true;
                }

                if (PsIsProcessBeingDebugged && PsIsProcessBeingDebugged(current_process)) {
                    return true;
                }
            }

            return false;

#elif defined(_WIN32)
            // user mode debugger detection

            // technique 1: isdebuggerpresent api
            if (::IsDebuggerPresent()) return true;

            // technique 2: peb flag check (x64 compatible)
            __try {
#ifdef _WIN64
                PPEB peb = (PPEB)__readgsqword(0x60);
                if (peb && peb->BeingDebugged) return true;
#else
                PPEB peb = (PPEB)__readfsdword(0x30);
                if (peb && peb->BeingDebugged) return true;
#endif
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                // silently handle access violations
            }
            return false;
#else
            return false;
#endif
        }

        // timing-based debugger detection with better thresholds
        template<typename Func>
        CW_FORCEINLINE bool timing_check(Func func, uint64_t threshold = 10000) {
#if CW_KERNEL_MODE
            // kernel mode timing check
            LARGE_INTEGER freq;
            LARGE_INTEGER start = KeQueryPerformanceCounter(&freq);

            uint64_t tsc_start = __rdtsc();

            func();

            LARGE_INTEGER end = KeQueryPerformanceCounter(nullptr);
            uint64_t tsc_end = __rdtsc();

            if (freq.QuadPart == 0) return false;

            uint64_t qpc_elapsed = ((end.QuadPart - start.QuadPart) * 1000000) / freq.QuadPart;
            uint64_t tsc_elapsed = tsc_end - tsc_start;

            // check if either clock shows suspicious delay
            if (qpc_elapsed > threshold || tsc_elapsed > threshold * 100) {
                return true;
            }

            // check for clock desync (kernel debugger stepping)
            if (qpc_elapsed > 0 && tsc_elapsed > 0) {
                double ratio = static_cast<double>(tsc_elapsed) / static_cast<double>(qpc_elapsed);
                if (ratio < 0.5 || ratio > 100000.0) return true;
            }

            return false;

#elif defined(_WIN32)
            // user mode timing check
            LARGE_INTEGER start, end, freq;
            QueryPerformanceFrequency(&freq);

            uint64_t tsc_start = __rdtsc();
            QueryPerformanceCounter(&start);

            func();

            QueryPerformanceCounter(&end);
            uint64_t tsc_end = __rdtsc();

            uint64_t qpc_elapsed = ((end.QuadPart - start.QuadPart) * 1000000) / freq.QuadPart;
            uint64_t tsc_elapsed = tsc_end - tsc_start;

            // check if either clock shows suspicious delay
            if (qpc_elapsed > threshold || tsc_elapsed > threshold * 100) {
                return true;
            }

            // check for clock desync (one is hooked)
            if (qpc_elapsed > 0 && tsc_elapsed > 0) {
                double ratio = static_cast<double>(tsc_elapsed) / static_cast<double>(qpc_elapsed);
                if (ratio < 0.5 || ratio > 100000.0) return true;
            }

            return false;
#else
            return false;
#endif
        }

        // breakpoint detection via int3 scan
        CW_FORCEINLINE bool has_breakpoints(void* addr, size_t size) {
            uint8_t* bytes = reinterpret_cast<uint8_t*>(addr);
            for(size_t i = 0; i < size; ++i) {
                if(bytes[i] == 0xCC) return true; // int3 instruction
            }
            return false;
        }

        // hardware breakpoint detection
        CW_FORCEINLINE bool has_hardware_breakpoints() {
#if CW_KERNEL_MODE
            // kernel mode: read debug registers directly
            // DR0-DR3 contain hardware breakpoint addresses
            // if any are non-zero, breakpoints are set
#ifdef _WIN64
            uint64_t dr0 = __readdr(0);
            uint64_t dr1 = __readdr(1);
            uint64_t dr2 = __readdr(2);
            uint64_t dr3 = __readdr(3);
            return (dr0 || dr1 || dr2 || dr3);
#else
            // x86 uses inline asm (not available in MSVC x64)
            unsigned long dr0, dr1, dr2, dr3;
            __asm {
                mov eax, dr0
                mov dr0, eax
                mov eax, dr1
                mov dr1, eax
                mov eax, dr2
                mov dr2, eax
                mov eax, dr3
                mov dr3, eax
            }
            return (dr0 || dr1 || dr2 || dr3);
#endif

#elif defined(_WIN32)
            // user mode: use GetThreadContext
            CONTEXT ctx = {};
            ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

            if(GetThreadContext(GetCurrentThread(), &ctx)) {
                return (ctx.Dr0 || ctx.Dr1 || ctx.Dr2 || ctx.Dr3);
            }
            return false;
#else
            return false;
#endif
        }

        // modern anti-debug techniques
        namespace advanced {

            // detect common anti-anti-debug plugins (scyllahide, titanhide, etc.)
            CW_FORCEINLINE bool detect_hiding_tools() {
#if CW_KERNEL_MODE
                // kernel mode: these are user-mode specific checks
                // in kernel, we rely on kernel debugger detection instead
                return false;
#elif defined(_WIN32)
                __try {
                    // check for scyllahide dlls
                    if (GetModuleHandleA("scylla_hide.dll")) return true;
                    if (GetModuleHandleA("ScyllaHideX64.dll")) return true;
                    if (GetModuleHandleA("ScyllaHideX86.dll")) return true;

                    // check for titanhide
                    if (GetModuleHandleA("TitanHide.dll")) return true;

                    // check for hyperhide
                    if (GetModuleHandleA("HyperHide.dll")) return true;

                    // check for known debugger window classes
                    const char* debugger_windows[] = {
                        "OLLYDBG",
                        "WinDbgFrameClass",
                        "ID",  // immunity debugger
                        "Zeta Debugger",
                        "Rock Debugger",
                        "ObsidianGUI",
                        nullptr
                    };

                    for (int i = 0; debugger_windows[i] != nullptr; i++) {
                        if (FindWindowA(debugger_windows[i], nullptr)) return true;
                    }

                    // check for x64dbg/x32dbg by window title
                    if (FindWindowA(nullptr, "x64dbg")) return true;
                    if (FindWindowA(nullptr, "x32dbg")) return true;
                    if (FindWindowA(nullptr, "x96dbg")) return true;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect kernel debugger
            CW_FORCEINLINE bool kernel_debugger_present() {
#if CW_KERNEL_MODE
                // kernel mode: direct access to kernel debugger flags
                if (*KdDebuggerEnabled) return true;
                if (!*KdDebuggerNotPresent) return true;

                // check SystemKernelDebuggerInformation via ZwQuerySystemInformation
                struct {
                    BOOLEAN KernelDebuggerEnabled;
                    BOOLEAN KernelDebuggerNotPresent;
                } kernel_debug_info = {};

                ULONG return_length = 0;
                // SYSTEM_INFORMATION_CLASS is not always defined, use raw value
                NTSTATUS status = ZwQuerySystemInformation(
                    23,  // SystemKernelDebuggerInformation
                    &kernel_debug_info,
                    sizeof(kernel_debug_info),
                    &return_length);

                if (NT_SUCCESS(status)) {
                    if (kernel_debug_info.KernelDebuggerEnabled ||
                        !kernel_debug_info.KernelDebuggerNotPresent) {
                        return true;
                    }
                }

                return false;

#elif defined(_WIN32)
                __try {
                    typedef NTSTATUS (NTAPI* pNtQuerySystemInformation)(
                        ULONG SystemInformationClass,
                        PVOID SystemInformation,
                        ULONG SystemInformationLength,
                        PULONG ReturnLength
                    );

                    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
                    if (!ntdll) return false;

                    auto NtQuerySystemInformation =
                        reinterpret_cast<pNtQuerySystemInformation>(
                            GetProcAddress(ntdll, "NtQuerySystemInformation"));

                    if (NtQuerySystemInformation) {
                        ULONG kernel_debug = 0;
                        // SystemKernelDebuggerInformation = 0x23
                        NTSTATUS status = NtQuerySystemInformation(0x23, &kernel_debug, sizeof(kernel_debug), nullptr);
                        if (status == 0 && kernel_debug != 0) return true;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // check if parent process is a debugger
            CW_FORCEINLINE bool suspicious_parent_process() {
#if CW_KERNEL_MODE
                // kernel mode: check parent process of current process
                PEPROCESS current = PsGetCurrentProcess();
                if (!current) return false;

                // get parent process id - offset varies by windows version
                // for simplicity, we skip this check in kernel mode
                // (kernel drivers don't typically have meaningful parent processes)
                return false;

#elif defined(_WIN32)
                __try {
                    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                    if (snapshot == INVALID_HANDLE_VALUE) return false;

                    PROCESSENTRY32W pe;
                    pe.dwSize = sizeof(PROCESSENTRY32W);
                    DWORD current_pid = GetCurrentProcessId();
                    DWORD parent_pid = 0;

                    // find our parent pid
                    if (Process32FirstW(snapshot, &pe)) {
                        do {
                            if (pe.th32ProcessID == current_pid) {
                                parent_pid = pe.th32ParentProcessID;
                                break;
                            }
                        } while (Process32NextW(snapshot, &pe));
                    }

                    // find parent process name
                    if (parent_pid) {
                        pe.dwSize = sizeof(PROCESSENTRY32W);
                        if (Process32FirstW(snapshot, &pe)) {
                            do {
                                if (pe.th32ProcessID == parent_pid) {
                                    // check against known debuggers
                                    const wchar_t* suspicious_parents[] = {
                                        L"x64dbg.exe", L"x32dbg.exe", L"ollydbg.exe",
                                        L"ida.exe", L"ida64.exe", L"windbg.exe",
                                        L"immunitydebugger.exe", L"cheatengine-x86_64.exe",
                                        L"cheatengine-i386.exe", L"processhacker.exe",
                                        nullptr
                                    };

                                    wchar_t lower_name[MAX_PATH];
                                    wcscpy_s(lower_name, MAX_PATH, pe.szExeFile);
                                    _wcslwr_s(lower_name, MAX_PATH);

                                    for (int i = 0; suspicious_parents[i] != nullptr; i++) {
                                        if (wcsstr(lower_name, suspicious_parents[i])) {
                                            CloseHandle(snapshot);
                                            return true;
                                        }
                                    }
                                    break;
                                }
                            } while (Process32NextW(snapshot, &pe));
                        }
                    }

                    CloseHandle(snapshot);
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect memory breakpoints (page guard)
            CW_FORCEINLINE bool detect_memory_breakpoints(void* address, size_t size) {
#if CW_KERNEL_MODE
                // kernel mode: check if memory is nonpaged and readable
                // page guard detection is less relevant in kernel mode
                // but we can check for suspicious memory states
                if (!MmIsAddressValid(address)) return false;
                return false;  // simplified for kernel

#elif defined(_WIN32)
                MEMORY_BASIC_INFORMATION mbi;
                uint8_t* ptr = static_cast<uint8_t*>(address);
                size_t remaining = size;

                while (remaining > 0) {
                    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0) break;

                    // check for page guard (used for memory breakpoints)
                    if (mbi.Protect & PAGE_GUARD) return true;

                    size_t block_size = mbi.RegionSize - (ptr - static_cast<uint8_t*>(mbi.BaseAddress));
                    if (block_size > remaining) block_size = remaining;

                    ptr += block_size;
                    remaining -= block_size;
                }
#endif
                return false;
            }

            // check for debugger-specific registry keys
            CW_FORCEINLINE bool detect_debugger_artifacts() {
#if CW_KERNEL_MODE
                // kernel mode: could use ZwOpenKey but registry access
                // from kernel is more complex. skip for now.
                return false;

#elif defined(_WIN32)
                __try {
                    // check for common debugger installation paths in registry
                    HKEY key;
                    const char* debugger_keys[] = {
                        "SOFTWARE\\x64dbg",
                        "SOFTWARE\\OllyDbg",
                        "SOFTWARE\\Immunity Inc\\Immunity Debugger",
                        nullptr
                    };

                    for (int i = 0; debugger_keys[i] != nullptr; i++) {
                        if (RegOpenKeyExA(HKEY_CURRENT_USER, debugger_keys[i], 0, KEY_READ, &key) == ERROR_SUCCESS) {
                            RegCloseKey(key);
                            return true;
                        }
                        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, debugger_keys[i], 0, KEY_READ, &key) == ERROR_SUCCESS) {
                            RegCloseKey(key);
                            return true;
                        }
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // advanced timing check (compares rdtsc vs qpc for hook detection)
            CW_FORCEINLINE bool advanced_timing_check() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    // compare rdtsc vs queryperformancecounter
                    LARGE_INTEGER freq, qpc_start, qpc_end;
                    if (!QueryPerformanceFrequency(&freq) || freq.QuadPart == 0) {
                        return false;
                    }

                    uint64_t tsc_start = __rdtsc();
                    QueryPerformanceCounter(&qpc_start);

                    // small operation
                    volatile int dummy = 0;
                    for (int i = 0; i < 100; i++) {
                        dummy += i;
                        CW_COMPILER_BARRIER();
                    }

                    QueryPerformanceCounter(&qpc_end);
                    uint64_t tsc_end = __rdtsc();

                    // calculate elapsed time using both methods
                    uint64_t tsc_delta = tsc_end - tsc_start;
                    uint64_t qpc_delta_us = ((qpc_end.QuadPart - qpc_start.QuadPart) * 1000000) / freq.QuadPart;

                    // if operation took suspiciously long (debugger step-through)
                    if (tsc_delta > 1000000) return true;

                    // check for inconsistent timing (one clock source is hooked)
                    if (qpc_delta_us > 0) {
                        double ratio = static_cast<double>(tsc_delta) / static_cast<double>(qpc_delta_us);
                        if (ratio < 0.5 || ratio > 100000.0) return true;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }
        }

        // comprehensive check combining all techniques
        CW_FORCEINLINE bool comprehensive_check() {
            __try {
                // basic checks
                if (is_debugger_present()) return true;
                if (has_hardware_breakpoints()) return true;

                // timing check with dummy operation
                bool timing_suspicious = timing_check([]() {
                    volatile int dummy = 0;
                    for (int i = 0; i < 100; i++) {
                        dummy += i;
                        CW_COMPILER_BARRIER();
                    }
                }, 50000);

                if (timing_suspicious) return true;

                // advanced checks - wrapped individually for safety
                __try {
                    if (advanced::detect_hiding_tools()) return true;
                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                __try {
                    if (advanced::kernel_debugger_present()) return true;
                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                __try {
                    if (advanced::suspicious_parent_process()) return true;
                } __except (EXCEPTION_EXECUTE_HANDLER) {}

                return false;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }

        // inline anti-debug check - scatter these throughout your code
        CW_FORCEINLINE void inline_check() {
#if CW_ANTI_DEBUG_RESPONSE == 1
            if (is_debugger_present() || has_hardware_breakpoints()) {
                __debugbreak();
                *(volatile int*)0 = 0;
            }
#elif CW_ANTI_DEBUG_RESPONSE == 2
            // fake response - pretend to work but return garbage
            // implementation depends on context
#endif
        }

#if CW_ENABLE_ANTI_VM
        // anti-VM and sandbox detection
        namespace anti_vm {

            // detect hypervisor via cpuid
            CW_FORCEINLINE bool is_hypervisor_present() {
#if defined(_WIN32)
                int cpuInfo[4];
                __cpuid(cpuInfo, 1);
                return (cpuInfo[2] >> 31) & 1;  // hypervisor bit
#else
                return false;
#endif
            }

            // detect VM vendor string
            CW_FORCEINLINE bool detect_vm_vendor() {
#if defined(_WIN32)
                __try {
                    int cpuInfo[4];
                    __cpuid(cpuInfo, 0x40000000);

                    char vendor[13];
                    memcpy(vendor, &cpuInfo[1], 4);
                    memcpy(vendor + 4, &cpuInfo[2], 4);
                    memcpy(vendor + 8, &cpuInfo[3], 4);
                    vendor[12] = 0;

                    const char* vmVendors[] = {
                        "VMwareVMware", "Microsoft Hv", "VBoxVBoxVBox",
                        "KVMKVMKVM\0\0\0", "XenVMMXenVMM", "prl hyperv  ",
                        "TCGTCGTCGTCG", nullptr
                    };

                    for (int i = 0; vmVendors[i]; ++i) {
                        if (memcmp(vendor, vmVendors[i], 12) == 0) return true;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // check for low resource counts (common in sandboxes)
            CW_FORCEINLINE bool detect_low_resources() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    SYSTEM_INFO si;
                    GetSystemInfo(&si);
                    if (si.dwNumberOfProcessors < 2) return true;

                    MEMORYSTATUSEX ms{sizeof(ms)};
                    GlobalMemoryStatusEx(&ms);
                    if (ms.ullTotalPhys < 2ULL * 1024 * 1024 * 1024) return true;

                    // check disk size
                    ULARGE_INTEGER freeBytesAvailable, totalBytes, freeBytes;
                    if (GetDiskFreeSpaceExA("C:\\", &freeBytesAvailable, &totalBytes, &freeBytes)) {
                        if (totalBytes.QuadPart < 60ULL * 1024 * 1024 * 1024) return true;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect common sandbox DLLs
            CW_FORCEINLINE bool detect_sandbox_dlls() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    const char* sandboxDlls[] = {
                        "SbieDll.dll",      // sandboxie
                        "dbghelp.dll",      // often loaded by analysis tools
                        "api_log.dll",      // api logging
                        "dir_watch.dll",    // directory watching
                        "pstorec.dll",      // password store (sometimes in sandboxes)
                        "vmcheck.dll",      // vm check library
                        "wpespy.dll",       // wpe pro (packet editor)
                        "cmdvrt32.dll",     // comodo sandbox
                        "cmdvrt64.dll",     // comodo sandbox
                        "cuckoomon.dll",    // cuckoo sandbox
                        nullptr
                    };

                    for (int i = 0; sandboxDlls[i]; ++i) {
                        if (GetModuleHandleA(sandboxDlls[i])) return true;
                    }

                    // check for analysis tool windows
                    const char* analysisWindows[] = {
                        "PROCMON_WINDOW_CLASS",
                        "FilemonClass",
                        "RegmonClass",
                        "Autoruns",
                        nullptr
                    };

                    for (int i = 0; analysisWindows[i]; ++i) {
                        if (FindWindowA(analysisWindows[i], nullptr)) return true;
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect suspicious usernames/computer names
            CW_FORCEINLINE bool detect_sandbox_names() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    char buffer[256];
                    DWORD size = sizeof(buffer);

                    // check username
                    if (GetUserNameA(buffer, &size)) {
                        const char* suspiciousUsers[] = {
                            "sandbox", "virus", "malware", "sample", "test",
                            "user", "admin", "currentuser", "vmware", "vbox",
                            nullptr
                        };

                        for (int i = 0; suspiciousUsers[i]; ++i) {
                            if (strstr(buffer, suspiciousUsers[i])) return true;
                        }
                    }

                    // check computer name
                    size = sizeof(buffer);
                    if (GetComputerNameA(buffer, &size)) {
                        const char* suspiciousComputers[] = {
                            "sandbox", "test", "virus", "malware", "sample",
                            nullptr
                        };

                        for (int i = 0; suspiciousComputers[i]; ++i) {
                            if (strstr(buffer, suspiciousComputers[i])) return true;
                        }
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect VM-specific registry keys
            CW_FORCEINLINE bool detect_vm_registry() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    HKEY key;
                    const char* vmKeys[] = {
                        "SOFTWARE\\VMware, Inc.\\VMware Tools",
                        "SOFTWARE\\Oracle\\VirtualBox Guest Additions",
                        "SYSTEM\\CurrentControlSet\\Services\\VBoxGuest",
                        "SYSTEM\\CurrentControlSet\\Services\\vmci",
                        "SYSTEM\\CurrentControlSet\\Services\\vmhgfs",
                        nullptr
                    };

                    for (int i = 0; vmKeys[i]; ++i) {
                        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, vmKeys[i], 0, KEY_READ, &key) == ERROR_SUCCESS) {
                            RegCloseKey(key);
                            return true;
                        }
                    }
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // detect VM-specific MAC address prefixes
            CW_FORCEINLINE bool detect_vm_mac() {
#if defined(_WIN32) && !CW_KERNEL_MODE
                __try {
                    // common VM MAC prefixes (first 3 bytes)
                    const uint8_t vmMacPrefixes[][3] = {
                        {0x00, 0x0C, 0x29},  // vmware
                        {0x00, 0x50, 0x56},  // vmware
                        {0x08, 0x00, 0x27},  // virtualbox
                        {0x00, 0x1C, 0x42},  // parallels
                        {0x00, 0x03, 0xFF},  // hyper-v
                        {0x00, 0x15, 0x5D},  // hyper-v
                    };

                    // get adapter info - use raw allocation to avoid SEH/RAII conflict
                    ULONG bufferSize = 0;
                    GetAdaptersInfo(nullptr, &bufferSize);
                    if (bufferSize == 0) return false;

                    uint8_t* adapters = static_cast<uint8_t*>(HeapAlloc(GetProcessHeap(), 0, bufferSize));
                    if (!adapters) return false;

                    auto adapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(adapters);
                    bool found = false;

                    if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_SUCCESS) {
                        for (auto adapter = adapterInfo; adapter && !found; adapter = adapter->Next) {
                            if (adapter->AddressLength >= 3) {
                                for (int i = 0; i < 6 && !found; ++i) {
                                    if (memcmp(adapter->Address, vmMacPrefixes[i], 3) == 0) {
                                        found = true;
                                    }
                                }
                            }
                        }
                    }

                    HeapFree(GetProcessHeap(), 0, adapters);
                    return found;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
#endif
                return false;
            }

            // comprehensive VM/sandbox check
            CW_FORCEINLINE bool comprehensive_check() {
                __try {
                    if (is_hypervisor_present()) return true;
                    if (detect_vm_vendor()) return true;
                    if (detect_low_resources()) return true;
                    if (detect_sandbox_dlls()) return true;
                    if (detect_sandbox_names()) return true;
                    if (detect_vm_registry()) return true;
                    if (detect_vm_mac()) return true;
                    return false;
                }
                __except (EXCEPTION_EXECUTE_HANDLER) {
                    return false;
                }
            }
        } // namespace anti_vm
#endif // CW_ENABLE_ANTI_VM
    }

    // macro to crash if debugger detected
    #define CW_ANTI_DEBUG() \
        do { \
            if(cloakwork::anti_debug::comprehensive_check()) { \
                __debugbreak(); \
                *(volatile int*)0 = 0; \
            } \
        } while(0)

    #define CW_INLINE_CHECK() cloakwork::anti_debug::inline_check()

    // anti-VM macro
    #if CW_ENABLE_ANTI_VM
        #define CW_ANTI_VM() \
            do { \
                if(cloakwork::anti_debug::anti_vm::comprehensive_check()) { \
                    __debugbreak(); \
                    *(volatile int*)0 = 0; \
                } \
            } while(0)

        #define CW_CHECK_VM() (cloakwork::anti_debug::anti_vm::comprehensive_check())
    #else
        #define CW_ANTI_VM() ((void)0)
        #define CW_CHECK_VM() (false)
    #endif

#else
    // no-op when anti-debug is disabled
    namespace anti_debug {
        inline bool is_debugger_present() { return false; }
        template<typename Func> inline bool timing_check(Func, uint64_t = 1000) { return false; }
        inline bool has_breakpoints(void*, size_t) { return false; }
        inline bool has_hardware_breakpoints() { return false; }
        inline bool comprehensive_check() { return false; }
        inline void inline_check() {}
        template<typename Func> inline bool verify_code_integrity(Func, size_t, uint32_t) { return true; }

        namespace anti_vm {
            inline bool is_hypervisor_present() { return false; }
            inline bool detect_vm_vendor() { return false; }
            inline bool detect_low_resources() { return false; }
            inline bool detect_sandbox_dlls() { return false; }
            inline bool detect_sandbox_names() { return false; }
            inline bool detect_vm_registry() { return false; }
            inline bool detect_vm_mac() { return false; }
            inline bool comprehensive_check() { return false; }
        }
    }
    #define CW_ANTI_DEBUG() ((void)0)
    #define CW_INLINE_CHECK() ((void)0)
    #define CW_ANTI_VM() ((void)0)
    #define CW_CHECK_VM() (false)
#endif

    // =================================================================
    // compile-time string encryption
    // =================================================================

#if CW_ENABLE_STRING_ENCRYPTION
    namespace string_encrypt {

        template<size_t N, char Key1 = CW_RAND_CT(1, 127), char Key2 = CW_RAND_CT(1, 127)>
        class encrypted_string {
        private:
            std::array<char, N> data;
            mutable CW_ATOMIC(bool) decrypted{false};
#if CW_KERNEL_MODE
            mutable CW_MUTEX mutex;
#else
            mutable CW_MUTEX mutex;
#endif

            // compile-time keys (unique per build)
            static constexpr uint8_t compile_key1 = static_cast<uint8_t>(Key1);
            static constexpr uint8_t compile_key2 = static_cast<uint8_t>(Key2);

            // compile-time encryption (constexpr allows both compile-time and runtime use)
            static constexpr char encrypt_char(char c, size_t i) {
                char k1 = compile_key1 + static_cast<char>(i);
                char k2 = compile_key2 - static_cast<char>(i * 3);
                char k3 = static_cast<char>((i * i) ^ 0x5A);
                return c ^ k1 ^ k2 ^ k3;
            }

            CW_FORCEINLINE void decrypt_impl() const {
                if(!decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);

                    // double-check after acquiring lock
                    if (!decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<char, N>&>(data);

                        for(size_t i = 0; i < N; ++i) {
                            // reverse compile-time encryption (XOR is self-inverse)
                            char k1 = compile_key1 + static_cast<char>(i);
                            char k2 = compile_key2 - static_cast<char>(i * 3);
                            char k3 = static_cast<char>((i * i) ^ 0x5A);
                            mutable_data[i] ^= k1 ^ k2 ^ k3;
                        }
                        decrypted.store(true, 0);
                    }
                }
            }

            CW_FORCEINLINE void encrypt_impl() const {
                if(decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);

                    if (decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<char, N>&>(data);

                        for(size_t i = 0; i < N; ++i) {
                            // re-apply compile-time encryption (XOR is self-inverse)
                            char k1 = compile_key1 + static_cast<char>(i);
                            char k2 = compile_key2 - static_cast<char>(i * 3);
                            char k3 = static_cast<char>((i * i) ^ 0x5A);
                            mutable_data[i] ^= k1 ^ k2 ^ k3;
                        }
                        decrypted.store(false, 0);
                    }
                }
            }

        public:
            template<size_t... I>
            constexpr encrypted_string(const char (&str)[N], std::index_sequence<I...>)
                : data{encrypt_char(str[I], I)...},
                  decrypted(false) {}

            constexpr encrypted_string(const char (&str)[N])
                : encrypted_string(str, std::make_index_sequence<N>{}) {}

            CW_FORCEINLINE const char* get() const {
                decrypt_impl();
                return data.data();
            }

            CW_FORCEINLINE operator const char*() const {
                return get();
            }

            // re-encrypt on destruction
            ~encrypted_string() {
                encrypt_impl();
            }
        };

        template<size_t N>
        encrypted_string(const char (&)[N]) -> encrypted_string<N>;

        // multi-layer encrypted string with polymorphic decryption
        template<size_t N,
                 uint8_t Layer1Key = CW_RAND_CT(1, 255),
                 uint8_t Layer2Key = CW_RAND_CT(1, 255),
                 uint8_t Layer3Key = CW_RAND_CT(1, 255)>
        class layered_encrypted_string {
        private:
            std::array<char, N> data;
            mutable CW_ATOMIC(bool) decrypted{false};
            mutable CW_ATOMIC(uint32_t) access_count{0};
            mutable CW_MUTEX mutex;

            static constexpr char encrypt_multilayer(char c, size_t i) {
                // layer 1: position-dependent xor
                char temp = c ^ (Layer1Key + static_cast<char>(i));

                // layer 2: rotation + xor
                temp = static_cast<char>(std::rotl(static_cast<uint8_t>(temp), static_cast<int>((i % 7) + 1)));
                temp ^= Layer2Key;

                // layer 3: polynomial mixing
                temp ^= static_cast<char>((i * i + i) ^ Layer3Key);

                return temp;
            }

            CW_FORCEINLINE void decrypt_impl() const {
                if(!decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);

                    if (!decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<char, N>&>(data);

                        for(size_t i = 0; i < N; ++i) {
                            char temp = mutable_data[i];

                            // reverse layer 3
                            temp ^= static_cast<char>((i * i + i) ^ Layer3Key);

                            // reverse layer 2
                            temp ^= Layer2Key;
                            temp = static_cast<char>(std::rotr(static_cast<uint8_t>(temp), static_cast<int>((i % 7) + 1)));

                            // reverse layer 1
                            temp ^= (Layer1Key + static_cast<char>(i));

                            mutable_data[i] = temp;
                        }
                        decrypted.store(true, 0);
                    }
                }
            }

            CW_FORCEINLINE void encrypt_impl() const {
                if(decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);

                    if (decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<char, N>&>(data);

                        for(size_t i = 0; i < N; ++i) {
                            mutable_data[i] = encrypt_multilayer(mutable_data[i], i);
                        }
                        decrypted.store(false, 0);
                    }
                }
            }

            // polymorphic re-encryption on each access
            CW_FORCEINLINE void morph() const {
                uint32_t count = access_count.fetch_add(1, 0);
                if(count % 10 == 0 && decrypted.load(0)) {
                    encrypt_impl();
                    decrypt_impl();
                }
            }

        public:
            template<size_t... I>
            constexpr layered_encrypted_string(const char (&str)[N], std::index_sequence<I...>)
                : data{encrypt_multilayer(str[I], I)...},
                  decrypted(false) {}

            constexpr layered_encrypted_string(const char (&str)[N])
                : layered_encrypted_string(str, std::make_index_sequence<N>{}) {}

            CW_FORCEINLINE const char* get() const {
                decrypt_impl();
                morph();
                return data.data();
            }

            CW_FORCEINLINE operator const char*() const {
                return get();
            }

            ~layered_encrypted_string() {
                encrypt_impl();
            }
        };

        template<size_t N>
        layered_encrypted_string(const char (&)[N]) -> layered_encrypted_string<N>;

        // stack-based encrypted string that auto-clears on scope exit
        template<size_t N>
        class stack_encrypted_string {
        private:
            char buffer[N];

            CW_FORCEINLINE void clear_buffer() {
                for(size_t i = 0; i < N; ++i) {
                    buffer[i] = static_cast<char>(CW_RANDOM_RT() & 0xFF);
                }
                // second pass with different random
                for(size_t i = 0; i < N; ++i) {
                    buffer[i] ^= static_cast<char>(CW_RANDOM_RT() & 0xFF);
                }
            }

        public:
            stack_encrypted_string(const encrypted_string<N>& enc) {
                const char* decrypted = enc.get();
                for(size_t i = 0; i < N; ++i) {
                    buffer[i] = decrypted[i];
                }
            }

            const char* get() const { return buffer; }
            operator const char*() const { return buffer; }

            ~stack_encrypted_string() {
                clear_buffer();
            }
        };

        // wide string encryption
        template<size_t N, wchar_t Key1 = static_cast<wchar_t>(CW_RAND_CT(1, 127)), wchar_t Key2 = static_cast<wchar_t>(CW_RAND_CT(1, 127))>
        class encrypted_wstring {
        private:
            std::array<wchar_t, N> data;
            mutable CW_ATOMIC(bool) decrypted{false};
            mutable CW_MUTEX mutex;

            static constexpr uint16_t compile_key1 = static_cast<uint16_t>(Key1);
            static constexpr uint16_t compile_key2 = static_cast<uint16_t>(Key2);

            static constexpr wchar_t encrypt_char(wchar_t c, size_t i) {
                wchar_t k1 = static_cast<wchar_t>(compile_key1 + static_cast<uint16_t>(i));
                wchar_t k2 = static_cast<wchar_t>(compile_key2 - static_cast<uint16_t>(i * 3));
                wchar_t k3 = static_cast<wchar_t>((i * i) ^ 0x5A5A);
                return c ^ k1 ^ k2 ^ k3;
            }

            CW_FORCEINLINE void decrypt_impl() const {
                if(!decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);
                    if (!decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<wchar_t, N>&>(data);
                        for(size_t i = 0; i < N; ++i) {
                            wchar_t k1 = static_cast<wchar_t>(compile_key1 + static_cast<uint16_t>(i));
                            wchar_t k2 = static_cast<wchar_t>(compile_key2 - static_cast<uint16_t>(i * 3));
                            wchar_t k3 = static_cast<wchar_t>((i * i) ^ 0x5A5A);
                            mutable_data[i] ^= k1 ^ k2 ^ k3;
                        }
                        decrypted.store(true, 0);
                    }
                }
            }

            CW_FORCEINLINE void encrypt_impl() const {
                if(decrypted.load(0)) {
                    CW_LOCK_GUARD(mutex);
                    if (decrypted.load(0)) {
                        auto& mutable_data = const_cast<std::array<wchar_t, N>&>(data);
                        for(size_t i = 0; i < N; ++i) {
                            wchar_t k1 = static_cast<wchar_t>(compile_key1 + static_cast<uint16_t>(i));
                            wchar_t k2 = static_cast<wchar_t>(compile_key2 - static_cast<uint16_t>(i * 3));
                            wchar_t k3 = static_cast<wchar_t>((i * i) ^ 0x5A5A);
                            mutable_data[i] ^= k1 ^ k2 ^ k3;
                        }
                        decrypted.store(false, 0);
                    }
                }
            }

        public:
            template<size_t... I>
            constexpr encrypted_wstring(const wchar_t (&str)[N], std::index_sequence<I...>)
                : data{encrypt_char(str[I], I)...}, decrypted(false) {}

            constexpr encrypted_wstring(const wchar_t (&str)[N])
                : encrypted_wstring(str, std::make_index_sequence<N>{}) {}

            CW_FORCEINLINE const wchar_t* get() const {
                decrypt_impl();
                return data.data();
            }

            CW_FORCEINLINE operator const wchar_t*() const { return get(); }

            ~encrypted_wstring() { encrypt_impl(); }
        };

        template<size_t N>
        encrypted_wstring(const wchar_t (&)[N]) -> encrypted_wstring<N>;
    }

    // macro for easy string encryption with immediate re-encryption
#define CW_STR(s) \
    static_cast<const char*>(([]() -> const char* { \
        static cloakwork::string_encrypt::encrypted_string<sizeof(s)> enc(s); \
        int dummy = static_cast<int>(CW_RANDOM_RT() & 1); \
        CW_COMPILER_BARRIER(); \
        const char* result = dummy >= 0 ? enc.get() : nullptr; \
        return result; \
    }()))

// layered encryption macro
#define CW_STR_LAYERED(s) \
    static_cast<const char*>(([]() -> const char* { \
        static cloakwork::string_encrypt::layered_encrypted_string<sizeof(s)> enc(s); \
        int dummy = static_cast<int>(CW_RANDOM_RT() & 1); \
        CW_COMPILER_BARRIER(); \
        return dummy >= 0 ? enc.get() : nullptr; \
    }()))

// stack-based encrypted string with tight scope control
#define CW_STR_STACK(s) \
    ([&]() { \
        static cloakwork::string_encrypt::encrypted_string<sizeof(s)> enc(s); \
        return cloakwork::string_encrypt::stack_encrypted_string<sizeof(s)>(enc); \
    }())

// wide string encryption macro
#define CW_WSTR(s) \
    static_cast<const wchar_t*>(([]() -> const wchar_t* { \
        static cloakwork::string_encrypt::encrypted_wstring<sizeof(s)/sizeof(wchar_t)> enc(s); \
        int dummy = static_cast<int>(CW_RANDOM_RT() & 1); \
        CW_COMPILER_BARRIER(); \
        return dummy >= 0 ? enc.get() : nullptr; \
    }()))

#else
    // no-op when string encryption is disabled
    #define CW_STR(s) (s)
    #define CW_STR_LAYERED(s) (s)
    #define CW_STR_STACK(s) (s)
    #define CW_WSTR(s) (s)
#endif

    // =================================================================
    // integer and data obfuscation with MBA (mixed boolean arithmetic)
    // =================================================================

#if CW_ENABLE_VALUE_OBFUSCATION

#if CW_KERNEL_MODE
    // kernel mode: use enable_if instead of concepts
    namespace detail {
        template<typename T>
        struct is_integral_type : std::is_integral<T> {};

        template<typename T>
        struct is_arithmetic_type : std::is_arithmetic<T> {};
    }

    // mixed boolean arithmetic obfuscation
    namespace mba {
        // MBA identity: x + y = (x ^ y) + 2 * (x & y)
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T add_mba(T x, T y) {
            return (x ^ y) + ((x & y) << 1);
        }

        // MBA identity: x - y = (x ^ y) - 2 * (~x & y)
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T sub_mba(T x, T y) {
            return (x ^ y) - ((~x & y) << 1);
        }

        // MBA identity: x * 2 = (x ^ (x << 1)) + (x << 1)
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T mul2_mba(T x) {
            return (x ^ (x << 1)) + (x << 1);
        }

        // MBA identity: -x = ~x + 1
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T neg_mba(T x) {
            return add_mba(static_cast<T>(~x), static_cast<T>(1));
        }

        // MBA identity: x & y = ~(~x | ~y)
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T and_mba(T x, T y) {
            return ~(~x | ~y);
        }

        // MBA identity: x | y = ~(~x & ~y)
        template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
        CW_FORCEINLINE constexpr T or_mba(T x, T y) {
            return ~(~x & ~y);
        }
    }

    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    class obfuscated_value {
    private:
        mutable T value{};
        T xor_key{};
        T add_key{};
        mutable CW_ATOMIC(uint32_t) access_count{0};

        // rotate bits for additional obfuscation
        template<typename U = T, typename = std::enable_if_t<std::is_integral_v<U>>>
        static constexpr U rotate_left(U val, int shift) {
            constexpr int bits = sizeof(U) * 8;
            shift %= bits;
            return (val << shift) | (val >> (bits - shift));
        }

        template<typename U = T, typename = std::enable_if_t<std::is_integral_v<U>>>
        static constexpr U rotate_right(U val, int shift) {
            constexpr int bits = sizeof(U) * 8;
            shift %= bits;
            return (val >> shift) | (val << (bits - shift));
        }

    public:
        obfuscated_value() {
            xor_key = static_cast<T>(CW_RANDOM_RT());
            add_key = static_cast<T>(CW_RANDOM_RT());
            set(static_cast<T>(0));
        }

        obfuscated_value(T val) {
            xor_key = static_cast<T>(CW_RANDOM_RT());
            add_key = static_cast<T>(CW_RANDOM_RT());
            set(val);
        }

        CW_FORCEINLINE void set(T val) {
            if constexpr(std::is_integral_v<T>) {
                // multi-step obfuscation for integers using MBA + XOR
                T temp = mba::add_mba(val, add_key);
                value = temp ^ xor_key;
            } else if constexpr(sizeof(T) == sizeof(uint64_t)) {
                // for floating point, use reinterpret
                uint64_t bits;
                memcpy(&bits, &val, sizeof(val));
                uint64_t key_bits;
                memcpy(&key_bits, &xor_key, sizeof(xor_key));
                bits ^= key_bits;
                memcpy(&value, &bits, sizeof(value));
            } else if constexpr(sizeof(T) == sizeof(uint32_t)) {

#else
    // user mode: use C++20 concepts

    // concepts for type constraints
    template<typename T>
    concept Integral = std::is_integral_v<T>;

    template<typename T>
    concept Arithmetic = std::is_arithmetic_v<T>;

    // mixed boolean arithmetic obfuscation
    namespace mba {
        // MBA identity: x + y = (x ^ y) + 2 * (x & y)
        template<Integral T>
        CW_FORCEINLINE constexpr T add_mba(T x, T y) {
            return (x ^ y) + ((x & y) << 1);
        }

        // MBA identity: x - y = (x ^ y) - 2 * (~x & y)
        template<Integral T>
        CW_FORCEINLINE constexpr T sub_mba(T x, T y) {
            return (x ^ y) - ((~x & y) << 1);
        }

        // MBA identity: x * 2 = (x ^ (x << 1)) + (x << 1)
        template<Integral T>
        CW_FORCEINLINE constexpr T mul2_mba(T x) {
            return (x ^ (x << 1)) + (x << 1);
        }

        // MBA identity: -x = ~x + 1
        template<Integral T>
        CW_FORCEINLINE constexpr T neg_mba(T x) {
            return add_mba(static_cast<T>(~x), static_cast<T>(1));
        }

        // MBA identity: x & y = ~(~x | ~y)
        template<Integral T>
        CW_FORCEINLINE constexpr T and_mba(T x, T y) {
            return ~(~x | ~y);
        }

        // MBA identity: x | y = ~(~x & ~y)
        template<Integral T>
        CW_FORCEINLINE constexpr T or_mba(T x, T y) {
            return ~(~x & ~y);
        }
    }

    template<Arithmetic T>
    class obfuscated_value {
    private:
        mutable T value{};
        T xor_key{};
        T add_key{};
        mutable CW_ATOMIC(uint32_t) access_count{0};

        // rotate bits for additional obfuscation
        template<Integral U = T>
        static constexpr U rotate_left(U val, int shift) {
            constexpr int bits = sizeof(U) * 8;
            shift %= bits;
            return (val << shift) | (val >> (bits - shift));
        }

        template<Integral U = T>
        static constexpr U rotate_right(U val, int shift) {
            constexpr int bits = sizeof(U) * 8;
            shift %= bits;
            return (val >> shift) | (val << (bits - shift));
        }

    public:
        obfuscated_value() {
            xor_key = static_cast<T>(CW_RANDOM_RT());
            add_key = static_cast<T>(CW_RANDOM_RT());
            set(static_cast<T>(0));
        }

        obfuscated_value(T val) {
            xor_key = static_cast<T>(CW_RANDOM_RT());
            add_key = static_cast<T>(CW_RANDOM_RT());
            set(val);
        }

        CW_FORCEINLINE void set(T val) {
            if constexpr(Integral<T>) {
                // multi-step obfuscation for integers using MBA + XOR
                T temp = mba::add_mba(val, add_key);
                value = temp ^ xor_key;
            } else if constexpr(sizeof(T) == sizeof(uint64_t)) {
                uint64_t bits = std::bit_cast<uint64_t>(val);
                uint64_t key_bits = std::bit_cast<uint64_t>(xor_key);
                bits ^= key_bits;
                value = std::bit_cast<T>(bits);
            } else if constexpr(sizeof(T) == sizeof(uint32_t)) {
#endif
                uint32_t bits = std::bit_cast<uint32_t>(val);
                uint32_t key_bits = std::bit_cast<uint32_t>(xor_key);
                bits ^= key_bits;
                value = std::bit_cast<T>(bits);
            } else {
                value = val;
            }
        }

        CW_FORCEINLINE T get() const {
            // inline anti-debug check every N accesses
            if ((++access_count % 1000) == 0) {
                CW_INLINE_CHECK();
            }

            if constexpr(Integral<T>) {
                T temp = value ^ xor_key;
                return mba::sub_mba(temp, add_key);
            } else if constexpr(sizeof(T) == sizeof(uint64_t)) {
                uint64_t bits = std::bit_cast<uint64_t>(value);
                uint64_t key_bits = std::bit_cast<uint64_t>(xor_key);
                bits ^= key_bits;
                return std::bit_cast<T>(bits);
            } else if constexpr(sizeof(T) == sizeof(uint32_t)) {
                uint32_t bits = std::bit_cast<uint32_t>(value);
                uint32_t key_bits = std::bit_cast<uint32_t>(xor_key);
                bits ^= key_bits;
                return std::bit_cast<T>(bits);
            } else {
                return value;
            }
        }

        CW_FORCEINLINE operator T() const { return get(); }
        CW_FORCEINLINE obfuscated_value& operator=(T val) { set(val); return *this; }
    };

    // enhanced obfuscation using only MBA transformations
    template<Integral T>
    class mba_obfuscated {
    private:
        T encoded{};
        T key1{};
        T key2{};

    public:
        mba_obfuscated() {
            key1 = static_cast<T>(CW_RANDOM_RT());
            key2 = static_cast<T>(CW_RANDOM_RT());
            set(static_cast<T>(0));
        }

        mba_obfuscated(T val) {
            key1 = static_cast<T>(CW_RANDOM_RT());
            key2 = static_cast<T>(CW_RANDOM_RT());
            set(val);
        }

        CW_FORCEINLINE void set(T val) {
            // encode using MBA: encoded = (val + key1) ^ key2
            T temp = mba::add_mba(val, key1);
            encoded = temp ^ key2;
        }

        CW_FORCEINLINE T get() const {
            // decode using MBA: val = (encoded ^ key2) - key1
            T temp = encoded ^ key2;
            return mba::sub_mba(temp, key1);
        }

        CW_FORCEINLINE operator T() const { return get(); }
        CW_FORCEINLINE mba_obfuscated& operator=(T val) { set(val); return *this; }
    };

    // =================================================================
    // boolean obfuscation
    // =================================================================

    namespace bool_obfuscation {

        // obfuscated true using multiple opaque predicates and runtime values
        template<int N = CW_RAND_CT(1, 1000)>
        CW_FORCEINLINE bool obfuscated_true() {
            // runtime value prevents constant folding
            volatile int runtime_val = static_cast<int>(reinterpret_cast<uintptr_t>(&runtime_val) & 0xFF);
            int x = N + runtime_val;
            CW_COMPILER_BARRIER();

            // multiple always-true predicates combined
            // property: product of consecutive integers is always even
            bool result = ((x * (x + 1)) % 2) == 0;

            // property: x OR x is always x
            result = result && ((x | x) == x);

            // property: x XOR 0 is always x
            result = result && ((x ^ 0) == x);

            // property: x AND x is always x
            result = result && ((x & x) == x);

            // MBA identity: ~(~x) == x
            result = result && ((~(~x)) == x);

            CW_COMPILER_BARRIER();
            return result;
        }

        // obfuscated false using multiple opaque predicates
        template<int N = CW_RAND_CT(1, 1000)>
        CW_FORCEINLINE bool obfuscated_false() {
            volatile int runtime_val = static_cast<int>(reinterpret_cast<uintptr_t>(&runtime_val) & 0xFF);
            int x = N + runtime_val;
            CW_COMPILER_BARRIER();

            // multiple always-false predicates combined
            // property: x AND (NOT x) is always 0
            bool result = ((x & (~x)) != 0);

            // property: x XOR x is always 0
            result = result || ((x ^ x) != 0);

            // property: impossible condition
            result = result || (x * x < 0);

            // MBA identity: (x - x) is always 0
            result = result || (mba::sub_mba(x, x) != 0);

            CW_COMPILER_BARRIER();
            return result;
        }

        // obfuscate a boolean expression through indirection
        template<int N = CW_RAND_CT(1, 1000)>
        CW_FORCEINLINE bool obfuscate_bool(bool value) {
            CW_COMPILER_BARRIER();

            // transform: value = (value AND true) OR (false AND anything)
            // mathematically equivalent to just 'value', but harder to analyze
            bool true_val = obfuscated_true<N>();
            bool false_val = obfuscated_false<N + 1>();

            // multiple transformation layers
            bool layer1 = value && true_val;
            bool layer2 = false_val && (!value);
            bool result = layer1 || layer2;

            // additional confusion: XOR with known values
            result = result ^ false_val;  // XOR with false doesn't change value

            CW_COMPILER_BARRIER();
            return result;
        }

        // class for storing obfuscated boolean values with anti-pattern storage
        template<uint8_t Key1 = static_cast<uint8_t>(CW_RAND_CT(1, 255)),
                 uint8_t Key2 = static_cast<uint8_t>(CW_RAND_CT(1, 255)),
                 uint8_t Key3 = static_cast<uint8_t>(CW_RAND_CT(1, 255))>
        class obfuscated_bool {
        private:
            mutable uint8_t encoded_primary;
            mutable uint8_t encoded_secondary;
            mutable uint8_t encoded_tertiary;
            mutable CW_ATOMIC(uint32_t) access_count{0};

            // distinct patterns for true/false that don't look like 0/1
            static constexpr uint8_t TRUE_PATTERN = Key1 ^ 0xAA ^ Key2;
            static constexpr uint8_t FALSE_PATTERN = Key1 ^ 0x55 ^ Key3;
            static constexpr uint8_t VERIFY_MASK = Key2 ^ Key3;

            CW_FORCEINLINE void encode(bool value) {
                uint8_t runtime_noise = static_cast<uint8_t>(CW_RANDOM_RT() & 0xF0);

                if (value) {
                    // encode true across multiple bytes with different patterns
                    encoded_primary = TRUE_PATTERN ^ runtime_noise;
                    encoded_secondary = static_cast<uint8_t>(~encoded_primary) ^ Key1;
                    encoded_tertiary = (encoded_primary + encoded_secondary) ^ VERIFY_MASK;
                } else {
                    // encode false with different patterns
                    encoded_primary = FALSE_PATTERN ^ runtime_noise;
                    encoded_secondary = static_cast<uint8_t>(~encoded_primary) ^ Key2;
                    encoded_tertiary = (encoded_primary - encoded_secondary) ^ VERIFY_MASK;
                }
            }

            CW_FORCEINLINE bool decode() const {
                // decode using pattern matching, not simple comparison
                uint8_t reconstructed = encoded_primary ^ (static_cast<uint8_t>(~encoded_primary) ^ Key1);
                uint8_t check = encoded_secondary ^ Key1;

                // verify integrity through tertiary byte
                uint8_t expected_true = (encoded_primary + (static_cast<uint8_t>(~encoded_primary) ^ Key1)) ^ VERIFY_MASK;
                bool is_true_pattern = (encoded_tertiary == expected_true);

                // use MBA to compute final result
                int true_indicator = is_true_pattern ? 1 : 0;
                int one = mba::sub_mba(2, 1);
                return mba::sub_mba(true_indicator, 0) == one;
            }

        public:
            obfuscated_bool() { encode(false); }
            obfuscated_bool(bool value) { encode(value); }

            CW_FORCEINLINE bool get() const {
                // periodic anti-debug check
                if ((++access_count % 500) == 0) {
                    CW_INLINE_CHECK();
                }

                bool raw_value = decode();
                // return through obfuscation layer
                return obfuscate_bool(raw_value);
            }

            CW_FORCEINLINE void set(bool value) {
                encode(value);
            }

            CW_FORCEINLINE operator bool() const { return get(); }
            CW_FORCEINLINE obfuscated_bool& operator=(bool value) { set(value); return *this; }

            // logical operators
            CW_FORCEINLINE obfuscated_bool operator!() const {
                return obfuscated_bool(!get());
            }

            CW_FORCEINLINE obfuscated_bool operator&&(bool other) const {
                return obfuscated_bool(get() && other);
            }

            CW_FORCEINLINE obfuscated_bool operator||(bool other) const {
                return obfuscated_bool(get() || other);
            }
        };
    }

    // convenience macros for boolean obfuscation
    #define CW_TRUE (cloakwork::bool_obfuscation::obfuscated_true<CW_RAND_CT(1, 1000)>())
    #define CW_FALSE (cloakwork::bool_obfuscation::obfuscated_false<CW_RAND_CT(1, 1000)>())
    #define CW_BOOL(x) (cloakwork::bool_obfuscation::obfuscate_bool<CW_RAND_CT(1, 1000)>(x))

    // convenience macros for MBA operations
    #define CW_ADD(a, b) (cloakwork::mba::add_mba((a), (b)))
    #define CW_SUB(a, b) (cloakwork::mba::sub_mba((a), (b)))
    #define CW_AND(a, b) (cloakwork::mba::and_mba((a), (b)))
    #define CW_OR(a, b) (cloakwork::mba::or_mba((a), (b)))

#else
    template<typename T>
    class obfuscated_value {
    private:
        T value{};
    public:
        obfuscated_value() = default;
        obfuscated_value(T val) : value(val) {}
        CW_FORCEINLINE void set(T val) { value = val; }
        CW_FORCEINLINE T get() const { return value; }
        CW_FORCEINLINE operator T() const { return value; }
        CW_FORCEINLINE obfuscated_value& operator=(T val) { value = val; return *this; }
    };

    template<typename T>
    class mba_obfuscated {
    private:
        T value{};
    public:
        mba_obfuscated() = default;
        mba_obfuscated(T val) : value(val) {}
        CW_FORCEINLINE void set(T val) { value = val; }
        CW_FORCEINLINE T get() const { return value; }
        CW_FORCEINLINE operator T() const { return value; }
        CW_FORCEINLINE mba_obfuscated& operator=(T val) { value = val; return *this; }
    };

    #define CW_ADD(a, b) ((a) + (b))
    #define CW_SUB(a, b) ((a) - (b))
    #define CW_AND(a, b) ((a) & (b))
    #define CW_OR(a, b) ((a) | (b))

    // no-op boolean obfuscation when disabled
    namespace bool_obfuscation {
        template<int N = 0> inline bool obfuscated_true() { return true; }
        template<int N = 0> inline bool obfuscated_false() { return false; }
        template<int N = 0> inline bool obfuscate_bool(bool value) { return value; }

        class obfuscated_bool {
        private:
            bool value;
        public:
            obfuscated_bool() : value(false) {}
            obfuscated_bool(bool val) : value(val) {}
            CW_FORCEINLINE bool get() const { return value; }
            CW_FORCEINLINE void set(bool val) { value = val; }
            CW_FORCEINLINE operator bool() const { return value; }
            CW_FORCEINLINE obfuscated_bool& operator=(bool val) { value = val; return *this; }
            CW_FORCEINLINE obfuscated_bool operator!() const { return obfuscated_bool(!value); }
            CW_FORCEINLINE obfuscated_bool operator&&(bool other) const { return obfuscated_bool(value && other); }
            CW_FORCEINLINE obfuscated_bool operator||(bool other) const { return obfuscated_bool(value || other); }
        };
    }

    #define CW_TRUE (true)
    #define CW_FALSE (false)
    #define CW_BOOL(x) (x)
#endif

    // =================================================================
    // control flow obfuscation
    // =================================================================

#if CW_ENABLE_CONTROL_FLOW
    namespace control_flow {

        // improved opaque predicates using runtime values
        template<int N = CW_RAND_CT(1, 100)>
        CW_FORCEINLINE bool opaque_true() {
            // use runtime address for non-trivial computation
            volatile int runtime_val = reinterpret_cast<uintptr_t>(&runtime_val) & 0xFF;
            int x = N + runtime_val;
            int y = N * 2 + runtime_val;
            CW_COMPILER_BARRIER();

            // mathematical property: (x * (x+1)) is always even
            bool result = ((x * (x + 1)) % 2) == 0;

            // additional non-obvious properties
            result = result && ((x * x - y * y + y * y) == (x * x));
            result = result && ((x | y) >= x);

            CW_COMPILER_BARRIER();
            return result;
        }

        template<int N = CW_RAND_CT(1, 100)>
        CW_FORCEINLINE bool opaque_false() {
            volatile int runtime_val = reinterpret_cast<uintptr_t>(&runtime_val) & 0xFF;
            int x = N + runtime_val;
            CW_COMPILER_BARRIER();

            // property that's always false: x^2 < 0 for real numbers
            bool result = (x * x) < 0;
            result = result || ((x & (~x)) != 0);
            result = result || ((x ^ x) != 0);

            CW_COMPILER_BARRIER();
            return result;
        }

        // control flow flattening via state machine with runtime keys
        template<typename Func>
        class flattened_flow {
        private:
            // runtime-generated state values
            int state_start;
            int state_exec;
            int state_end;
            int state_fake1;
            int state_fake2;
            uint32_t state_xor_key;

            void init_states() {
                state_xor_key = static_cast<uint32_t>(CW_RANDOM_RT());
                state_start = CW_RAND_RT(100, 200);
                state_exec = CW_RAND_RT(201, 300);
                state_end = CW_RAND_RT(301, 400);
                state_fake1 = CW_RAND_RT(401, 500);
                state_fake2 = CW_RAND_RT(501, 600);
            }

        public:
            flattened_flow() {
                init_states();
            }

            template<typename... Args>
            CW_FORCEINLINE auto execute(Func func, Args&&... args) -> decltype(func(std::forward<Args>(args)...)) {
                int state = state_start;
                int next_state = 0;
                CW_COMPILER_BARRIER();

                using ResultType = decltype(func(std::forward<Args>(args)...));
                ResultType result{};

                int iteration = 0;
                while(state != state_end && iteration++ < 100) {
                    // use runtime key for state comparison
                    int decoded_state = state ^ state_xor_key;

                    switch(decoded_state) {
                        case 0xDEADBEEF: // will never match
                            next_state = state_fake1 ^ state_xor_key;
                            break;

                        default:
                            // decode state and route appropriately
                            if (state == state_start) {
                                next_state = opaque_true<>() ? state_exec : state_fake1;
                                if (opaque_false<>()) next_state = state_fake2;
                            } else if (state == state_exec) {
                                result = func(std::forward<Args>(args)...);
                                next_state = state_end;
                                if (opaque_false<>()) next_state = state_fake1;
                            } else if (state == state_fake1) {
                                // actually execute fake path occasionally
                                volatile int dummy = 42;
                                CW_COMPILER_BARRIER();
                                dummy = dummy * 2 + 1;
                                next_state = state_exec;
                            } else if (state == state_fake2) {
                                // another fake path
                                volatile float dummy2 = 3.14f;
                                CW_COMPILER_BARRIER();
                                dummy2 = dummy2 * 2.0f;
                                next_state = state_start;
                            } else {
                                next_state = state_start;
                            }
                            break;
                    }

                    state = next_state;

                    // anti-debug check inline
                    if ((iteration % 10) == 0) {
                        CW_INLINE_CHECK();
                    }

                    for(int i = 0; i < CW_RAND_CT(1, 3); ++i) {
                        CW_COMPILER_BARRIER();
                    }
                }

                return result;
            }
        };

        // indirect branching via computed goto
        template<typename T>
        CW_FORCEINLINE T indirect_branch(T value) {
            T result = value;
            int selector = static_cast<int>(CW_RANDOM_RT() % 5);
            CW_COMPILER_BARRIER();

            // create multiple paths that all lead to same result
            switch(selector) {
                case 0:
                    result = result ^ 0 ^ 0;
                    break;
                case 1:
                    result = (result * 1) / 1;
                    break;
                case 2:
                    result = result + 0 - 0;
                    break;
                case 3:
                    result = result | 0 & ~0;
                    break;
                case 4:
                    result = result << 0 >> 0;
                    break;
            }

            return result;
        }
    }

    // macros for control flow obfuscation
    #define CW_IF(cond) \
        if(cloakwork::control_flow::opaque_true<>() && (cond))

    #define CW_ELSE \
        else if(cloakwork::control_flow::opaque_true<>())

    #define CW_FLATTEN(func, ...) \
        cloakwork::control_flow::flattened_flow<decltype(func)>().execute(func, __VA_ARGS__)
#else
    namespace control_flow {
        template<int N = 0> inline bool opaque_true() { return true; }
        template<int N = 0> inline bool opaque_false() { return false; }
        template<typename T> inline T indirect_branch(T value) { return value; }
    }
    #define CW_IF(cond) if(cond)
    #define CW_ELSE else
    #define CW_FLATTEN(func, ...) func(__VA_ARGS__)
#endif

    // =================================================================
    // function pointer obfuscation and indirection
    // =================================================================

#if CW_ENABLE_FUNCTION_OBFUSCATION
    // obfuscation constants
    constexpr uintptr_t PTR_OBFUSCATION_CONSTANT = 0xDEADBEEFCAFEBABE;
    constexpr uintptr_t PTR_XOR_LAYER = 0xCAFEBABEDEADC0DE;

    template<typename Func>
    class obfuscated_call {
    private:
        uintptr_t obfuscated_ptr;
        uintptr_t xor_key;

        static constexpr size_t DECOY_COUNT = 10;
        uintptr_t decoys[DECOY_COUNT];

        CW_FORCEINLINE uintptr_t obfuscate_ptr(Func* ptr) {
            uintptr_t addr = std::bit_cast<uintptr_t>(ptr);

            // multi-stage obfuscation
            addr = std::rotl(addr, 13);
            addr ^= xor_key;
            addr = std::rotr(addr, 7);
            addr += PTR_OBFUSCATION_CONSTANT;

            return addr;
        }

        CW_FORCEINLINE Func* deobfuscate_ptr(uintptr_t obf) {
            uintptr_t addr = obf;
            addr -= PTR_OBFUSCATION_CONSTANT;
            addr = std::rotl(addr, 7);
            addr ^= xor_key;
            addr = std::rotr(addr, 13);

            return std::bit_cast<Func*>(addr);
        }

    public:
        obfuscated_call(Func* func)
            : xor_key(CW_RANDOM_RT() | PTR_XOR_LAYER) {

            obfuscated_ptr = obfuscate_ptr(func);

            // fill decoy array with garbage
            for(size_t i = 0; i < DECOY_COUNT; ++i) {
                decoys[i] = CW_RANDOM_RT() ^ xor_key;
            }

            // randomly place real pointer in decoy array
            decoys[CW_RAND_RT(0, DECOY_COUNT-1)] = obfuscated_ptr;
        }

        template<typename... Args>
        CW_FORCEINLINE auto operator()(Args&&... args) {
            // periodic inline checks instead of every call (reduces overhead)
            static CW_ATOMIC(uint32_t) call_count{0};
            if ((++call_count % 100) == 0) {
                CW_INLINE_CHECK();
            }

            Func* real_func = deobfuscate_ptr(obfuscated_ptr);

            auto call_wrapper = [&]() {
                return real_func(std::forward<Args>(args)...);
            };

            return call_wrapper();
        }
    };
#else
    template<typename Func>
    class obfuscated_call {
    private:
        Func* func_ptr;
    public:
        obfuscated_call(Func* func) : func_ptr(func) {}
        template<typename... Args>
        CW_FORCEINLINE auto operator()(Args&&... args) {
            return func_ptr(std::forward<Args>(args)...);
        }
    };
#endif

    // =================================================================
    // advanced data hiding techniques
    // =================================================================

#if CW_ENABLE_DATA_HIDING
    namespace data_hiding {

        // actually scatter data across heap allocations (not just logical)
        template<typename T, size_t Chunks = 8>
        class scattered_value {
        private:
            static_assert(Chunks > 1 && Chunks <= 64, "Chunks must be between 2 and 64");
            static_assert(sizeof(T) >= Chunks || Chunks == 2, "Too many chunks for type size");

            struct chunk_holder {
                std::unique_ptr<uint8_t[]> data;
                size_t size;
                uint8_t xor_key;

                chunk_holder() : size(0), xor_key(0) {}
            };

            std::array<chunk_holder, Chunks> chunks;

            void scatter_data(const T& value) {
                const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                size_t bytes_per_chunk = sizeof(T) / Chunks;
                size_t remainder = sizeof(T) % Chunks;
                size_t byte_idx = 0;

                for(size_t i = 0; i < Chunks; ++i) {
                    size_t chunk_size = bytes_per_chunk + (i < remainder ? 1 : 0);
                    chunks[i].size = chunk_size;
                    chunks[i].data = std::make_unique<uint8_t[]>(chunk_size);
                    chunks[i].xor_key = static_cast<uint8_t>(CW_RANDOM_RT());

                    for(size_t j = 0; j < chunk_size && byte_idx < sizeof(T); ++j, ++byte_idx) {
                        chunks[i].data[j] = bytes[byte_idx] ^ chunks[i].xor_key;
                    }
                }
            }

        public:
            scattered_value() {
                T default_value{};
                scatter_data(default_value);
            }

            scattered_value(const T& value) {
                scatter_data(value);
            }

            CW_FORCEINLINE T get() const {
                T result;
                uint8_t* result_bytes = reinterpret_cast<uint8_t*>(&result);
                size_t byte_idx = 0;

                for(size_t i = 0; i < Chunks; ++i) {
                    for(size_t j = 0; j < chunks[i].size && byte_idx < sizeof(T); ++j, ++byte_idx) {
                        result_bytes[byte_idx] = chunks[i].data[j] ^ chunks[i].xor_key;
                    }
                }

                return result;
            }

            CW_FORCEINLINE operator T() const { return get(); }

            CW_FORCEINLINE void set(const T& value) {
                scatter_data(value);
            }
        };

        // polymorphic value that changes representation
        template<Arithmetic T>
        class polymorphic_value {
        private:
            mutable T value;
            mutable CW_ATOMIC(uint32_t) mutation_count{0};

            CW_FORCEINLINE void mutate() const {
                if(++mutation_count % 100 == 0) {
                    T temp = value;

                    uint32_t transform = CW_RANDOM_RT() % 4;
                    CW_COMPILER_BARRIER();

                    switch(transform) {
                        case 0:
                            if constexpr(Integral<T> && sizeof(T) <= sizeof(uint64_t)) {
                                if constexpr(sizeof(T) == 8) {
                                    uint64_t bits = std::bit_cast<uint64_t>(temp);
                                    bits = ~bits;
                                    value = std::bit_cast<T>(bits);
                                    bits = std::bit_cast<uint64_t>(value);
                                    bits = ~bits;
                                    value = std::bit_cast<T>(bits);
                                } else if constexpr(sizeof(T) == 4) {
                                    uint32_t bits = std::bit_cast<uint32_t>(temp);
                                    bits = ~bits;
                                    value = std::bit_cast<T>(bits);
                                    bits = std::bit_cast<uint32_t>(value);
                                    bits = ~bits;
                                    value = std::bit_cast<T>(bits);
                                } else if constexpr(sizeof(T) == 2) {
                                    uint16_t bits = std::bit_cast<uint16_t>(temp);
                                    bits = ~bits;
                                    value = std::bit_cast<T>(bits);
                                } else {
                                    value = temp;
                                }
                            }
                            break;
                        case 1:
                            if constexpr(std::is_unsigned_v<T> && Integral<T>) {
                                value = std::rotl(value, 1);
                                value = std::rotr(value, 1);
                            }
                            break;
                        case 2:
                            if constexpr(Arithmetic<T>) {
                                T key = static_cast<T>(mutation_count.load());
                                value = temp + key;
                                value = value - key;
                            }
                            break;
                        case 3:
                            break;
                    }
                }
            }

        public:
            polymorphic_value() : value{} {}
            polymorphic_value(T val) : value(val) {}

            CW_FORCEINLINE T get() const {
                mutate();
                return value;
            }

            CW_FORCEINLINE void set(T val) {
                value = val;
                mutate();
            }

            CW_FORCEINLINE operator T() const { return get(); }
        };
    }
#else
    namespace data_hiding {
        template<typename T, size_t Chunks = 8>
        class scattered_value {
        private:
            T value;
        public:
            scattered_value() : value{} {}
            scattered_value(const T& val) : value(val) {}
            CW_FORCEINLINE T get() const { return value; }
            CW_FORCEINLINE operator T() const { return value; }
            CW_FORCEINLINE void set(const T& val) { value = val; }
        };

        template<typename T>
        class polymorphic_value {
        private:
            T value;
        public:
            polymorphic_value() : value{} {}
            polymorphic_value(T val) : value(val) {}
            CW_FORCEINLINE T get() const { return value; }
            CW_FORCEINLINE void set(T val) { value = val; }
            CW_FORCEINLINE operator T() const { return value; }
        };
    }
#endif

    // =================================================================
    // metamorphic code generation
    // =================================================================

#if CW_ENABLE_METAMORPHIC
    namespace metamorphic {

        template<typename Func>
        class metamorphic_function {
        private:
            static constexpr size_t MAX_MUTATIONS = 5;

            struct mutation {
                Func* func;
                uint32_t key;
            };

            mutation mutations[MAX_MUTATIONS];
            mutable CW_ATOMIC(size_t) current_mutation{0};

        public:
            metamorphic_function(std::initializer_list<Func*> funcs) {
                size_t i = 0;
                for(auto* func : funcs) {
                    if(i < MAX_MUTATIONS) {
                        mutations[i].func = func;
                        mutations[i].key = CW_RANDOM_RT();
                        ++i;
                    }
                }

                while(i < MAX_MUTATIONS) {
                    mutations[i] = mutations[0];
                    ++i;
                }
            }

            template<typename... Args>
            CW_FORCEINLINE auto operator()(Args&&... args) const {
                size_t next = (current_mutation.load() + 1) % MAX_MUTATIONS;

                if(CW_RANDOM_RT() % 100 < 20) {
                    next = CW_RANDOM_RT() % MAX_MUTATIONS;
                }

                current_mutation.store(next);
                return mutations[next].func(std::forward<Args>(args)...);
            }
        };
    }
#else
    // stub when metamorphic is disabled - just wraps a single function pointer
    namespace metamorphic {
        template<typename Func>
        class metamorphic_function {
        private:
            Func* func_ptr;
        public:
            metamorphic_function(Func* func) : func_ptr(func) {}
            template<typename... Args>
            CW_FORCEINLINE auto operator()(Args&&... args) const {
                return func_ptr(std::forward<Args>(args)...);
            }
        };
    }
#endif

    // =================================================================
    // import hiding and dynamic API resolution
    // =================================================================

#if CW_ENABLE_IMPORT_HIDING
    namespace imports {

        // get module base by hash
        CW_FORCEINLINE void* getModuleBase(uint32_t moduleHash) {
#if CW_KERNEL_MODE
            // kernel mode: walk the driver section list via DRIVER_OBJECT
            // we use PsLoadedModuleList which is the kernel's loaded module list

            // first, try to find ntoskrnl.exe base using a known export
            // this is a common kernel technique
            typedef PVOID (*RtlPcToFileHeaderFn)(PVOID PcValue, PVOID* BaseOfImage);
            static RtlPcToFileHeaderFn RtlPcToFileHeader = nullptr;
            static bool rtl_resolved = false;

            if (!rtl_resolved) {
                UNICODE_STRING func_name;
                RtlInitUnicodeString(&func_name, L"RtlPcToFileHeader");
                RtlPcToFileHeader = reinterpret_cast<RtlPcToFileHeaderFn>(
                    MmGetSystemRoutineAddress(&func_name));
                rtl_resolved = true;
            }

            // get ntoskrnl base first if that's what we're looking for
            if (RtlPcToFileHeader) {
                PVOID ntoskrnl_base = nullptr;
                RtlPcToFileHeader(reinterpret_cast<PVOID>(RtlPcToFileHeader), &ntoskrnl_base);

                // check if this is the module we want
                if (ntoskrnl_base) {
                    auto dos = static_cast<IMAGE_DOS_HEADER*>(ntoskrnl_base);
                    if (dos->e_magic == IMAGE_DOS_SIGNATURE) {
                        auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                            reinterpret_cast<uint8_t*>(ntoskrnl_base) + dos->e_lfanew);

                        // we found ntoskrnl, check if that's the hash we want
                        // common names to check
                        uint32_t ntoskrnl_hashes[] = {
                            hash::fnv1a_ci("ntoskrnl.exe", 12),
                            hash::fnv1a_ci("ntkrnlpa.exe", 12),
                            hash::fnv1a_ci("ntkrnlmp.exe", 12),
                        };

                        for (auto h : ntoskrnl_hashes) {
                            if (h == moduleHash) return ntoskrnl_base;
                        }
                    }
                }
            }

            // for other modules, use MmGetSystemRoutineAddress approach
            // by searching for known exports that only exist in specific modules
            // this is limited but works for common cases (ntoskrnl exports)

            return nullptr;

#elif defined(_WIN32)
            // user mode: walk PEB module list
            __try {
#ifdef _WIN64
                auto peb = reinterpret_cast<PEB*>(__readgsqword(0x60));
#else
                auto peb = reinterpret_cast<PEB*>(__readfsdword(0x30));
#endif
                if (!peb || !peb->Ldr) return nullptr;

                auto ldr = peb->Ldr;
                auto head = &ldr->InMemoryOrderModuleList;

                for (auto curr = head->Flink; curr != head; curr = curr->Flink) {
                    // use our full structure definition
                    auto entry = CONTAINING_RECORD(curr, cloakwork_internal::CW_LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
                    if (!entry->BaseDllName.Buffer) continue;

                    // use wide-to-ascii hash to match CW_HASH_CI (ascii) against wide string from PEB
                    uint32_t modHash = hash::fnv1a_runtime_ci_w2a(entry->BaseDllName.Buffer);
                    if (modHash == moduleHash) {
                        return entry->DllBase;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
#endif
            return nullptr;
        }

        // get function address by walking export table
        CW_FORCEINLINE void* getProcAddress(void* module, uint32_t funcHash) {
#if CW_KERNEL_MODE
            // kernel mode: walk export table (same as user mode but no SEH)
            if (!module) return nullptr;
            if (!MmIsAddressValid(module)) return nullptr;

            auto dos = static_cast<IMAGE_DOS_HEADER*>(module);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

            auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                reinterpret_cast<uint8_t*>(module) + dos->e_lfanew);
            if (!MmIsAddressValid(nt)) return nullptr;
            if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

            auto exportDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
            if (exportDir.VirtualAddress == 0) return nullptr;

            auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
                reinterpret_cast<uint8_t*>(module) + exportDir.VirtualAddress);
            if (!MmIsAddressValid(exports)) return nullptr;

            auto names = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfNames);
            auto ordinals = reinterpret_cast<uint16_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfNameOrdinals);
            auto functions = reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(module) + exports->AddressOfFunctions);

            for (uint32_t i = 0; i < exports->NumberOfNames; ++i) {
                auto name = reinterpret_cast<const char*>(
                    reinterpret_cast<uint8_t*>(module) + names[i]);

                if (!MmIsAddressValid(const_cast<char*>(name))) continue;

                if (hash::fnv1a_runtime(name) == funcHash) {
                    uint32_t funcRva = functions[ordinals[i]];

                    // check for forwarded export
                    if (funcRva >= exportDir.VirtualAddress &&
                        funcRva < exportDir.VirtualAddress + exportDir.Size) {
                        // forwarded export - not handling for simplicity
                        return nullptr;
                    }

                    return reinterpret_cast<uint8_t*>(module) + funcRva;
                }
            }

            return nullptr;

#elif defined(_WIN32)
            // user mode: walk export table with SEH
            if (!module) return nullptr;

            __try {
                auto dos = static_cast<IMAGE_DOS_HEADER*>(module);
                if (dos->e_magic != IMAGE_DOS_SIGNATURE) return nullptr;

                auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                    reinterpret_cast<uint8_t*>(module) + dos->e_lfanew);
                if (nt->Signature != IMAGE_NT_SIGNATURE) return nullptr;

                auto exportDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
                if (exportDir.VirtualAddress == 0) return nullptr;

                auto exports = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(
                    reinterpret_cast<uint8_t*>(module) + exportDir.VirtualAddress);

                auto names = reinterpret_cast<uint32_t*>(
                    reinterpret_cast<uint8_t*>(module) + exports->AddressOfNames);
                auto ordinals = reinterpret_cast<uint16_t*>(
                    reinterpret_cast<uint8_t*>(module) + exports->AddressOfNameOrdinals);
                auto functions = reinterpret_cast<uint32_t*>(
                    reinterpret_cast<uint8_t*>(module) + exports->AddressOfFunctions);

                for (uint32_t i = 0; i < exports->NumberOfNames; ++i) {
                    auto name = reinterpret_cast<const char*>(
                        reinterpret_cast<uint8_t*>(module) + names[i]);

                    if (hash::fnv1a_runtime(name) == funcHash) {
                        uint32_t funcRva = functions[ordinals[i]];

                        // check for forwarded export
                        if (funcRva >= exportDir.VirtualAddress &&
                            funcRva < exportDir.VirtualAddress + exportDir.Size) {
                            // forwarded export - not handling for simplicity
                            return nullptr;
                        }

                        return reinterpret_cast<uint8_t*>(module) + funcRva;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
#endif
            return nullptr;
        }

        // cached import resolution
        template<uint32_t ModuleHash, uint32_t FuncHash>
        CW_FORCEINLINE void* getCachedImport() {
            static void* cached = nullptr;
            if (!cached) {
                void* mod = getModuleBase(ModuleHash);
                if (mod) cached = getProcAddress(mod, FuncHash);
            }
            return cached;
        }
    }

    // macro for hidden imports (uses case-insensitive hash for module names)
    #define CW_IMPORT(mod, func) \
        reinterpret_cast<decltype(&func)>( \
            cloakwork::imports::getCachedImport<CW_HASH_CI(mod), CW_HASH(#func)>())

    #define CW_IMPORT_WIDE(mod, func) \
        reinterpret_cast<decltype(&func)>( \
            cloakwork::imports::getCachedImport<CW_HASH_WIDE(mod), CW_HASH(#func)>())
#else
    namespace imports {
        inline void* getModuleBase(uint32_t) { return nullptr; }
        inline void* getProcAddress(void*, uint32_t) { return nullptr; }
    }
    #define CW_IMPORT(mod, func) (&func)
    #define CW_IMPORT_WIDE(mod, func) (&func)
#endif

    // =================================================================
    // direct syscalls
    // =================================================================

#if CW_ENABLE_SYSCALLS
    namespace syscall {

        // get syscall number from ntdll function
        CW_FORCEINLINE uint32_t getSyscallNumber(uint32_t funcHash) {
#ifdef _WIN32
            __try {
                void* ntdll = imports::getModuleBase(CW_HASH_CI("ntdll.dll"));
                if (!ntdll) return 0;

                auto func = reinterpret_cast<uint8_t*>(imports::getProcAddress(ntdll, funcHash));
                if (!func) return 0;

                // pattern: mov r10, rcx; mov eax, <syscall_number>
                // 4C 8B D1 B8 XX XX XX XX
                if (func[0] == 0x4C && func[1] == 0x8B && func[2] == 0xD1 && func[3] == 0xB8) {
                    return *reinterpret_cast<uint32_t*>(func + 4);
                }

                // alternative pattern for older windows
                // B8 XX XX XX XX
                if (func[0] == 0xB8) {
                    return *reinterpret_cast<uint32_t*>(func + 1);
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return 0;
            }
#endif
            return 0;
        }

        // cached syscall number
        template<uint32_t FuncHash>
        CW_FORCEINLINE uint32_t getCachedSyscallNumber() {
            static uint32_t cached = 0;
            if (cached == 0) {
                cached = getSyscallNumber(FuncHash);
            }
            return cached;
        }

#ifdef _WIN64
        // x64 syscall invoker - uses inline assembly via intrinsic workaround
        // note: actual syscall invocation requires asm. this provides the infrastructure.
        extern "C" NTSTATUS __syscall_stub(uint32_t syscallNumber, ...);
#endif
    }

    #define CW_SYSCALL_NUMBER(func) (cloakwork::syscall::getCachedSyscallNumber<CW_HASH(#func)>())
#else
    namespace syscall {
        inline uint32_t getSyscallNumber(uint32_t) { return 0; }
    }
    #define CW_SYSCALL_NUMBER(func) (0)
#endif

    // =================================================================
    // obfuscated comparisons
    // =================================================================

#if CW_ENABLE_VALUE_OBFUSCATION
    namespace comparison {

        // obfuscated equality check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_equals(T a, T b) {
            if constexpr (std::is_integral_v<T>) {
                // (a == b) <=> ((a ^ b) == 0)
                T diff = a ^ b;
                // use MBA to check if zero
                T zero_check = mba::sub_mba(diff, diff);
                CW_COMPILER_BARRIER();
                return zero_check == static_cast<T>(0) && diff == static_cast<T>(0);
            } else {
                return a == b;
            }
        }

        // obfuscated not-equals check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_not_equals(T a, T b) {
            if constexpr (std::is_integral_v<T>) {
                T diff = a ^ b;
                CW_COMPILER_BARRIER();
                return diff != static_cast<T>(0);
            } else {
                return a != b;
            }
        }

        // obfuscated less-than check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_less(T a, T b) {
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
                // use subtraction and check sign bit
                T diff = mba::sub_mba(a, b);
                CW_COMPILER_BARRIER();
                return diff < 0;
            } else if constexpr (std::is_integral_v<T>) {
                // unsigned comparison via subtraction with overflow check
                CW_COMPILER_BARRIER();
                return a < b;  // fall back for unsigned
            } else {
                return a < b;
            }
        }

        // obfuscated greater-than check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_greater(T a, T b) {
            return obfuscated_less(b, a);
        }

        // obfuscated less-or-equal check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_less_equal(T a, T b) {
            return !obfuscated_greater(a, b);
        }

        // obfuscated greater-or-equal check
        template<typename T>
        CW_FORCEINLINE bool obfuscated_greater_equal(T a, T b) {
            return !obfuscated_less(a, b);
        }
    }

    #define CW_EQ(a, b) (cloakwork::comparison::obfuscated_equals((a), (b)))
    #define CW_NE(a, b) (cloakwork::comparison::obfuscated_not_equals((a), (b)))
    #define CW_LT(a, b) (cloakwork::comparison::obfuscated_less((a), (b)))
    #define CW_GT(a, b) (cloakwork::comparison::obfuscated_greater((a), (b)))
    #define CW_LE(a, b) (cloakwork::comparison::obfuscated_less_equal((a), (b)))
    #define CW_GE(a, b) (cloakwork::comparison::obfuscated_greater_equal((a), (b)))
#else
    // stubs when value obfuscation is disabled
    #define CW_EQ(a, b) ((a) == (b))
    #define CW_NE(a, b) ((a) != (b))
    #define CW_LT(a, b) ((a) < (b))
    #define CW_GT(a, b) ((a) > (b))
    #define CW_LE(a, b) ((a) <= (b))
    #define CW_GE(a, b) ((a) >= (b))
#endif

    // =================================================================
    // encrypted compile-time constants
    // =================================================================

    namespace constants {

        template<typename T, T Value, uint8_t Key = static_cast<uint8_t>(CW_RAND_CT(1, 255))>
        struct encrypted_constant {
            static CW_FORCEINLINE T get() {
                if constexpr (std::is_integral_v<T>) {
                    constexpr T encrypted = Value ^ static_cast<T>(Key);
                    volatile T result = encrypted;
                    CW_COMPILER_BARRIER();
                    return result ^ static_cast<T>(Key);
                } else {
                    return Value;
                }
            }
        };

        // runtime-keyed constant (different each execution)
        template<typename T>
        class runtime_constant {
        private:
            T encrypted;
            T key;

        public:
            runtime_constant(T value) {
                key = static_cast<T>(CW_RANDOM_RT());
                if constexpr (std::is_integral_v<T>) {
                    encrypted = value ^ key;
                } else {
                    encrypted = value;
                }
            }

            CW_FORCEINLINE T get() const {
                if constexpr (std::is_integral_v<T>) {
                    volatile T temp = encrypted;
                    CW_COMPILER_BARRIER();
                    return temp ^ key;
                } else {
                    return encrypted;
                }
            }

            CW_FORCEINLINE operator T() const { return get(); }
        };
    }

    #define CW_CONST(val) \
        (cloakwork::constants::encrypted_constant<decltype(val), val>::get())

    // =================================================================
    // junk code insertion
    // =================================================================

#if CW_ENABLE_CONTROL_FLOW
    namespace junk {

        // generate junk computation that looks real but does nothing useful
        template<int N = CW_RAND_CT(1, 1000)>
        CW_NOINLINE void junk_computation() {
            volatile int x = N;
            volatile int y = N * 2;
            CW_COMPILER_BARRIER();

            // meaningless operations
            x = x ^ y;
            y = y + x;
            x = x - y;
            y = ~y;
            x = x & y;

            CW_COMPILER_BARRIER();

            // dead branch that will never execute
            if (control_flow::opaque_false<N>()) {
                volatile int z = x * y;
                z = z >> 3;
                x = z ^ y;
            }
        }

        // more complex junk with fake control flow
        template<int N = CW_RAND_CT(1, 1000)>
        CW_NOINLINE void junk_control_flow() {
            volatile int state = N % 5;
            CW_COMPILER_BARRIER();

            for (int i = 0; i < 3; ++i) {
                switch (state) {
                    case 0:
                        state = (state + 1) % 5;
                        break;
                    case 1:
                        state = (state * 2) % 5;
                        break;
                    case 2:
                        state = (state - 1 + 5) % 5;
                        break;
                    default:
                        state = 0;
                        break;
                }
                CW_COMPILER_BARRIER();
            }
        }
    }

    // junk code insertion macro
    #define CW_JUNK() \
        do { \
            cloakwork::junk::junk_computation<CW_RAND_CT(1, 1000)>(); \
        } while(0)

    #define CW_JUNK_FLOW() \
        do { \
            cloakwork::junk::junk_control_flow<CW_RAND_CT(1, 1000)>(); \
        } while(0)
#else
    #define CW_JUNK() ((void)0)
    #define CW_JUNK_FLOW() ((void)0)
#endif

    // =================================================================
    // return address spoofing
    // =================================================================

#if CW_ENABLE_FUNCTION_OBFUSCATION
    namespace spoof {

        // find a "ret" gadget in a legitimate module
        CW_FORCEINLINE void* findRetGadget() {
#ifdef _WIN32
            __try {
                void* ntdll = imports::getModuleBase(CW_HASH_CI("ntdll.dll"));
                if (!ntdll) return nullptr;

                auto dos = static_cast<IMAGE_DOS_HEADER*>(ntdll);
                auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                    reinterpret_cast<uint8_t*>(ntdll) + dos->e_lfanew);

                auto base = reinterpret_cast<uint8_t*>(ntdll);
                size_t size = nt->OptionalHeader.SizeOfImage;

                // search for ret (0xC3) gadget
                for (size_t i = 0; i < size - 1; ++i) {
                    if (base[i] == 0xC3) {
                        return base + i;
                    }
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                return nullptr;
            }
#endif
            return nullptr;
        }

        // cached ret gadget
        CW_FORCEINLINE void* getRetGadget() {
            static void* gadget = nullptr;
            if (!gadget) {
                gadget = findRetGadget();
            }
            return gadget;
        }

        // spoofed call wrapper - stores real return address and uses gadget
        // note: full implementation requires asm for stack manipulation
        template<typename Ret, typename... Args>
        class spoofed_call {
        private:
            using FuncPtr = Ret(*)(Args...);
            FuncPtr func;
            void* gadget;

        public:
            spoofed_call(FuncPtr f) : func(f), gadget(getRetGadget()) {}

            CW_FORCEINLINE Ret operator()(Args... args) {
                // simplified version - full spoof requires asm
                CW_COMPILER_BARRIER();
                return func(args...);
            }
        };
    }

    #define CW_SPOOF_CALL(func) (cloakwork::spoof::spoofed_call<decltype(func)>{func})
#else
    namespace spoof {
        inline void* findRetGadget() { return nullptr; }
        inline void* getRetGadget() { return nullptr; }
    }
    #define CW_SPOOF_CALL(func) (func)
#endif

    // =================================================================
    // self-integrity verification
    // =================================================================

#if CW_ENABLE_INTEGRITY_CHECKS
    namespace integrity {

        // compute hash of memory region
        CW_FORCEINLINE uint32_t computeHash(const void* data, size_t size) {
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            uint32_t hash = 0x811c9dc5;

            for (size_t i = 0; i < size; ++i) {
                hash ^= bytes[i];
                hash *= 0x01000193;
            }

            return hash;
        }

        // integrity-checked function wrapper
        template<typename Func>
        class integrity_checked {
        private:
            Func* func;
            uint32_t expectedHash;
            size_t codeSize;
            mutable CW_ATOMIC(uint32_t) checkCount{0};

        public:
            integrity_checked(Func* f, size_t size)
                : func(f), codeSize(size) {
                expectedHash = computeHash(reinterpret_cast<const void*>(f), size);
            }

            template<typename... Args>
            CW_FORCEINLINE auto operator()(Args&&... args) {
                // periodic integrity check
                if ((++checkCount % 100) == 0) {
                    uint32_t currentHash = computeHash(
                        reinterpret_cast<const void*>(func), codeSize);

                    if (currentHash != expectedHash) {
                        // tampered - crash
#if CW_ANTI_DEBUG_RESPONSE == 1
                        __debugbreak();
                        *(volatile int*)0 = 0;
#endif
                    }
                }

                return func(std::forward<Args>(args)...);
            }

            bool verify() const {
                uint32_t currentHash = computeHash(
                    reinterpret_cast<const void*>(func), codeSize);
                return currentHash == expectedHash;
            }
        };

        // check for common hook patterns at function start
        CW_FORCEINLINE bool detectHook(const void* func) {
#ifdef _WIN32
            const uint8_t* bytes = static_cast<const uint8_t*>(func);

            // check for jmp rel32 (E9 XX XX XX XX)
            if (bytes[0] == 0xE9) return true;

            // check for jmp [rip+disp32] (FF 25 XX XX XX XX)
            if (bytes[0] == 0xFF && bytes[1] == 0x25) return true;

            // check for mov rax, addr; jmp rax (48 B8 XX XX XX XX XX XX XX XX FF E0)
            if (bytes[0] == 0x48 && bytes[1] == 0xB8) return true;

            // check for push addr; ret (68 XX XX XX XX C3)
            if (bytes[0] == 0x68 && bytes[5] == 0xC3) return true;

            // check for int3 breakpoint
            if (bytes[0] == 0xCC) return true;
#endif
            return false;
        }

        // verify multiple functions at once
        template<typename... Funcs>
        CW_FORCEINLINE bool verifyFunctions(Funcs*... funcs) {
            return ((!detectHook(reinterpret_cast<const void*>(funcs))) && ...);
        }
    }

    #define CW_INTEGRITY_CHECK(func, size) \
        (cloakwork::integrity::integrity_checked<decltype(func)>{&func, size})

    #define CW_DETECT_HOOK(func) \
        (cloakwork::integrity::detectHook(reinterpret_cast<const void*>(&func)))
#else
    namespace integrity {
        inline uint32_t computeHash(const void*, size_t) { return 0; }
        inline bool detectHook(const void*) { return false; }
        template<typename... Funcs>
        inline bool verifyFunctions(Funcs*...) { return true; }
    }
    #define CW_INTEGRITY_CHECK(func, size) (&func)
    #define CW_DETECT_HOOK(func) (false)
#endif

    // =================================================================
    // convenience macros
    // =================================================================

    #if CW_ENABLE_VALUE_OBFUSCATION
        #define CW_INT(x) (cloakwork::obfuscated_value<decltype(x)>{x})
        #define CW_MBA(x) (cloakwork::mba_obfuscated<decltype(x)>{x})
    #else
        #define CW_INT(x) (x)
        #define CW_MBA(x) (x)
    #endif

    #if CW_ENABLE_FUNCTION_OBFUSCATION
        #define CW_CALL(func) cloakwork::obfuscated_call<decltype(func)>{func}
    #else
        #define CW_CALL(func) (func)
    #endif

    #if CW_ENABLE_DATA_HIDING
        #define CW_SCATTER(x) (cloakwork::data_hiding::scattered_value<decltype(x)>{x})
        #define CW_POLY(x) (cloakwork::data_hiding::polymorphic_value<decltype(x)>{x})
    #else
        #define CW_SCATTER(x) (x)
        #define CW_POLY(x) (x)
    #endif

    #if CW_ENABLE_ANTI_DEBUG
        #define CW_CHECK_ANALYSIS() \
            do { \
                if(cloakwork::anti_debug::comprehensive_check()) { \
                    volatile int crash = *(int*)0; \
                } \
            } while(0)
    #else
        #define CW_CHECK_ANALYSIS() ((void)0)
    #endif

    #if CW_ENABLE_CONTROL_FLOW
        #define CW_BRANCH(cond) \
            if(cloakwork::control_flow::indirect_branch(cloakwork::control_flow::opaque_true<>() && (cond)))
    #else
        #define CW_BRANCH(cond) if(cond)
    #endif

} // namespace cloakwork

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif // CLOAKWORK_H
