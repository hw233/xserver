/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "chat.pb-c.h"
void   chat__init
                     (Chat         *message)
{
  static Chat init_value = CHAT__INIT;
  *message = init_value;
}
size_t chat__get_packed_size
                     (const Chat *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t chat__pack
                     (const Chat *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t chat__pack_to_buffer
                     (const Chat *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Chat *
       chat__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Chat *)
     protobuf_c_message_unpack (&chat__descriptor,
                                allocator, len, data);
}
void   chat__free_unpacked
                     (Chat *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   chat_horse__init
                     (ChatHorse         *message)
{
  static ChatHorse init_value = CHAT_HORSE__INIT;
  *message = init_value;
}
size_t chat_horse__get_packed_size
                     (const ChatHorse *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat_horse__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t chat_horse__pack
                     (const ChatHorse *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat_horse__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t chat_horse__pack_to_buffer
                     (const ChatHorse *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat_horse__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
ChatHorse *
       chat_horse__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (ChatHorse *)
     protobuf_c_message_unpack (&chat_horse__descriptor,
                                allocator, len, data);
}
void   chat_horse__free_unpacked
                     (ChatHorse *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &chat_horse__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   ans_chat__init
                     (AnsChat         *message)
{
  static AnsChat init_value = ANS_CHAT__INIT;
  *message = init_value;
}
size_t ans_chat__get_packed_size
                     (const AnsChat *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &ans_chat__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t ans_chat__pack
                     (const AnsChat *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &ans_chat__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t ans_chat__pack_to_buffer
                     (const AnsChat *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &ans_chat__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
AnsChat *
       ans_chat__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (AnsChat *)
     protobuf_c_message_unpack (&ans_chat__descriptor,
                                allocator, len, data);
}
void   ans_chat__free_unpacked
                     (AnsChat *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &ans_chat__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor chat__field_descriptors[14] =
{
  {
    "channel",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, channel),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "contain",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, contain),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendName",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, sendname),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendPlayerId",
    4,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, sendplayerid),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendPlayerLv",
    5,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, sendplayerlv),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendPlayerJob",
    6,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, sendplayerjob),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "recvPlayerId",
    7,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT64,
    PROTOBUF_C_OFFSETOF(Chat, has_recvplayerid),
    PROTOBUF_C_OFFSETOF(Chat, recvplayerid),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "guild",
    8,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, guild),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendPlayerPicture",
    9,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT32,
    PROTOBUF_C_OFFSETOF(Chat, has_sendplayerpicture),
    PROTOBUF_C_OFFSETOF(Chat, sendplayerpicture),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "costtype",
    10,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT32,
    PROTOBUF_C_OFFSETOF(Chat, has_costtype),
    PROTOBUF_C_OFFSETOF(Chat, costtype),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "url",
    11,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, url),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "time_stamp",
    12,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT32,
    PROTOBUF_C_OFFSETOF(Chat, has_time_stamp),
    PROTOBUF_C_OFFSETOF(Chat, time_stamp),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "tanslation",
    13,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(Chat, tanslation),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "sendTime",
    14,
    PROTOBUF_C_LABEL_OPTIONAL,
    PROTOBUF_C_TYPE_UINT32,
    PROTOBUF_C_OFFSETOF(Chat, has_sendtime),
    PROTOBUF_C_OFFSETOF(Chat, sendtime),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned chat__field_indices_by_name[] = {
  0,   /* field[0] = channel */
  1,   /* field[1] = contain */
  9,   /* field[9] = costtype */
  7,   /* field[7] = guild */
  6,   /* field[6] = recvPlayerId */
  2,   /* field[2] = sendName */
  3,   /* field[3] = sendPlayerId */
  5,   /* field[5] = sendPlayerJob */
  4,   /* field[4] = sendPlayerLv */
  8,   /* field[8] = sendPlayerPicture */
  13,   /* field[13] = sendTime */
  12,   /* field[12] = tanslation */
  11,   /* field[11] = time_stamp */
  10,   /* field[10] = url */
};
static const ProtobufCIntRange chat__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 14 }
};
const ProtobufCMessageDescriptor chat__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "Chat",
  "Chat",
  "Chat",
  "",
  sizeof(Chat),
  14,
  chat__field_descriptors,
  chat__field_indices_by_name,
  1,  chat__number_ranges,
  (ProtobufCMessageInit) chat__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor chat_horse__field_descriptors[3] =
{
  {
    "id",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(ChatHorse, id),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "prior",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(ChatHorse, prior),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "content",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(ChatHorse, content),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned chat_horse__field_indices_by_name[] = {
  2,   /* field[2] = content */
  0,   /* field[0] = id */
  1,   /* field[1] = prior */
};
static const ProtobufCIntRange chat_horse__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor chat_horse__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "ChatHorse",
  "ChatHorse",
  "ChatHorse",
  "",
  sizeof(ChatHorse),
  3,
  chat_horse__field_descriptors,
  chat_horse__field_indices_by_name,
  1,  chat_horse__number_ranges,
  (ProtobufCMessageInit) chat_horse__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor ans_chat__field_descriptors[2] =
{
  {
    "ret",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(AnsChat, ret),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "cd",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(AnsChat, cd),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned ans_chat__field_indices_by_name[] = {
  1,   /* field[1] = cd */
  0,   /* field[0] = ret */
};
static const ProtobufCIntRange ans_chat__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor ans_chat__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "AnsChat",
  "AnsChat",
  "AnsChat",
  "",
  sizeof(AnsChat),
  2,
  ans_chat__field_descriptors,
  ans_chat__field_indices_by_name,
  1,  ans_chat__number_ranges,
  (ProtobufCMessageInit) ans_chat__init,
  NULL,NULL,NULL    /* reserved[123] */
};
const ProtobufCEnumValue channel__enum_values_by_number[9] =
{
  { "world", "CHANNEL__WORLD", 1 },
  { "zhaomu", "CHANNEL__ZHAOMU", 2 },
  { "private", "CHANNEL__PRIVATE", 3 },
  { "team", "CHANNEL__TEAM", 4 },
  { "family", "CHANNEL__FAMILY", 5 },
  { "menpai", "CHANNEL__MENPAI", 6 },
  { "area", "CHANNEL__AREA", 7 },
  { "system", "CHANNEL__SYSTEM", 8 },
  { "group", "CHANNEL__GROUP", 9 },
};
static const ProtobufCIntRange channel__value_ranges[] = {
{1, 0},{0, 9}
};
const ProtobufCEnumValueIndex channel__enum_values_by_name[9] =
{
  { "area", 6 },
  { "family", 4 },
  { "group", 8 },
  { "menpai", 5 },
  { "private", 2 },
  { "system", 7 },
  { "team", 3 },
  { "world", 0 },
  { "zhaomu", 1 },
};
const ProtobufCEnumDescriptor channel__descriptor =
{
  PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC,
  "Channel",
  "Channel",
  "Channel",
  "",
  9,
  channel__enum_values_by_number,
  9,
  channel__enum_values_by_name,
  1,
  channel__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
const ProtobufCEnumValue char_ret_code__enum_values_by_number[4] =
{
  { "success", "CHAR_RET_CODE__SUCCESS", 0 },
  { "offLine", "CHAR_RET_CODE__OFFLINE", 1 },
  { "noTeam", "CHAR_RET_CODE__NOTEAM", 2 },
  { "noGuild", "CHAR_RET_CODE__NOGUILD", 3 },
};
static const ProtobufCIntRange char_ret_code__value_ranges[] = {
{0, 0},{0, 4}
};
const ProtobufCEnumValueIndex char_ret_code__enum_values_by_name[4] =
{
  { "noGuild", 3 },
  { "noTeam", 2 },
  { "offLine", 1 },
  { "success", 0 },
};
const ProtobufCEnumDescriptor char_ret_code__descriptor =
{
  PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC,
  "CharRetCode",
  "CharRetCode",
  "CharRetCode",
  "",
  4,
  char_ret_code__enum_values_by_number,
  4,
  char_ret_code__enum_values_by_name,
  1,
  char_ret_code__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
const ProtobufCEnumValue chat_broadcast_type__enum_values_by_number[2] =
{
  { "ITEM", "CHAT_BROADCAST_TYPE__ITEM", 1 },
  { "MONEY", "CHAT_BROADCAST_TYPE__MONEY", 2 },
};
static const ProtobufCIntRange chat_broadcast_type__value_ranges[] = {
{1, 0},{0, 2}
};
const ProtobufCEnumValueIndex chat_broadcast_type__enum_values_by_name[2] =
{
  { "ITEM", 0 },
  { "MONEY", 1 },
};
const ProtobufCEnumDescriptor chat_broadcast_type__descriptor =
{
  PROTOBUF_C_ENUM_DESCRIPTOR_MAGIC,
  "ChatBroadcastType",
  "ChatBroadcastType",
  "ChatBroadcastType",
  "",
  2,
  chat_broadcast_type__enum_values_by_number,
  2,
  chat_broadcast_type__enum_values_by_name,
  1,
  chat_broadcast_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
