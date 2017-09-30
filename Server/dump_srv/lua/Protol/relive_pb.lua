-- Generated By protoc-gen-lua Do not Edit
local protobuf = require "protobuf"
module('relive_pb')


local RELIVE_REQUEST = protobuf.Descriptor();
local RELIVE_REQUEST_TYPE_FIELD = protobuf.FieldDescriptor();
local RELIVE_NOTIFY = protobuf.Descriptor();
local RELIVE_NOTIFY_PLAYERID_FIELD = protobuf.FieldDescriptor();
local RELIVE_NOTIFY_TYPE_FIELD = protobuf.FieldDescriptor();
local RELIVE_NOTIFY_POS_X_FIELD = protobuf.FieldDescriptor();
local RELIVE_NOTIFY_POS_Z_FIELD = protobuf.FieldDescriptor();
local RELIVE_NOTIFY_DIRECT_FIELD = protobuf.FieldDescriptor();

RELIVE_REQUEST_TYPE_FIELD.name = "type"
RELIVE_REQUEST_TYPE_FIELD.full_name = ".relive_request.type"
RELIVE_REQUEST_TYPE_FIELD.number = 1
RELIVE_REQUEST_TYPE_FIELD.index = 0
RELIVE_REQUEST_TYPE_FIELD.label = 2
RELIVE_REQUEST_TYPE_FIELD.has_default_value = false
RELIVE_REQUEST_TYPE_FIELD.default_value = 0
RELIVE_REQUEST_TYPE_FIELD.type = 13
RELIVE_REQUEST_TYPE_FIELD.cpp_type = 3

RELIVE_REQUEST.name = "relive_request"
RELIVE_REQUEST.full_name = ".relive_request"
RELIVE_REQUEST.nested_types = {}
RELIVE_REQUEST.enum_types = {}
RELIVE_REQUEST.fields = {RELIVE_REQUEST_TYPE_FIELD}
RELIVE_REQUEST.is_extendable = false
RELIVE_REQUEST.extensions = {}
RELIVE_NOTIFY_PLAYERID_FIELD.name = "playerid"
RELIVE_NOTIFY_PLAYERID_FIELD.full_name = ".relive_notify.playerid"
RELIVE_NOTIFY_PLAYERID_FIELD.number = 1
RELIVE_NOTIFY_PLAYERID_FIELD.index = 0
RELIVE_NOTIFY_PLAYERID_FIELD.label = 2
RELIVE_NOTIFY_PLAYERID_FIELD.has_default_value = false
RELIVE_NOTIFY_PLAYERID_FIELD.default_value = 0
RELIVE_NOTIFY_PLAYERID_FIELD.type = 4
RELIVE_NOTIFY_PLAYERID_FIELD.cpp_type = 4

RELIVE_NOTIFY_TYPE_FIELD.name = "type"
RELIVE_NOTIFY_TYPE_FIELD.full_name = ".relive_notify.type"
RELIVE_NOTIFY_TYPE_FIELD.number = 2
RELIVE_NOTIFY_TYPE_FIELD.index = 1
RELIVE_NOTIFY_TYPE_FIELD.label = 2
RELIVE_NOTIFY_TYPE_FIELD.has_default_value = false
RELIVE_NOTIFY_TYPE_FIELD.default_value = 0
RELIVE_NOTIFY_TYPE_FIELD.type = 13
RELIVE_NOTIFY_TYPE_FIELD.cpp_type = 3

RELIVE_NOTIFY_POS_X_FIELD.name = "pos_x"
RELIVE_NOTIFY_POS_X_FIELD.full_name = ".relive_notify.pos_x"
RELIVE_NOTIFY_POS_X_FIELD.number = 3
RELIVE_NOTIFY_POS_X_FIELD.index = 2
RELIVE_NOTIFY_POS_X_FIELD.label = 2
RELIVE_NOTIFY_POS_X_FIELD.has_default_value = false
RELIVE_NOTIFY_POS_X_FIELD.default_value = 0
RELIVE_NOTIFY_POS_X_FIELD.type = 5
RELIVE_NOTIFY_POS_X_FIELD.cpp_type = 1

RELIVE_NOTIFY_POS_Z_FIELD.name = "pos_z"
RELIVE_NOTIFY_POS_Z_FIELD.full_name = ".relive_notify.pos_z"
RELIVE_NOTIFY_POS_Z_FIELD.number = 4
RELIVE_NOTIFY_POS_Z_FIELD.index = 3
RELIVE_NOTIFY_POS_Z_FIELD.label = 2
RELIVE_NOTIFY_POS_Z_FIELD.has_default_value = false
RELIVE_NOTIFY_POS_Z_FIELD.default_value = 0
RELIVE_NOTIFY_POS_Z_FIELD.type = 5
RELIVE_NOTIFY_POS_Z_FIELD.cpp_type = 1

RELIVE_NOTIFY_DIRECT_FIELD.name = "direct"
RELIVE_NOTIFY_DIRECT_FIELD.full_name = ".relive_notify.direct"
RELIVE_NOTIFY_DIRECT_FIELD.number = 5
RELIVE_NOTIFY_DIRECT_FIELD.index = 4
RELIVE_NOTIFY_DIRECT_FIELD.label = 2
RELIVE_NOTIFY_DIRECT_FIELD.has_default_value = false
RELIVE_NOTIFY_DIRECT_FIELD.default_value = 0
RELIVE_NOTIFY_DIRECT_FIELD.type = 5
RELIVE_NOTIFY_DIRECT_FIELD.cpp_type = 1

RELIVE_NOTIFY.name = "relive_notify"
RELIVE_NOTIFY.full_name = ".relive_notify"
RELIVE_NOTIFY.nested_types = {}
RELIVE_NOTIFY.enum_types = {}
RELIVE_NOTIFY.fields = {RELIVE_NOTIFY_PLAYERID_FIELD, RELIVE_NOTIFY_TYPE_FIELD, RELIVE_NOTIFY_POS_X_FIELD, RELIVE_NOTIFY_POS_Z_FIELD, RELIVE_NOTIFY_DIRECT_FIELD}
RELIVE_NOTIFY.is_extendable = false
RELIVE_NOTIFY.extensions = {}

relive_notify = protobuf.Message(RELIVE_NOTIFY)
relive_request = protobuf.Message(RELIVE_REQUEST)
