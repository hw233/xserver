/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C_NO_DEPRECATED
#define PROTOBUF_C_NO_DEPRECATED
#endif

#include "gift_package.pb-c.h"
void   gift_comm_notify__init
                     (GiftCommNotify         *message)
{
  static GiftCommNotify init_value = GIFT_COMM_NOTIFY__INIT;
  *message = init_value;
}
size_t gift_comm_notify__get_packed_size
                     (const GiftCommNotify *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_comm_notify__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t gift_comm_notify__pack
                     (const GiftCommNotify *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_comm_notify__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t gift_comm_notify__pack_to_buffer
                     (const GiftCommNotify *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_comm_notify__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
GiftCommNotify *
       gift_comm_notify__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (GiftCommNotify *)
     protobuf_c_message_unpack (&gift_comm_notify__descriptor,
                                allocator, len, data);
}
void   gift_comm_notify__free_unpacked
                     (GiftCommNotify *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_comm_notify__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   gift_receive_answer__init
                     (GiftReceiveAnswer         *message)
{
  static GiftReceiveAnswer init_value = GIFT_RECEIVE_ANSWER__INIT;
  *message = init_value;
}
size_t gift_receive_answer__get_packed_size
                     (const GiftReceiveAnswer *message)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_receive_answer__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t gift_receive_answer__pack
                     (const GiftReceiveAnswer *message,
                      uint8_t       *out)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_receive_answer__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t gift_receive_answer__pack_to_buffer
                     (const GiftReceiveAnswer *message,
                      ProtobufCBuffer *buffer)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_receive_answer__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
GiftReceiveAnswer *
       gift_receive_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (GiftReceiveAnswer *)
     protobuf_c_message_unpack (&gift_receive_answer__descriptor,
                                allocator, len, data);
}
void   gift_receive_answer__free_unpacked
                     (GiftReceiveAnswer *message,
                      ProtobufCAllocator *allocator)
{
  PROTOBUF_C_ASSERT (message->base.descriptor == &gift_receive_answer__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor gift_comm_notify__field_descriptors[1] =
{
  {
    "gift_type",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(GiftCommNotify, gift_type),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned gift_comm_notify__field_indices_by_name[] = {
  0,   /* field[0] = gift_type */
};
static const ProtobufCIntRange gift_comm_notify__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 1 }
};
const ProtobufCMessageDescriptor gift_comm_notify__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "GiftCommNotify",
  "GiftCommNotify",
  "GiftCommNotify",
  "",
  sizeof(GiftCommNotify),
  1,
  gift_comm_notify__field_descriptors,
  gift_comm_notify__field_indices_by_name,
  1,  gift_comm_notify__number_ranges,
  (ProtobufCMessageInit) gift_comm_notify__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor gift_receive_answer__field_descriptors[2] =
{
  {
    "result",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT32,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(GiftReceiveAnswer, result),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "gift_type",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_UINT64,
    0,   /* quantifier_offset */
    PROTOBUF_C_OFFSETOF(GiftReceiveAnswer, gift_type),
    NULL,
    NULL,
    0,            /* packed */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned gift_receive_answer__field_indices_by_name[] = {
  1,   /* field[1] = gift_type */
  0,   /* field[0] = result */
};
static const ProtobufCIntRange gift_receive_answer__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor gift_receive_answer__descriptor =
{
  PROTOBUF_C_MESSAGE_DESCRIPTOR_MAGIC,
  "GiftReceiveAnswer",
  "GiftReceiveAnswer",
  "GiftReceiveAnswer",
  "",
  sizeof(GiftReceiveAnswer),
  2,
  gift_receive_answer__field_descriptors,
  gift_receive_answer__field_indices_by_name,
  1,  gift_receive_answer__number_ranges,
  (ProtobufCMessageInit) gift_receive_answer__init,
  NULL,NULL,NULL    /* reserved[123] */
};
