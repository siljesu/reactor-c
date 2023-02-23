/**
 * @file
 * @author Edward A. Lee
 *
 * @section LICENSE
Copyright (c) 2020, The University of California at Berkeley and TU Dresden

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

 * @section DESCRIPTION
 * Definitions of tracepoint events for use with the C code generator and any other
 * code generator that uses the C infrastructure (such as the Python code generator).
 *
 * See: https://www.lf-lang.org/docs/handbook/tracing?target=c
 *
 * The trace file is named trace.lft and is a binary file with the following format:
 *
 * Header:
 * * instant_t: The start time. This is both the starting physical time and the starting logical time.
 * * int: Size N of the table mapping pointers to descriptions.
 * This is followed by N records each of which has:
 * * A pointer value (the key).
 * * A null-terminated string (the description).
 *
 * Traces:
 * A sequence of traces, each of which begins with an int giving the length of the trace
 * followed by binary representations of the trace_record struct written using fwrite().
 */
#ifndef TRACE_H
#define TRACE_H

#include "lf_types.h"

#ifdef FEDERATED
#include "net_common.h"
#endif // FEDERATED

/**
 * Trace event types. If you update this, be sure to update the
 * string representation below. Also, create a tracepoint function
 * for each event type.
 */
typedef enum {
    reaction_starts,
    reaction_ends,
    schedule_called,
    user_event,
    user_value,
    worker_wait_starts,
    worker_wait_ends,
    scheduler_advancing_time_starts,
    scheduler_advancing_time_ends,
    federate_NET,
    federate_TAG,
    federate_PTAG,
    federate_LTC,
    rti_receive_TIMESTAMP,
    rti_receive_ADDRESS_QUERY,
    rti_receive_ADDRESS_ADVERTISEMENT,
    rti_receive_TAGGED_MESSAGE,
    rti_receive_RESIGN,
    rti_receive_NEXT_EVENT_TAG,
    rti_receive_LOGICAL_TAG_COMPLETE,
    rti_receive_STOP_REQUEST,
    rti_receive_STOP_REQUEST_REPLY,
    rti_receive_PORT_ABSENT,
    rti_receive_unidentified,
    NUM_EVENT_TYPES
} trace_event_t;

#ifdef LF_TRACE

/**
 * String description of event types.
 */
static const char *trace_event_names[] = {
        "Reaction starts",
        "Reaction ends",
        "Schedule called",
        "User-defined event",
        "User-defined valued event",
        "Worker wait starts",
        "Worker wait ends",
        "Scheduler advancing time starts",
        "Scheduler advancing time ends",
        "Federate sends NET to RTI",
        "Federate receives TAG from RTI",
        "Federate receives PTAG from RTI",
        "Federate sends LTC to RTI",
        "RTI receives TIMESTAMP from federate",
        "RTI receives ADDRESS_QUERY from federate",
        "RTI receives ADDRESS_ADVERTISEMENT from federate",
        "RTI receives TAGGED_MESSAGE from federate",
        "RTI receives RESIGN from federate",
        "RTI receives NEXT_EVENT_TAG from federate",
        "RTI receives LOGICAL_TAG_COMPLETE from federate",
        "RTI receives STOP_REQUEST from federate",
        "RTI receives STOP_REQUEST_REPLY from federate",
        "RTI receives PORT_ABSENT from federate",
        "RTI receives unidentified message from federate"
};

// FIXME: Target property should specify the capacity of the trace buffer.
#define TRACE_BUFFER_CAPACITY 2048

/** Size of the table of trace objects. */
#define TRACE_OBJECT_TABLE_SIZE 1024

/**
 * @brief A trace record that is written in binary to the trace file.
 */
typedef struct trace_record_t {
    trace_event_t event_type;
    void* pointer;  // pointer identifying the record, e.g. to self struct for a reactor.
    int id_number;
    int worker;
    instant_t logical_time;
    microstep_t microstep;
    instant_t physical_time;
    trigger_t* trigger;
    interval_t extra_delay;
} trace_record_t;

