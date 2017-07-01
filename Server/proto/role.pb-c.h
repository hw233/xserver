/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_role_2eproto__INCLUDED
#define PROTOBUF_C_role_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS

#include "comm_message.pb-c.h"

typedef struct _PlayerAttrNotify PlayerAttrNotify;
typedef struct _PlayerNameNotify PlayerNameNotify;
typedef struct _PlayerRenameRequest PlayerRenameRequest;
typedef struct _HeadIconReplaceRequest HeadIconReplaceRequest;
typedef struct _HeadIconUnlockNotify HeadIconUnlockNotify;
typedef struct _HeadIconInfoAnswer HeadIconInfoAnswer;
typedef struct _HeadIconOldAnswer HeadIconOldAnswer;
typedef struct _SetFashion SetFashion;
typedef struct _SystemNoticeNotify SystemNoticeNotify;


/* --- enums --- */


/* --- messages --- */

struct  _PlayerAttrNotify
{
  ProtobufCMessage base;
  uint64_t player_id;
  size_t n_attrs;
  AttrData **attrs;
};
#define PLAYER_ATTR_NOTIFY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&player_attr_notify__descriptor) \
    , 0, 0,NULL }


struct  _PlayerNameNotify
{
  ProtobufCMessage base;
  uint64_t player_id;
  char *player_name;
};
#define PLAYER_NAME_NOTIFY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&player_name_notify__descriptor) \
    , 0, NULL }


struct  _PlayerRenameRequest
{
  ProtobufCMessage base;
  char *name;
};
#define PLAYER_RENAME_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&player_rename_request__descriptor) \
    , NULL }


struct  _HeadIconReplaceRequest
{
  ProtobufCMessage base;
  uint32_t icon_id;
};
#define HEAD_ICON_REPLACE_REQUEST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&head_icon_replace_request__descriptor) \
    , 0 }


struct  _HeadIconUnlockNotify
{
  ProtobufCMessage base;
  uint32_t icon_id;
};
#define HEAD_ICON_UNLOCK_NOTIFY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&head_icon_unlock_notify__descriptor) \
    , 0 }


struct  _HeadIconInfoAnswer
{
  ProtobufCMessage base;
  uint32_t result;
  size_t n_icon_list;
  HeadIconData **icon_list;
};
#define HEAD_ICON_INFO_ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&head_icon_info_answer__descriptor) \
    , 0, 0,NULL }


struct  _HeadIconOldAnswer
{
  ProtobufCMessage base;
  uint32_t result;
  uint32_t icon_id;
};
#define HEAD_ICON_OLD_ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&head_icon_old_answer__descriptor) \
    , 0, 0 }


struct  _SetFashion
{
  ProtobufCMessage base;
  uint32_t id;
  uint32_t vaual;
};
#define SET_FASHION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&set_fashion__descriptor) \
    , 0, 0 }


struct  _SystemNoticeNotify
{
  ProtobufCMessage base;
  uint32_t id;
  size_t n_args;
  char **args;
  protobuf_c_boolean has_targetid;
  uint64_t targetid;
};
#define SYSTEM_NOTICE_NOTIFY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&system_notice_notify__descriptor) \
    , 0, 0,NULL, 0,0 }


/* PlayerAttrNotify methods */
void   player_attr_notify__init
                     (PlayerAttrNotify         *message);
size_t player_attr_notify__get_packed_size
                     (const PlayerAttrNotify   *message);
size_t player_attr_notify__pack
                     (const PlayerAttrNotify   *message,
                      uint8_t             *out);
size_t player_attr_notify__pack_to_buffer
                     (const PlayerAttrNotify   *message,
                      ProtobufCBuffer     *buffer);
PlayerAttrNotify *
       player_attr_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   player_attr_notify__free_unpacked
                     (PlayerAttrNotify *message,
                      ProtobufCAllocator *allocator);
/* PlayerNameNotify methods */
void   player_name_notify__init
                     (PlayerNameNotify         *message);
size_t player_name_notify__get_packed_size
                     (const PlayerNameNotify   *message);
size_t player_name_notify__pack
                     (const PlayerNameNotify   *message,
                      uint8_t             *out);
size_t player_name_notify__pack_to_buffer
                     (const PlayerNameNotify   *message,
                      ProtobufCBuffer     *buffer);
PlayerNameNotify *
       player_name_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   player_name_notify__free_unpacked
                     (PlayerNameNotify *message,
                      ProtobufCAllocator *allocator);
/* PlayerRenameRequest methods */
void   player_rename_request__init
                     (PlayerRenameRequest         *message);
size_t player_rename_request__get_packed_size
                     (const PlayerRenameRequest   *message);
size_t player_rename_request__pack
                     (const PlayerRenameRequest   *message,
                      uint8_t             *out);
size_t player_rename_request__pack_to_buffer
                     (const PlayerRenameRequest   *message,
                      ProtobufCBuffer     *buffer);
PlayerRenameRequest *
       player_rename_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   player_rename_request__free_unpacked
                     (PlayerRenameRequest *message,
                      ProtobufCAllocator *allocator);
/* HeadIconReplaceRequest methods */
void   head_icon_replace_request__init
                     (HeadIconReplaceRequest         *message);
size_t head_icon_replace_request__get_packed_size
                     (const HeadIconReplaceRequest   *message);
size_t head_icon_replace_request__pack
                     (const HeadIconReplaceRequest   *message,
                      uint8_t             *out);
