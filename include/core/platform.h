/* Platform API support for the C target of Lingua Franca. */

/*************
Copyright (c) 2021, The University of California at Berkeley.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************/

/**
 * Platform API support for the C target of Lingua Franca.
 * This file detects the platform on which the C compiler is being run
 * (e.g. Windows, Linux, Mac) and conditionally includes platform-specific
 * files that define core datatypes and function signatures for Lingua Franca.
 * For example, the type instant_t represents a time value (long long on
 * most of the platforms). The conditionally included files define a type
 * _instant_t, and this file defines the type instant_t to be whatever
 * the included defines _instant_t to be. All platform-independent code
 * in Lingua Franca, therefore, should use the type instant_t for time
 * values.
 *
 * @author{Soroush Bateni <soroush@utdallas.edu>}
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(ARDUINO)
    #include "platform/lf_arduino_support.h"
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   // Windows platforms
   #include "lf_windows_support.h"
#elif __APPLE__
    // Apple platforms
    #include "lf_macos_support.h"
#elif __linux__
    // Linux
    #include "lf_linux_support.h"
#elif __unix__ // all unices not caught above
    // Unix
    #include "lf_POSIX_threads_support.h"
#elif defined(_POSIX_VERSION)
    // POSIX
    #include "lf_POSIX_threads_support.h"
#elif defined(__riscv) || defined(__riscv__)
    // RISC-V (see https://github.com/riscv/riscv-toolchain-conventions)
    #error "RISC-V not supported"
#elif defined(__patmos__)
    #include "lf_patmos_support.h"
#else
#error "Platform not supported"
#endif

#if defined NUMBER_OF_WORKERS || defined LINGUA_FRANCA_TRACE
#define LF_TIMEOUT _LF_TIMEOUT

typedef _lf_mutex_t lf_mutex_t;          // Type to hold handle to a mutex
typedef _lf_cond_t lf_cond_t;            // Type to hold handle to a condition variable
typedef _lf_thread_t lf_thread_t;        // Type to hold handle to a thread
#endif

/**
 * Time instant. Both physical and logical times are represented
 * using this typedef.
 */
typedef _instant_t instant_t;

/**
 * Interval of time.
 */
typedef _interval_t interval_t;

/**
 * Microstep instant.
 */
typedef _microstep_t microstep_t;

#ifdef NUMBER_OF_WORKERS

/**
 * @brief Get the number of cores on the host machine.
 */
extern int lf_available_cores();

/**
 * Create a new thread, starting with execution of lf_thread
 * getting passed arguments. The new handle is stored in thread_id.
 *
 * @return 0 on success, platform-specific error number otherwise.
 *
 */
extern int lf_thread_create(lf_thread_t* thread, void *(*lf_thread) (void *), void* arguments);

/**
 * Make calling thread wait for termination of the thread.  The
 * exit status of the thread is stored in thread_return, if thread_return
 * is not NULL.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_thread_join(lf_thread_t thread, void** thread_return);

/**
 * Initialize a mutex.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_mutex_init(lf_mutex_t* mutex);

/**
 * Lock a mutex.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_mutex_lock(lf_mutex_t* mutex);

/**
 * Unlock a mutex.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_mutex_unlock(lf_mutex_t* mutex);


/**
 * Initialize a conditional variable.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_cond_init(lf_cond_t* cond);

/**
 * Wake up all threads waiting for condition variable cond.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_cond_broadcast(lf_cond_t* cond);

/**
 * Wake up one thread waiting for condition variable cond.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_cond_signal(lf_cond_t* cond);

/**
 * Wait for condition variable "cond" to be signaled or broadcast.
 * "mutex" is assumed to be locked before.
 *
 * @return 0 on success, platform-specific error number otherwise.
 */
extern int lf_cond_wait(lf_cond_t* cond, lf_mutex_t* mutex);

/**
 * Block current thread on the condition variable until condition variable
 * pointed by "cond" is signaled or time pointed by "absolute_time_ns" in
 * nanoseconds is reached.
 *
 * @return 0 on success, LF_TIMEOUT on timeout, and platform-specific error
 *  number otherwise.
 */
