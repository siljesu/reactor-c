/**
 * @file lf_os_single_threaded_support.c
 * @author Marten Lohstroh (marten@berkeley.edu)
 * @brief Implementation of platform functions to ensure safe concurrent
 * access to a critical section, which are unnecessary in an OS-supported
 * single-threaded runtime and therefore are left blank.
 * @note This file is only to be used in conjuction with an OS-supported
 * single-threaded runtime. If threads are enabled, this file will fail
 * to compile. If threads are needed, use a multi-threaded runtime instead.
 * @copyright BSD 2-Clause License (see LICENSE.md)
 */

#if defined(_THREADS_H) || defined(_PTHREAD_H)
    #error Usage of threads in the single-threaded runtime is not safe.
#endif

int lf_critical_section_enter() {
    // FIXME: Is this what we want? Dont we want to grab a mutex here? 
    //  Even if we are using a single-threaded LF we might have other threads 
    //  trying to schedule physical actions. Or interrupt routines.
    return 0;
}

int lf_critical_section_exit() {
    return 0;
}

int lf_notify_of_event() {
    return 0;
}

int lf_ack_events() {
    return 0;
}