/**
 * Identifier for what is in the object table.
 */
typedef enum {
    trace_reactor,   // Self struct.
    trace_trigger,   // Timer or action (argument to schedule()).
    trace_user       // User-defined trace object.
} _lf_trace_object_t;

/**
 * Struct for table of pointers to a description of the object.
 */
typedef struct object_description_t object_description_t;
struct object_description_t {
    void* pointer;      // Pointer to the reactor self struct or other identifying pointer.
    void* trigger;      // Pointer to the trigger (action or timer) or other secondary ID, if any.
    _lf_trace_object_t type;  // The type of trace object.
    char* description; // A NULL terminated string.
};

/**
 * Register a trace object.
 * @param pointer1 Pointer that identifies the object, typically to a reactor self struct.
 * @param pointer2 Further identifying pointer, typically to a trigger (action or timer) or NULL if irrelevant.
 * @param type The type of trace object.
 * @param description The human-readable description of the object.
 * @return 1 if successful, 0 if the trace object table is full.
 */
int _lf_register_trace_event(void* pointer1, void* pointer2, _lf_trace_object_t type, char* description);

/**
 * Register a user trace event. This should be called once, providing a pointer to a string
 * that describes a phenomenon being traced. Use the same pointer as the first argument to
 * tracepoint_user_event() and tracepoint_user_value().
 * @param description Pointer to a human-readable description of the event.
 * @return 1 if successful, 0 if the trace object table is full.
 */
int register_user_trace_event(char* description);

/**
 * Open a trace file and start tracing.
 * @param filename The filename for the trace file.
 */
void start_trace(char* filename);

/**
 * Trace an event identified by a type and a pointer to the self struct of the reactor instance.
 * This is a generic tracepoint function. It is better to use one of the specific functions.
 * @param event_type The type of event (see trace_event_t in trace.h)
 * @param reactor The pointer to the self struct of the reactor instance in the trace table.
 * @param tag Pointer to a tag or NULL to use current tag.
 * @param id_number The id number (e.g. reaction or federate) or -1 if the event does not have an id number.
 * @param worker The thread number of the worker thread or 0 for unthreaded execution.
 * @param physical_time If the caller has already accessed physical time, provide it here.
 *  Otherwise, provide NULL. This argument avoids a second call to lf_time_physical()
 *  and ensures that the physical time in the trace is the same as that used by the caller.
 * @param trigger Pointer to the trigger_t struct for calls to schedule or NULL otherwise.
 * @param extra_delay The extra delay passed to schedule(). If not relevant for this event
 *  type, pass 0.
 */
void tracepoint(
        trace_event_t event_type,
        void* reactor,
        tag_t* tag,
        int id_number,
        int worker,
        instant_t* physical_time,
        trigger_t* trigger,
        interval_t extra_delay
);

/**
 * Trace the start of a reaction execution.
 * @param reaction Pointer to the reaction_t struct for the reaction.
 * @param worker The thread number of the worker thread or 0 for unthreaded execution.
 */
void tracepoint_reaction_starts(reaction_t* reaction, int worker);

/**
 * Trace the end of a reaction execution.
 * @param reaction Pointer to the reaction_t struct for the reaction.
 * @param worker The thread number of the worker thread or 0 for unthreaded execution.
 */
void tracepoint_reaction_ends(reaction_t* reaction, int worker);

/**
 * Trace a call to schedule.
 * @param trigger Pointer to the trigger_t struct for the trigger.
 * @param extra_delay The extra delay passed to schedule().
 */
void tracepoint_schedule(trigger_t* trigger, interval_t extra_delay);

/**
 * Trace a user-defined event. Before calling this, you must call
 * register_user_trace_event() with a pointer to the same string
 * or else the event will not be recognized.
 * @param description Pointer to the description string.
 */