extern int lf_cond_timedwait(lf_cond_t* cond, lf_mutex_t* mutex, instant_t absolute_time_ns);

/*
 * Atomically increment the variable that ptr points to by the given value, and return the original value of the variable.
 * @param ptr A pointer to a variable. The value of this variable will be replaced with the result of the operation.
 * @param value The value to be added to the variable pointed to by the ptr parameter.
 * @return The original value of the variable that ptr points to (i.e., from before the application of this operation).
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Assume that an integer is 32 bits.
#define lf_atomic_fetch_add(ptr, value) InterlockedExchangeAdd(ptr, value)
#elif defined(__GNUC__) || defined(__clang__)
#define lf_atomic_fetch_add(ptr, value) __sync_fetch_and_add(ptr, value)
#else
#error "Compiler not supported"
#endif

/*
 * Atomically increment the variable that ptr points to by the given value, and return the new value of the variable.
 * @param ptr A pointer to a variable. The value of this variable will be replaced with the result of the operation.
 * @param value The value to be added to the variable pointed to by the ptr parameter.
 * @return The new value of the variable that ptr points to (i.e., from before the application of this operation).
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Assume that an integer is 32 bits.
#define lf_atomic_add_fetch(ptr, value) InterlockedAdd(ptr, value)
#elif defined(__GNUC__) || defined(__clang__)
#define lf_atomic_add_fetch(ptr, value) __sync_add_and_fetch(ptr, value)
#else
#error "Compiler not supported"
#endif

/*
 * Atomically compare the variable that ptr points to against oldval. If the
 * current value is oldval, then write newval into *ptr.
 * @param ptr A pointer to a variable.
 * @param oldval The value to compare against.
 * @param newval The value to assign to *ptr if comparison is successful.
 * @return True if comparison was successful. False otherwise.
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
// Assume that a boolean is represented with a 32-bit integer.
#define lf_bool_compare_and_swap(ptr, oldval, newval) (InterlockedCompareExchange(ptr, newval, oldval) == oldval)
#elif defined(__GNUC__) || defined(__clang__)
#define lf_bool_compare_and_swap(ptr, oldval, newval) __sync_bool_compare_and_swap(ptr, oldval, newval)
#else
#error "Compiler not supported"
#endif

/*
 * Atomically compare the 32-bit value that ptr points to against oldval. If the
 * current value is oldval, then write newval into *ptr.
 * @param ptr A pointer to a variable.
 * @param oldval The value to compare against.
 * @param newval The value to assign to *ptr if comparison is successful.
 * @return The initial value of *ptr.
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define lf_val_compare_and_swap(ptr, oldval, newval) InterlockedCompareExchange(ptr, newval, oldval)
#elif defined(__GNUC__) || defined(__clang__)
#define lf_val_compare_and_swap(ptr, oldval, newval) __sync_val_compare_and_swap(ptr, oldval, newval)
#else
#error "Compiler not supported"
#endif

#endif

/**
 * Initialize the LF clock. Must be called before using other clock-related APIs.
 */
extern void lf_initialize_clock(void);

/**
 * Fetch the value of an internal (and platform-specific) physical clock and
 * store it in `t`.
 *
 * Ideally, the underlying platform clock should be monotonic. However, the
 * core lib tries to enforce monotonicity at higher level APIs (see tag.h).
 *
 * @return 0 for success, or -1 for failure
 */
extern int lf_clock_gettime(instant_t* t);

/**
 * Pause execution for a number of nanoseconds.
 *
 * @return 0 for success, or -1 for failure.
 */
extern int lf_nanosleep(instant_t requested_time);


/**
 * Macros for marking function as deprecated
 */
#ifdef __GNUC__
#define DEPRECATED(X) X __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(X) __declspec(deprecated) X
#else
#define DEPRECATED(X) X
#endif

#endif // PLATFORM_H
