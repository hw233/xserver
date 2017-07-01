/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_scene_5ftransfer_2eproto__INCLUDED
#define PROTOBUF_C_scene_5ftransfer_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _SceneTransferRequest SceneTransferRequest;
typedef struct _SceneTransferAnswer SceneTransferAnswer;
typedef struct _TransferToPlayerSceneRequest TransferToPlayerSceneRequest;


/* --- enums --- */


/* --- messages --- */

struct  _SceneTransferRequest
{
  ProtobufCMessage base;
  uint32_t transfer_id;
  uint32_t type;
};
#define SCENE_TRANSFER_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&scene_transfer_request__descriptor) \
    , 0, 0 }


struct  _SceneTransferAnswer
{
  ProtobufCMessage base;
  int32_t result;
  uint32_t new_scene_id;
  float pos_x;
  float pos_z;
  float direct;
  float pos_y;
};
#define SCENE_TRANSFER_ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&scene_transfer_answer__descriptor) \
    , 0, 0, 0, 0, 0, 0 }


struct  _TransferToPlayerSceneRequest
{
  ProtobufCMessage base;
  uint64_t player_id;
};
#define TRANSFER_TO_PLAYER_SCENE_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&transfer_to_player_scene_request__descriptor) \
    , 0 }


/* SceneTransferRequest methods */
void   scene_transfer_request__init
                     (SceneTransferRequest         *message);
size_t scene_transfer_request__get_packed_size
                     (const SceneTransferRequest   *message);
size_t scene_transfer_request__pack
                     (const SceneTransferRequest   *message,
                      uint8_t             *out);
size_t scene_transfer_request__pack_to_buffer
                     (const SceneTransferRequest   *message,
                      ProtobufCBuffer     *buffer);
SceneTransferRequest *
       scene_transfer_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   scene_transfer_request__free_unpacked
                     (SceneTransferRequest *message,
                      ProtobufCAllocator *allocator);
/* SceneTransferAnswer methods */
void   scene_transfer_answer__init
                     (SceneTransferAnswer         *message);
size_t scene_transfer_answer__get_packed_size
                     (const SceneTransferAnswer   *message);
size_t scene_transfer_answer__pack
                     (const SceneTransferAnswer   *message,
                      uint8_t             *out);
size_t scene_transfer_answer__pack_to_buffer
                     (const SceneTransferAnswer   *message,
                      ProtobufCBuffer     *buffer);
SceneTransferAnswer *
       scene_transfer_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   scene_transfer_answer__free_unpacked
                     (SceneTransferAnswer *message,
                      ProtobufCAllocator *allocator);
/* TransferToPlayerSceneRequest methods */
void   transfer_to_player_scene_request__init
                     (TransferToPlayerSceneRequest         *message);
size_t transfer_to_player_scene_request__get_packed_size
                     (const TransferToPlayerSceneRequest   *message);
size_t transfer_to_player_scene_request__pack
                     (const TransferToPlayerSceneRequest   *message,
                      uint8_t             *out);
size_t transfer_to_player_scene_request__pack_to_buffer
                     (const TransferToPlayerSceneRequest   *message,
                      ProtobufCBuffer     *buffer);
TransferToPlayerSceneRequest *
       transfer_to_player_scene_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   transfer_to_player_scene_request__free_unpacked
                     (TransferToPlayerSceneRequest *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*SceneTransferRequest_Closure)
                 (const SceneTransferRequest *message,
                  void *closure_data);
typedef void (*SceneTransferAnswer_Closure)
                 (const SceneTransferAnswer *message,
                  void *closure_data);
typedef void (*TransferToPlayerSceneRequest_Closure)
                 (const TransferToPlayerSceneRequest *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor scene_transfer_request__descriptor;
extern const ProtobufCMessageDescriptor scene_transfer_answer__descriptor;
extern const ProtobufCMessageDescriptor transfer_to_player_scene_request__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_scene_5ftransfer_2eproto__INCLUDED */
