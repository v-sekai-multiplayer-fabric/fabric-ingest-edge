/* The iceoryx2 C ABI types the harness names, declared here rather than included.
 *
 * The generated dispatch table must compile with no iceoryx2 headers present. So every
 * type in ../iceoryx2.sigs is declared below, transcribed from iceoryx2.h v0.9.3.
 *
 * Two kinds of type, and the difference is the whole safety argument.
 *
 * A handle, `iox2_..._h`, is a pointer to an opaque struct. Its size never matters here,
 * so an incomplete struct is exact.
 *
 * A storage struct, `iox2_..._t`, is a real sized struct that a caller may allocate to
 * keep an object off the heap. Its size does matter, and transcribing it would be a
 * silent memory bug the day upstream adds a field. So it stays incomplete, and the
 * harness passes NULL for every one. The C API allocates on the heap when it gets NULL.
 * That is the documented contract, and it costs one allocation for each object.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef WEFT_HARNESS_IOX2_DECLS_H
#define WEFT_HARNESS_IOX2_DECLS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Storage structs. Incomplete on purpose. Pass NULL. */
struct iox2_node_builder_t;
struct iox2_node_t;
struct iox2_service_name_t;
struct iox2_service_builder_t;
struct iox2_port_factory_pub_sub_t;
struct iox2_port_factory_publisher_builder_t;
struct iox2_port_factory_subscriber_builder_t;
struct iox2_publisher_t;
struct iox2_subscriber_t;
struct iox2_sample_t;
struct iox2_sample_mut_t;

/* Handles. Opaque pointers, and a _ref is a const pointer to one. */
struct iox2_name_h_t;
typedef struct iox2_name_h_t* iox2_node_h;
typedef const iox2_node_h* iox2_node_h_ref;

struct iox2_node_builder_h_t;
typedef struct iox2_node_builder_h_t* iox2_node_builder_h;

struct iox2_service_name_h_t;
typedef struct iox2_service_name_h_t* iox2_service_name_h;

struct iox2_service_name_ptr_t;
typedef const struct iox2_service_name_ptr_t* iox2_service_name_ptr;

struct iox2_service_builder_h_t;
typedef struct iox2_service_builder_h_t* iox2_service_builder_h;

struct iox2_service_builder_pub_sub_h_t;
typedef struct iox2_service_builder_pub_sub_h_t* iox2_service_builder_pub_sub_h;
typedef const iox2_service_builder_pub_sub_h* iox2_service_builder_pub_sub_h_ref;

struct iox2_port_factory_pub_sub_h_t;
typedef struct iox2_port_factory_pub_sub_h_t* iox2_port_factory_pub_sub_h;
typedef const iox2_port_factory_pub_sub_h* iox2_port_factory_pub_sub_h_ref;

struct iox2_port_factory_publisher_builder_h_t;
typedef struct iox2_port_factory_publisher_builder_h_t* iox2_port_factory_publisher_builder_h;
typedef const iox2_port_factory_publisher_builder_h* iox2_port_factory_publisher_builder_h_ref;

struct iox2_port_factory_subscriber_builder_h_t;
typedef struct iox2_port_factory_subscriber_builder_h_t* iox2_port_factory_subscriber_builder_h;

struct iox2_publisher_h_t;
typedef struct iox2_publisher_h_t* iox2_publisher_h;
typedef const iox2_publisher_h* iox2_publisher_h_ref;

struct iox2_subscriber_h_t;
typedef struct iox2_subscriber_h_t* iox2_subscriber_h;
typedef const iox2_subscriber_h* iox2_subscriber_h_ref;

struct iox2_sample_h_t;
typedef struct iox2_sample_h_t* iox2_sample_h;
typedef const iox2_sample_h* iox2_sample_h_ref;

struct iox2_sample_mut_h_t;
typedef struct iox2_sample_mut_h_t* iox2_sample_mut_h;
typedef const iox2_sample_mut_h* iox2_sample_mut_h_ref;