void tracepoint_user_event(char* description);

/**
 * Trace a user-defined event with a value.
 * Before calling this, you must call
 * register_user_trace_event() with a pointer to the same string
 * or else the event will not be recognized.
 * @param description Pointer to the description string.
 * @param value The value of the event. This is a long long for
 *  convenience so that time values can be passed unchanged.
 *  But int values work as well.
 */
void tracepoint_user_value(char* description, long long value);

/**
 * Trace the start of a worker waiting for something to change on the reaction queue.
 * @param worker The thread number of the worker thread or 0 for unthreaded execution.
 */
void tracepoint_worker_wait_starts(int worker);

/**
 * Trace the end of a worker waiting for something to change on reaction queue.
 * @param worker The thread number of the worker thread or 0 for unthreaded execution.
 */
void tracepoint_worker_wait_ends(int worker);

/**
 * Trace the start of the scheduler waiting for logical time to advance or an event to
 * appear on the event queue.
 */
void tracepoint_scheduler_advancing_time_starts();

/**
 * Trace the end of the scheduler waiting for logical time to advance or an event to
 * appear on the event queue.
 */
void tracepoint_scheduler_advancing_time_ends();

////////////////////////////////////////////////////////////
//// For federated execution

#ifdef FEDERATED

/**
 * Trace sending a Next Event Tag (NET) or Logical Tag Complete (LTC) message to the RTI.
 * @param type Either MSG_TYPE_NEXT_EVENT_TAG or MSG_TYPE_LOGICAL_TAG_COMPLETE.
 * @param fed_id The federate identifier.
 * @param tag The tag that has been sent.
 */
void tracepoint_tag_to_RTI(unsigned char type, int fed_id, tag_t tag);

/**
 * Trace receiving a Tag Advance Grant (TAG) or Provisional Tag Advance Grant (PTAG) message from the RTI.
 * @param type Either MSG_TYPE_TAG_ADVANCE_GRANT or MSG_TYPE_PROVISIONAL_TAG_ADVANCE_GRANT.
 * @param fed_id The federate identifier.
 * @param tag The tag that has been received.
 */
void tracepoint_tag_from_RTI(unsigned char type, int fed_id, tag_t tag);

#endif // FEDERATED

////////////////////////////////////////////////////////////
//// For RTI execution

#ifdef RTI_TRACE

/**
 * Trace of RTI sending a message to a federate.
 * @param type ...
 * @param fed_id The fedaerate identifier.
 *        FIXME: The pointer is not correctly passed for now.
 * @param tag The tag that has been sent, if any.
 */
void tracepoint_message_to_federate(unsigned char type, const char *fed_id, tag_t tag);

/**
 * Trace RTI receiving a message from a federate.
 * @param type ....
 * @param fed_id The fedaerate identifier.
 *        FIXME: The pointer is not correctly passed for now.
 * @param tag The tag that has been received, if any.
 */
void tracepoint_message_from_federate(unsigned char type, const char *fed_id, tag_t tag);

#endif // RTI_TRACE

void stop_trace(void);

#else

// empty definition in case we compile without tracing
#define _lf_register_trace_event(...)
#define register_user_trace_event(...)
#define tracepoint(...)
#define tracepoint_reaction_starts(...)
#define tracepoint_reaction_ends(...)
#define tracepoint_schedule(...)
#define tracepoint_user_event(...)
#define tracepoint_user_value(...)
#define tracepoint_worker_wait_starts(...)
#define tracepoint_worker_wait_ends(...)
#define tracepoint_scheduler_advancing_time_starts(...);
#define tracepoint_scheduler_advancing_time_ends(...);
#define tracepoint_tag_to_RTI(...);
#define tracepoint_tag_from_RTI(...);
#define tracepoint_message_to_federate(...);
#define tracepoint_message_from_federate(...) ;

#define start_trace(...)
#define stop_trace(...)

#endif // LF_TRACE
#endif // TRACE_H
