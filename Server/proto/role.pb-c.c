/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "role.pb-c.h"
void   player_attr_notify__init
                     (PlayerAttrNotify         *message)
{
  static PlayerAttrNotify init_value = PLAYER_ATTR_NOTIFY__INIT;
  *message = init_value;
}
size_t player_attr_notify__get_packed_size
                     (const PlayerAttrNotify *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_attr_notify__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t player_attr_notify__pack
                     (const PlayerAttrNotify *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_attr_notify__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t player_attr_notify__pack_to_buffer
                     (const PlayerAttrNotify *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_attr_notify__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
PlayerAttrNotify *
       player_attr_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (PlayerAttrNotify *)
     protobuf_c_message_unpack (&player_attr_notify__descriptor,
                                allocator, len, data);
}
void   player_attr_notify__free_unpacked
                     (PlayerAttrNotify *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_attr_notify__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   player_name_notify__init
                     (PlayerNameNotify         *message)
{
  static PlayerNameNotify init_value = PLAYER_NAME_NOTIFY__INIT;
  *message = init_value;
}
size_t player_name_notify__get_packed_size
                     (const PlayerNameNotify *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_name_notify__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t player_name_notify__pack
                     (const PlayerNameNotify *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_name_notify__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t player_name_notify__pack_to_buffer
                     (const PlayerNameNotify *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_name_notify__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
PlayerNameNotify *
       player_name_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (PlayerNameNotify *)
     protobuf_c_message_unpack (&player_name_notify__descriptor,
                                allocator, len, data);
}
void   player_name_notify__free_unpacked
                     (PlayerNameNotify *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_name_notify__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   player_rename_request__init
                     (PlayerRenameRequest         *message)
{
  static PlayerRenameRequest init_value = PLAYER_RENAME_REQUEST__INIT;
  *message = init_value;
}
size_t player_rename_request__get_packed_size
                     (const PlayerRenameRequest *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_rename_request__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t player_rename_request__pack
                     (const PlayerRenameRequest *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_rename_request__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t player_rename_request__pack_to_buffer
                     (const PlayerRenameRequest *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_rename_request__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
PlayerRenameRequest *
       player_rename_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (PlayerRenameRequest *)
     protobuf_c_message_unpack (&player_rename_request__descriptor,
                                allocator, len, data);
}
void   player_rename_request__free_unpacked
                     (PlayerRenameRequest *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &player_rename_request__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   head_icon_replace_request__init
                     (HeadIconReplaceRequest         *message)
{
  static HeadIconReplaceRequest init_value = HEAD_ICON_REPLACE_REQUEST__INIT;
  *message = init_value;
}
size_t head_icon_replace_request__get_packed_size
                     (const HeadIconReplaceRequest *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_replace_request__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t head_icon_replace_request__pack
                     (const HeadIconReplaceRequest *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_replace_request__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t head_icon_replace_request__pack_to_buffer
                     (const HeadIconReplaceRequest *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_replace_request__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
HeadIconReplaceRequest *
       head_icon_replace_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (HeadIconReplaceRequest *)
     protobuf_c_message_unpack (&head_icon_replace_request__descriptor,
                                allocator, len, data);
}
void   head_icon_replace_request__free_unpacked
                     (HeadIconReplaceRequest *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_replace_request__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   head_icon_unlock_notify__init
                     (HeadIconUnlockNotify         *message)
{
  static HeadIconUnlockNotify init_value = HEAD_ICON_UNLOCK_NOTIFY__INIT;
  *message = init_value;
}
size_t head_icon_unlock_notify__get_packed_size
                     (const HeadIconUnlockNotify *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_unlock_notify__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t head_icon_unlock_notify__pack
                     (const HeadIconUnlockNotify *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_unlock_notify__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t head_icon_unlock_notify__pack_to_buffer
                     (const HeadIconUnlockNotify *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_unlock_notify__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
HeadIconUnlockNotify *
       head_icon_unlock_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (HeadIconUnlockNotify *)
     protobuf_c_message_unpack (&head_icon_unlock_notify__descriptor,
                                allocator, len, data);
}
void   head_icon_unlock_notify__free_unpacked
                     (HeadIconUnlockNotify *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_unlock_notify__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   head_icon_info_answer__init
                     (HeadIconInfoAnswer         *message)
{
  static HeadIconInfoAnswer init_value = HEAD_ICON_INFO_ANSWER__INIT;
  *message = init_value;
}
size_t head_icon_info_answer__get_packed_size
                     (const HeadIconInfoAnswer *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_info_answer__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t head_icon_info_answer__pack
                     (const HeadIconInfoAnswer *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_info_answer__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t head_icon_info_answer__pack_to_buffer
                     (const HeadIconInfoAnswer *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_info_answer__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
HeadIconInfoAnswer *
       head_icon_info_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (HeadIconInfoAnswer *)
     protobuf_c_message_unpack (&head_icon_info_answer__descriptor,
                                allocator, len, data);
}
void   head_icon_info_answer__free_unpacked
                     (HeadIconInfoAnswer *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_info_answer__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   head_icon_old_answer__init
                     (HeadIconOldAnswer         *message)
{
  static HeadIconOldAnswer init_value = HEAD_ICON_OLD_ANSWER__INIT;
  *message = init_value;
}
size_t head_icon_old_answer__get_packed_size
                     (const HeadIconOldAnswer *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_old_answer__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t head_icon_old_answer__pack
                     (const HeadIconOldAnswer *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_old_answer__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t head_icon_old_answer__pack_to_buffer
                     (const HeadIconOldAnswer *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_old_answer__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
HeadIconOldAnswer *
       head_icon_old_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (HeadIconOldAnswer *)
     protobuf_c_message_unpack (&head_icon_old_answer__descriptor,
                                allocator, len, data);
}
void   head_icon_old_answer__free_unpacked
                     (HeadIconOldAnswer *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &head_icon_old_answer__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   set_fashion__init
                     (SetFashion         *message)
{
  static SetFashion init_value = SET_FASHION__INIT;
  *message = init_value;
}
size_t set_fashion__get_packed_size
                     (const SetFashion *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &set_fashion__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t set_fashion__pack
                     (const SetFashion *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &set_fashion__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t set_fashion__pack_to_buffer
                     (const SetFashion *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &set_fashion__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
SetFashion *
       set_fashion__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (SetFashion *)
     protobuf_c_message_unpack (&set_fashion__descriptor,
                                allocator, len, data);
}
void   set_fashion__free_unpacked
                     (SetFashion *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &set_fashion__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   system_notice_notify__init
                     (SystemNoticeNotify         *message)
{
  static SystemNoticeNotify init_value = SYSTEM_NOTICE_NOTIFY__INIT;
  *message = init_value;
}
size_t system_notice_notify__get_packed_size
                     (const SystemNoticeNotify *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &system_notice_notify__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t system_notice_notify__pack
                     (const SystemNoticeNotify *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &system_notice_notify__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t system_notice_notify__pack_to_buffer
                     (const SystemNoticeNotify *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &system_notice_notify__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
SystemNoticeNotify *
       system_notice_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (SystemNoticeNotify *)
     protobuf_c_message_unpack (&system_notice_notify__descriptor,
                                allocator, len, data);
}
void   system_notice_notify__free_unpacked
                     (SystemNoticeNotify *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &system_notice_notify__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor player_attr_notify__field_descriptors[2] =
{
  {
    "player_id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(PlayerAttrNotify, player_id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "attrs",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    PROTOBUF_C_OFFSETOF(PlayerAttrNotify, n_attrs),
    PROTOBUF_C_OFFSETOF(PlayerAttrNotify, attrs),
    &attr_data__descriptor,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned player_attr_notify__field_indices_by_name[] = {
  1,   /* field[1] = attrs */
  0,   /* field[0] = player_id */
};
static const ProtobufCIntRange player_attr_notify__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor player_attr_notify__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "PlayerAttrNotify",
  "PlayerAttrNotify",
  "PlayerAttrNotify",
  "",
  sizeof(PlayerAttrNotify),
  2,
  player_attr_notify__field_descriptors,
  player_attr_notify__field_indices_by_name,
  1,  player_attr_notify__number_ranges,
  (ProtobufCMessageInit) player_attr_notify__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor player_name_notify__field_descriptors[2] =
{
  {
    "player_id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(PlayerNameNotify, player_id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "player_name",
    2,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(PlayerNameNotify, player_name),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned player_name_notify__field_indices_by_name[] = {
  0,   /* field[0] = player_id */
  1,   /* field[1] = player_name */
};
static const ProtobufCIntRange player_name_notify__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor player_name_notify__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "PlayerNameNotify",
  "PlayerNameNotify",
  "PlayerNameNotify",
  "",
  sizeof(PlayerNameNotify),
  2,
  player_name_notify__field_descriptors,
  player_name_notify__field_indices_by_name,
  1,  player_name_notify__number_ranges,
  (ProtobufCMessageInit) player_name_notify__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor player_rename_request__field_descriptors[1] =
{
  {
    "name",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(PlayerRenameRequest, name),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned player_rename_request__field_indices_by_name[] = {
  0,   /* field[0] = name */
};
static const ProtobufCIntRange player_rename_request__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor player_rename_request__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "PlayerRenameRequest",
  "PlayerRenameRequest",
  "PlayerRenameRequest",
  "",
  sizeof(PlayerRenameRequest),
  1,
  player_rename_request__field_descriptors,
  player_rename_request__field_indices_by_name,
  1,  player_rename_request__number_ranges,
  (ProtobufCMessageInit) player_rename_request__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor head_icon_replace_request__field_descriptors[1] =
{
  {
    "icon_id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(HeadIconReplaceRequest, icon_id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned head_icon_replace_request__field_indices_by_name[] = {
  0,   /* field[0] = icon_id */
};
static const ProtobufCIntRange head_icon_replace_request__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor head_icon_replace_request__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "HeadIconReplaceRequest",
  "HeadIconReplaceRequest",
  "HeadIconReplaceRequest",
  "",
  sizeof(HeadIconReplaceRequest),
  1,
  head_icon_replace_request__field_descriptors,
  head_icon_replace_request__field_indices_by_name,
  1,  head_icon_replace_request__number_ranges,
  (ProtobufCMessageInit) head_icon_replace_request__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor head_icon_unlock_notify__field_descriptors[1] =
{
  {
    "icon_id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(HeadIconUnlockNotify, icon_id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned head_icon_unlock_notify__field_indices_by_name[] = {
  0,   /* field[0] = icon_id */
};
static const ProtobufCIntRange head_icon_unlock_notify__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor head_icon_unlock_notify__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "HeadIconUnlockNotify",
  "HeadIconUnlockNotify",
  "HeadIconUnlockNotify",
  "",
  sizeof(HeadIconUnlockNotify),
  1,
  head_icon_unlock_notify__field_descriptors,
  head_icon_unlock_notify__field_indices_by_name,
  1,  head_icon_unlock_notify__number_ranges,
  (ProtobufCMessageInit) head_icon_unlock_notify__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor head_icon_info_answer__field_descriptors[2] =
{
  {
    "result",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(HeadIconInfoAnswer, result),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "icon_list",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    PROTOBUF_C_OFFSETOF(HeadIconInfoAnswer, n_icon_list),
    PROTOBUF_C_OFFSETOF(HeadIconInfoAnswer, icon_list),
    &head_icon_data__descriptor,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned head_icon_info_answer__field_indices_by_name[] = {
  1,   /* field[1] = icon_list */
  0,   /* field[0] = result */
};
static const ProtobufCIntRange head_icon_info_answer__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor head_icon_info_answer__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "HeadIconInfoAnswer",
  "HeadIconInfoAnswer",
  "HeadIconInfoAnswer",
  "",
  sizeof(HeadIconInfoAnswer),
  2,
  head_icon_info_answer__field_descriptors,
  head_icon_info_answer__field_indices_by_name,
  1,  head_icon_info_answer__number_ranges,
  (ProtobufCMessageInit) head_icon_info_answer__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor head_icon_old_answer__field_descriptors[2] =
{
  {
    "result",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(HeadIconOldAnswer, result),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "icon_id",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(HeadIconOldAnswer, icon_id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned head_icon_old_answer__field_indices_by_name[] = {
  1,   /* field[1] = icon_id */
  0,   /* field[0] = result */
};
static const ProtobufCIntRange head_icon_old_answer__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor head_icon_old_answer__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "HeadIconOldAnswer",
  "HeadIconOldAnswer",
  "HeadIconOldAnswer",
  "",
  sizeof(HeadIconOldAnswer),
  2,
  head_icon_old_answer__field_descriptors,
  head_icon_old_answer__field_indices_by_name,
  1,  head_icon_old_answer__number_ranges,
  (ProtobufCMessageInit) head_icon_old_answer__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor set_fashion__field_descriptors[2] =
{
  {
    "id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(SetFashion, id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "vaual",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(SetFashion, vaual),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned set_fashion__field_indices_by_name[] = {
  0,   /* field[0] = id */
  1,   /* field[1] = vaual */
};
static const ProtobufCIntRange set_fashion__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor set_fashion__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "SetFashion",
  "SetFashion",
  "SetFashion",
  "",
  sizeof(SetFashion),
  2,
  set_fashion__field_descriptors,
  set_fashion__field_indices_by_name,
  1,  set_fashion__number_ranges,
  (ProtobufCMessageInit) set_fashion__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor system_notice_notify__field_descriptors[3] =
{
  {
    "id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(SystemNoticeNotify, id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "args",
    2,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_STRING,
    PROTOBUF_C_OFFSETOF(SystemNoticeNotify, n_args),
    PROTOBUF_C_OFFSETOF(SystemNoticeNotify, args),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "targetId",
    3,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT64,
    PROTOBUF_C_OFFSETOF(SystemNoticeNotify, has_targetid),
    PROTOBUF_C_OFFSETOF(SystemNoticeNotify, targetid),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned system_notice_notify__field_indices_by_name[] = {
  1,   /* field[1] = args */
  0,   /* field[0] = id */
  2,   /* field[2] = targetId */
};
static const ProtobufCIntRange system_notice_notify__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor system_notice_notify__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "SystemNoticeNotify",
  "SystemNoticeNotify",
  "SystemNoticeNotify",
  "",
  sizeof(SystemNoticeNotify),
  3,
  system_notice_notify__field_descriptors,
  system_notice_notify__field_indices_by_name,
  1,  system_notice_notify__number_ranges,
  (ProtobufCMessageInit) system_notice_notify__init,
  NULL,NULL,NULL    /* reserved[123] */
};