/* Enums. A value here is part of the ABI, so each one is transcribed exactly. */
typedef enum iox2_service_type_e {
    iox2_service_type_e_LOCAL,
    iox2_service_type_e_IPC,
} iox2_service_type_e;

typedef enum iox2_type_variant_e {
    iox2_type_variant_e_FIXED_SIZE,
    iox2_type_variant_e_DYNAMIC,
} iox2_type_variant_e;

typedef enum iox2_log_level_e {
    iox2_log_level_e_TRACE = 0,
    iox2_log_level_e_DEBUG = 1,
    iox2_log_level_e_INFO = 2,
    iox2_log_level_e_WARN = 3,
    iox2_log_level_e_ERROR = 4,
    iox2_log_level_e_FATAL = 5,
} iox2_log_level_e;


/* ── The WaitSet ────────────────────────────────────────────────────────────
 *
 * One event loop for a process that has both a bus and a socket. `iox2_node_wait` is a
 * periodic sleep and cannot wake on a packet, so an edge that used it would still need a
 * second loop for the network. The WaitSet takes an arbitrary file descriptor, which is
 * what removes the second loop.
 *
 * Transcribed from iceoryx2-ffi/c/src/api at v0.9.3. Storage structs stay incomplete and
 * every call passes NULL for them, exactly as above.
 */
struct iox2_waitset_builder_t;
struct iox2_waitset_t;
struct iox2_waitset_guard_t;
struct iox2_file_descriptor_t;

typedef struct iox2_waitset_builder_h_t *iox2_waitset_builder_h;
typedef struct iox2_waitset_h_t *iox2_waitset_h;
typedef struct iox2_waitset_guard_h_t *iox2_waitset_guard_h;
typedef struct iox2_waitset_attachment_id_h_t *iox2_waitset_attachment_id_h;
typedef struct iox2_file_descriptor_h_t *iox2_file_descriptor_h;

/* A `_h_ref` is a pointer TO a handle and not a const handle. `waitset.rs` says
   `pub type iox2_waitset_h_ref = *const iox2_waitset_h`, and the C example passes
   `&waitset`. Every `_h_ref` above this block already has that shape, and writing these
   as const handles would have compiled and passed the wrong thing through a function
   pointer, which is a crash with no diagnostic. */
typedef const iox2_waitset_h *iox2_waitset_h_ref;
typedef const iox2_waitset_guard_h *iox2_waitset_guard_h_ref;
typedef const iox2_waitset_attachment_id_h *iox2_waitset_attachment_id_h_ref;

/* A file descriptor pointer is its own type rather than a `_h_ref`, and
   `iox2_cast_file_descriptor_ptr` produces it from a handle. */
typedef const struct iox2_file_descriptor_ptr_t *iox2_file_descriptor_ptr;

/* A context pointer the WaitSet hands back to the callback untouched. */
typedef void *iox2_callback_context;

/* Whether the WaitSet keeps delivering events this wakeup, or stops. */
typedef enum iox2_callback_progression_e {
    iox2_callback_progression_e_STOP = 0,
    iox2_callback_progression_e_CONTINUE,
} iox2_callback_progression_e;

/* Why `iox2_waitset_wait_and_process` returned. IOX2_OK is 0, and these follow it. */
typedef enum iox2_waitset_run_result_e {
    iox2_waitset_run_result_e_TERMINATION_REQUEST = 1,
    iox2_waitset_run_result_e_INTERRUPT,
    iox2_waitset_run_result_e_STOP_REQUEST,
    iox2_waitset_run_result_e_ALL_EVENTS_HANDLED,
} iox2_waitset_run_result_e;

/* Called once for each attachment that fired. The id says which one, and comparing it with
 * `iox2_waitset_attachment_id_has_event_from` against a guard is how a caller tells the UDP
 * socket from a timer. */
typedef iox2_callback_progression_e (*iox2_waitset_run_callback)(
    iox2_waitset_attachment_id_h, iox2_callback_context);

/* Success. Every int-returning call below returns this or an error code. */
#define IOX2_OK 0

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* WEFT_HARNESS_IOX2_DECLS_H */