size_t head_icon_replace_request__pack_to_buffer
                     (const HeadIconReplaceRequest   *message,
                      ProtobufCBuffer     *buffer);
HeadIconReplaceRequest *
       head_icon_replace_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   head_icon_replace_request__free_unpacked
                     (HeadIconReplaceRequest *message,
                      ProtobufCAllocator *allocator);
/* HeadIconUnlockNotify methods */
void   head_icon_unlock_notify__init
                     (HeadIconUnlockNotify         *message);
size_t head_icon_unlock_notify__get_packed_size
                     (const HeadIconUnlockNotify   *message);
size_t head_icon_unlock_notify__pack
                     (const HeadIconUnlockNotify   *message,
                      uint8_t             *out);
size_t head_icon_unlock_notify__pack_to_buffer
                     (const HeadIconUnlockNotify   *message,
                      ProtobufCBuffer     *buffer);
HeadIconUnlockNotify *
       head_icon_unlock_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   head_icon_unlock_notify__free_unpacked
                     (HeadIconUnlockNotify *message,
                      ProtobufCAllocator *allocator);
/* HeadIconInfoAnswer methods */
void   head_icon_info_answer__init
                     (HeadIconInfoAnswer         *message);
size_t head_icon_info_answer__get_packed_size
                     (const HeadIconInfoAnswer   *message);
size_t head_icon_info_answer__pack
                     (const HeadIconInfoAnswer   *message,
                      uint8_t             *out);
size_t head_icon_info_answer__pack_to_buffer
                     (const HeadIconInfoAnswer   *message,
                      ProtobufCBuffer     *buffer);
HeadIconInfoAnswer *
       head_icon_info_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   head_icon_info_answer__free_unpacked
                     (HeadIconInfoAnswer *message,
                      ProtobufCAllocator *allocator);
/* HeadIconOldAnswer methods */
void   head_icon_old_answer__init
                     (HeadIconOldAnswer         *message);
size_t head_icon_old_answer__get_packed_size
                     (const HeadIconOldAnswer   *message);
size_t head_icon_old_answer__pack
                     (const HeadIconOldAnswer   *message,
                      uint8_t             *out);
size_t head_icon_old_answer__pack_to_buffer
                     (const HeadIconOldAnswer   *message,
                      ProtobufCBuffer     *buffer);
HeadIconOldAnswer *
       head_icon_old_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   head_icon_old_answer__free_unpacked
                     (HeadIconOldAnswer *message,
                      ProtobufCAllocator *allocator);
/* SetFashion methods */
void   set_fashion__init
                     (SetFashion         *message);
size_t set_fashion__get_packed_size
                     (const SetFashion   *message);
size_t set_fashion__pack
                     (const SetFashion   *message,
                      uint8_t             *out);
size_t set_fashion__pack_to_buffer
                     (const SetFashion   *message,
                      ProtobufCBuffer     *buffer);
SetFashion *
       set_fashion__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   set_fashion__free_unpacked
                     (SetFashion *message,
                      ProtobufCAllocator *allocator);
/* SystemNoticeNotify methods */
void   system_notice_notify__init
                     (SystemNoticeNotify         *message);
size_t system_notice_notify__get_packed_size
                     (const SystemNoticeNotify   *message);
size_t system_notice_notify__pack
                     (const SystemNoticeNotify   *message,
                      uint8_t             *out);
size_t system_notice_notify__pack_to_buffer
                     (const SystemNoticeNotify   *message,
                      ProtobufCBuffer     *buffer);
SystemNoticeNotify *
       system_notice_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   system_notice_notify__free_unpacked
                     (SystemNoticeNotify *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*PlayerAttrNotify_Closure)
                 (const PlayerAttrNotify *message,
                  void *closure_data);
typedef void (*PlayerNameNotify_Closure)
                 (const PlayerNameNotify *message,
                  void *closure_data);
typedef void (*PlayerRenameRequest_Closure)
                 (const PlayerRenameRequest *message,
                  void *closure_data);
typedef void (*HeadIconReplaceRequest_Closure)
                 (const HeadIconReplaceRequest *message,
                  void *closure_data);
typedef void (*HeadIconUnlockNotify_Closure)
                 (const HeadIconUnlockNotify *message,
                  void *closure_data);
typedef void (*HeadIconInfoAnswer_Closure)
                 (const HeadIconInfoAnswer *message,
                  void *closure_data);
typedef void (*HeadIconOldAnswer_Closure)
                 (const HeadIconOldAnswer *message,
                  void *closure_data);
typedef void (*SetFashion_Closure)
                 (const SetFashion *message,
                  void *closure_data);
typedef void (*SystemNoticeNotify_Closure)
                 (const SystemNoticeNotify *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor player_attr_notify__descriptor;
extern const ProtobufCMessageDescriptor player_name_notify__descriptor;
extern const ProtobufCMessageDescriptor player_rename_request__descriptor;
extern const ProtobufCMessageDescriptor head_icon_replace_request__descriptor;
extern const ProtobufCMessageDescriptor head_icon_unlock_notify__descriptor;
extern const ProtobufCMessageDescriptor head_icon_info_answer__descriptor;
extern const ProtobufCMessageDescriptor head_icon_old_answer__descriptor;
extern const ProtobufCMessageDescriptor set_fashion__descriptor;
extern const ProtobufCMessageDescriptor system_notice_notify__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_role_2eproto__INCLUDED */
