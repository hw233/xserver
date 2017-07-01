/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_relive_2eproto__INCLUDED
#define PROTOBUF_C_relive_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _ReliveRequest ReliveRequest;
typedef struct _ReliveNotify ReliveNotify;


/* --- enums --- */


/* --- messages --- */

struct  _ReliveRequest
{
  ProtobufCMessage base;
  uint32_t type;
};
#define RELIVE_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&relive_request__descriptor) \
    , 0 }


struct  _ReliveNotify
{
  ProtobufCMessage base;
  uint64_t playerid;
  uint32_t type;
  int32_t pos_x;
  int32_t pos_z;
  int32_t direct;
};
#define RELIVE_NOTIFY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&relive_notify__descriptor) \
    , 0, 0, 0, 0, 0 }


/* ReliveRequest methods */
void   relive_request__init
                     (ReliveRequest         *message);
size_t relive_request__get_packed_size
                     (const ReliveRequest   *message);
size_t relive_request__pack
                     (const ReliveRequest   *message,
                      uint8_t             *out);
size_t relive_request__pack_to_buffer
                     (const ReliveRequest   *message,
                      ProtobufCBuffer     *buffer);
ReliveRequest *
       relive_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   relive_request__free_unpacked
                     (ReliveRequest *message,
                      ProtobufCAllocator *allocator);
/* ReliveNotify methods */
void   relive_notify__init
                     (ReliveNotify         *message);
size_t relive_notify__get_packed_size
                     (const ReliveNotify   *message);
size_t relive_notify__pack
                     (const ReliveNotify   *message,
                      uint8_t             *out);
size_t relive_notify__pack_to_buffer
                     (const ReliveNotify   *message,
                      ProtobufCBuffer     *buffer);
ReliveNotify *
       relive_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   relive_notify__free_unpacked
                     (ReliveNotify *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*ReliveRequest_Closure)
                 (const ReliveRequest *message,
                  void *closure_data);
typedef void (*ReliveNotify_Closure)
                 (const ReliveNotify *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor relive_request__descriptor;
extern const ProtobufCMessageDescriptor relive_notify__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_relive_2eproto__INCLUDED */
