/* Generated by the protocol buffer compiler.  DO NOT EDIT! */

#ifndef PROTOBUF_C_team_2eproto__INCLUDED
#define PROTOBUF_C_team_2eproto__INCLUDED

#include <google/protobuf-c/protobuf-c.h>

PROTOBUF_C_BEGIN_DECLS


typedef struct _TeamMemInfo TeamMemInfo;
typedef struct _TeamApplyAnswer TeamApplyAnswer;
typedef struct _TeamInfo TeamInfo;
typedef struct _Teamid Teamid;
typedef struct _TeamPlayerid TeamPlayerid;
typedef struct _TeamInvite TeamInvite;
typedef struct _HandleTeamInvite HandleTeamInvite;
typedef struct _TeamApplyerList TeamApplyerList;
typedef struct _DelTeamPlayer DelTeamPlayer;
typedef struct _HandleTeamApply HandleTeamApply;
typedef struct _TeamLimited TeamLimited;
typedef struct _TeamTarget TeamTarget;
typedef struct _TeamListInfo TeamListInfo;
typedef struct _TeamList TeamList;
typedef struct _TeamBeLead TeamBeLead;
typedef struct _BeLeadAnswer BeLeadAnswer;
typedef struct _TeamNotifyCd TeamNotifyCd;
typedef struct _TeamHp TeamHp;
typedef struct _RefuceApplyTeam RefuceApplyTeam;
typedef struct _ChangeTeamid ChangeTeamid;
typedef struct _MatchAnser MatchAnser;
typedef struct _Follow Follow;


/* --- enums --- */

typedef enum _TeamChose {
  TEAM_CHOSE__YES = 1,
  TEAM_CHOSE__NO = 2
} TeamChose;
typedef enum _TeamTargetType {
  TEAM_TARGET_TYPE__sys = 1,
  TEAM_TARGET_TYPE__near = 2,
  TEAM_TARGET_TYPE__fb = 3
} TeamTargetType;

/* --- messages --- */

struct  _TeamMemInfo
{
  ProtobufCMessage base;
  uint64_t playerid;
  uint32_t icon;
  char *name;
  uint32_t lv;
  uint32_t job;
  int32_t hp;
  uint32_t maxhp;
  protobuf_c_boolean online;
  uint32_t clothes;
  uint32_t clothes_color_up;
  uint32_t clothes_color_down;
  uint32_t hat;
  uint32_t hat_color;
  protobuf_c_boolean follow;
  uint32_t weapon;
  uint32_t fight;
  float pos_x;
  float pos_z;
  uint32_t scene_id;
  uint32_t weapon_color;
  uint32_t head_icon;
  uint32_t zhenying;
};
#define TEAM_MEM_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_mem_info__descriptor) \
    , 0, 0, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }


struct  _TeamApplyAnswer
{
  ProtobufCMessage base;
  uint32_t errcode;
  uint32_t teamid;
};
#define TEAM_APPLY_ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_apply_answer__descriptor) \
    , 0, 0 }


struct  _TeamInfo
{
  ProtobufCMessage base;
  uint32_t teamid;
  size_t n_mem;
  TeamMemInfo **mem;
  TeamLimited *limit;
};
#define TEAM_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_info__descriptor) \
    , 0, 0,NULL, NULL }


struct  _Teamid
{
  ProtobufCMessage base;
  uint32_t id;
};
#define TEAMID__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&teamid__descriptor) \
    , 0 }


struct  _TeamPlayerid
{
  ProtobufCMessage base;
  uint64_t id;
};
#define TEAM_PLAYERID__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_playerid__descriptor) \
    , 0 }


struct  _TeamInvite
{
  ProtobufCMessage base;
  uint32_t teamid;
  TeamMemInfo *lead;
  uint32_t target;
};
#define TEAM_INVITE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_invite__descriptor) \
    , 0, NULL, 0 }


struct  _HandleTeamInvite
{
  ProtobufCMessage base;
  uint32_t teamid;
  uint64_t playerid;
  uint32_t accept;
};
#define HANDLE_TEAM_INVITE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&handle_team_invite__descriptor) \
    , 0, 0, 0 }


struct  _TeamApplyerList
{
  ProtobufCMessage base;
  size_t n_apply;
  TeamMemInfo **apply;
};
#define TEAM_APPLYER_LIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_applyer_list__descriptor) \
    , 0,NULL }


struct  _DelTeamPlayer
{
  ProtobufCMessage base;
  uint64_t playerid;
  protobuf_c_boolean kick;
};
#define DEL_TEAM_PLAYER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&del_team_player__descriptor) \
    , 0, 0 }


struct  _HandleTeamApply
{
  ProtobufCMessage base;
  protobuf_c_boolean has_chose;
  int32_t chose;
  protobuf_c_boolean has_id;
  uint64_t id;
};
#define HANDLE_TEAM_APPLY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&handle_team_apply__descriptor) \
    , 0,0, 0,0 }


struct  _TeamLimited
{
  ProtobufCMessage base;
  uint32_t target;
  uint32_t lv;
  protobuf_c_boolean auto_accept;
  protobuf_c_boolean has_speek;
  protobuf_c_boolean speek;
  uint32_t lv_max;
  uint32_t lv_min;
};
#define TEAM_LIMITED__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_limited__descriptor) \
    , 0, 0, 0, 0,0, 0, 0 }


struct  _TeamTarget
{
  ProtobufCMessage base;
  uint32_t target;
  int32_t type;
};
#define TEAM_TARGET__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_target__descriptor) \
    , 0, 0 }


struct  _TeamListInfo
{
  ProtobufCMessage base;
  uint32_t teamid;
  TeamLimited *limit;
  size_t n_lead;
  TeamMemInfo **lead;
  size_t n_job;
  int32_t *job;
  size_t n_lv;
  int32_t *lv;
  protobuf_c_boolean apply;
};
#define TEAM_LIST_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_list_info__descriptor) \
    , 0, NULL, 0,NULL, 0,NULL, 0,NULL, 0 }


struct  _TeamList
{
  ProtobufCMessage base;
  size_t n_team;
  TeamListInfo **team;
  int32_t player;
};
#define TEAM_LIST__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_list__descriptor) \
    , 0,NULL, 0 }


struct  _TeamBeLead
{
  ProtobufCMessage base;
  uint64_t playerid;
  int32_t chose;
  char *name;
};
#define TEAM_BE_LEAD__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_be_lead__descriptor) \
    , 0, 0, NULL }


struct  _BeLeadAnswer
{
  ProtobufCMessage base;
  int32_t ret;
  char *name;
};
#define BE_LEAD_ANSWER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&be_lead_answer__descriptor) \
    , 0, NULL }


struct  _TeamNotifyCd
{
  ProtobufCMessage base;
  int32_t cd;
};
#define TEAM_NOTIFY_CD__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_notify_cd__descriptor) \
    , 0 }


struct  _TeamHp
{
  ProtobufCMessage base;
  uint32_t hp;
  uint32_t maxhp;
  uint32_t lv;
  uint64_t playerid;
};
#define TEAM_HP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&team_hp__descriptor) \
    , 0, 0, 0, 0 }


struct  _RefuceApplyTeam
{
  ProtobufCMessage base;
  char *name;
};
#define REFUCE_APPLY_TEAM__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&refuce_apply_team__descriptor) \
    , NULL }


struct  _ChangeTeamid
{
  ProtobufCMessage base;
  uint64_t playerid;
  uint32_t teamid;
};
#define CHANGE_TEAMID__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&change_teamid__descriptor) \
    , 0, 0 }


struct  _MatchAnser
{
  ProtobufCMessage base;
  int32_t ret;
  uint32_t target;
};
#define MATCH_ANSER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&match_anser__descriptor) \
    , 0, 0 }


struct  _Follow
{
  ProtobufCMessage base;
  protobuf_c_boolean state;
  uint64_t playerid;
};
#define FOLLOW__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&follow__descriptor) \
    , 0, 0 }


/* TeamMemInfo methods */
void   team_mem_info__init
                     (TeamMemInfo         *message);
size_t team_mem_info__get_packed_size
                     (const TeamMemInfo   *message);
size_t team_mem_info__pack
                     (const TeamMemInfo   *message,
                      uint8_t             *out);
size_t team_mem_info__pack_to_buffer
                     (const TeamMemInfo   *message,
                      ProtobufCBuffer     *buffer);
TeamMemInfo *
       team_mem_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_mem_info__free_unpacked
                     (TeamMemInfo *message,
                      ProtobufCAllocator *allocator);
/* TeamApplyAnswer methods */
void   team_apply_answer__init
                     (TeamApplyAnswer         *message);
size_t team_apply_answer__get_packed_size
                     (const TeamApplyAnswer   *message);
size_t team_apply_answer__pack
                     (const TeamApplyAnswer   *message,
                      uint8_t             *out);
size_t team_apply_answer__pack_to_buffer
                     (const TeamApplyAnswer   *message,
                      ProtobufCBuffer     *buffer);
TeamApplyAnswer *
       team_apply_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_apply_answer__free_unpacked
                     (TeamApplyAnswer *message,
                      ProtobufCAllocator *allocator);
/* TeamInfo methods */
void   team_info__init
                     (TeamInfo         *message);
size_t team_info__get_packed_size
                     (const TeamInfo   *message);
size_t team_info__pack
                     (const TeamInfo   *message,
                      uint8_t             *out);
size_t team_info__pack_to_buffer
                     (const TeamInfo   *message,
                      ProtobufCBuffer     *buffer);
TeamInfo *
       team_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_info__free_unpacked
                     (TeamInfo *message,
                      ProtobufCAllocator *allocator);
/* Teamid methods */
void   teamid__init
                     (Teamid         *message);
size_t teamid__get_packed_size
                     (const Teamid   *message);
size_t teamid__pack
                     (const Teamid   *message,
                      uint8_t             *out);
size_t teamid__pack_to_buffer
                     (const Teamid   *message,
                      ProtobufCBuffer     *buffer);
Teamid *
       teamid__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   teamid__free_unpacked
                     (Teamid *message,
                      ProtobufCAllocator *allocator);
/* TeamPlayerid methods */
void   team_playerid__init
                     (TeamPlayerid         *message);
size_t team_playerid__get_packed_size
                     (const TeamPlayerid   *message);
size_t team_playerid__pack
                     (const TeamPlayerid   *message,
                      uint8_t             *out);
size_t team_playerid__pack_to_buffer
                     (const TeamPlayerid   *message,
                      ProtobufCBuffer     *buffer);
TeamPlayerid *
       team_playerid__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_playerid__free_unpacked
                     (TeamPlayerid *message,
                      ProtobufCAllocator *allocator);
/* TeamInvite methods */
void   team_invite__init
                     (TeamInvite         *message);
size_t team_invite__get_packed_size
                     (const TeamInvite   *message);
size_t team_invite__pack
                     (const TeamInvite   *message,
                      uint8_t             *out);
size_t team_invite__pack_to_buffer
                     (const TeamInvite   *message,
                      ProtobufCBuffer     *buffer);
TeamInvite *
       team_invite__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_invite__free_unpacked
                     (TeamInvite *message,
                      ProtobufCAllocator *allocator);
/* HandleTeamInvite methods */
void   handle_team_invite__init
                     (HandleTeamInvite         *message);
size_t handle_team_invite__get_packed_size
                     (const HandleTeamInvite   *message);
size_t handle_team_invite__pack
                     (const HandleTeamInvite   *message,
                      uint8_t             *out);
size_t handle_team_invite__pack_to_buffer
                     (const HandleTeamInvite   *message,
                      ProtobufCBuffer     *buffer);
HandleTeamInvite *
       handle_team_invite__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   handle_team_invite__free_unpacked
                     (HandleTeamInvite *message,
                      ProtobufCAllocator *allocator);
/* TeamApplyerList methods */
void   team_applyer_list__init
                     (TeamApplyerList         *message);
size_t team_applyer_list__get_packed_size
                     (const TeamApplyerList   *message);
size_t team_applyer_list__pack
                     (const TeamApplyerList   *message,
                      uint8_t             *out);
size_t team_applyer_list__pack_to_buffer
                     (const TeamApplyerList   *message,
                      ProtobufCBuffer     *buffer);
TeamApplyerList *
       team_applyer_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_applyer_list__free_unpacked
                     (TeamApplyerList *message,
                      ProtobufCAllocator *allocator);
/* DelTeamPlayer methods */
void   del_team_player__init
                     (DelTeamPlayer         *message);
size_t del_team_player__get_packed_size
                     (const DelTeamPlayer   *message);
size_t del_team_player__pack
                     (const DelTeamPlayer   *message,
                      uint8_t             *out);
size_t del_team_player__pack_to_buffer
                     (const DelTeamPlayer   *message,
                      ProtobufCBuffer     *buffer);
DelTeamPlayer *
       del_team_player__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   del_team_player__free_unpacked
                     (DelTeamPlayer *message,
                      ProtobufCAllocator *allocator);
/* HandleTeamApply methods */
void   handle_team_apply__init
                     (HandleTeamApply         *message);
size_t handle_team_apply__get_packed_size
                     (const HandleTeamApply   *message);
size_t handle_team_apply__pack
                     (const HandleTeamApply   *message,
                      uint8_t             *out);
size_t handle_team_apply__pack_to_buffer
                     (const HandleTeamApply   *message,
                      ProtobufCBuffer     *buffer);
HandleTeamApply *
       handle_team_apply__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   handle_team_apply__free_unpacked
                     (HandleTeamApply *message,
                      ProtobufCAllocator *allocator);
/* TeamLimited methods */
void   team_limited__init
                     (TeamLimited         *message);
size_t team_limited__get_packed_size
                     (const TeamLimited   *message);
size_t team_limited__pack
                     (const TeamLimited   *message,
                      uint8_t             *out);
size_t team_limited__pack_to_buffer
                     (const TeamLimited   *message,
                      ProtobufCBuffer     *buffer);
TeamLimited *
       team_limited__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_limited__free_unpacked
                     (TeamLimited *message,
                      ProtobufCAllocator *allocator);
/* TeamTarget methods */
void   team_target__init
                     (TeamTarget         *message);
size_t team_target__get_packed_size
                     (const TeamTarget   *message);
size_t team_target__pack
                     (const TeamTarget   *message,
                      uint8_t             *out);
size_t team_target__pack_to_buffer
                     (const TeamTarget   *message,
                      ProtobufCBuffer     *buffer);
TeamTarget *
       team_target__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_target__free_unpacked
                     (TeamTarget *message,
                      ProtobufCAllocator *allocator);
/* TeamListInfo methods */
void   team_list_info__init
                     (TeamListInfo         *message);
size_t team_list_info__get_packed_size
                     (const TeamListInfo   *message);
size_t team_list_info__pack
                     (const TeamListInfo   *message,
                      uint8_t             *out);
size_t team_list_info__pack_to_buffer
                     (const TeamListInfo   *message,
                      ProtobufCBuffer     *buffer);
TeamListInfo *
       team_list_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_list_info__free_unpacked
                     (TeamListInfo *message,
                      ProtobufCAllocator *allocator);
/* TeamList methods */
void   team_list__init
                     (TeamList         *message);
size_t team_list__get_packed_size
                     (const TeamList   *message);
size_t team_list__pack
                     (const TeamList   *message,
                      uint8_t             *out);
size_t team_list__pack_to_buffer
                     (const TeamList   *message,
                      ProtobufCBuffer     *buffer);
TeamList *
       team_list__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_list__free_unpacked
                     (TeamList *message,
                      ProtobufCAllocator *allocator);
/* TeamBeLead methods */
void   team_be_lead__init
                     (TeamBeLead         *message);
size_t team_be_lead__get_packed_size
                     (const TeamBeLead   *message);
size_t team_be_lead__pack
                     (const TeamBeLead   *message,
                      uint8_t             *out);
size_t team_be_lead__pack_to_buffer
                     (const TeamBeLead   *message,
                      ProtobufCBuffer     *buffer);
TeamBeLead *
       team_be_lead__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_be_lead__free_unpacked
                     (TeamBeLead *message,
                      ProtobufCAllocator *allocator);
/* BeLeadAnswer methods */
void   be_lead_answer__init
                     (BeLeadAnswer         *message);
size_t be_lead_answer__get_packed_size
                     (const BeLeadAnswer   *message);
size_t be_lead_answer__pack
                     (const BeLeadAnswer   *message,
                      uint8_t             *out);
size_t be_lead_answer__pack_to_buffer
                     (const BeLeadAnswer   *message,
                      ProtobufCBuffer     *buffer);
BeLeadAnswer *
       be_lead_answer__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   be_lead_answer__free_unpacked
                     (BeLeadAnswer *message,
                      ProtobufCAllocator *allocator);
/* TeamNotifyCd methods */
void   team_notify_cd__init
                     (TeamNotifyCd         *message);
size_t team_notify_cd__get_packed_size
                     (const TeamNotifyCd   *message);
size_t team_notify_cd__pack
                     (const TeamNotifyCd   *message,
                      uint8_t             *out);
size_t team_notify_cd__pack_to_buffer
                     (const TeamNotifyCd   *message,
                      ProtobufCBuffer     *buffer);
TeamNotifyCd *
       team_notify_cd__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_notify_cd__free_unpacked
                     (TeamNotifyCd *message,
                      ProtobufCAllocator *allocator);
/* TeamHp methods */
void   team_hp__init
                     (TeamHp         *message);
size_t team_hp__get_packed_size
                     (const TeamHp   *message);
size_t team_hp__pack
                     (const TeamHp   *message,
                      uint8_t             *out);
size_t team_hp__pack_to_buffer
                     (const TeamHp   *message,
                      ProtobufCBuffer     *buffer);
TeamHp *
       team_hp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   team_hp__free_unpacked
                     (TeamHp *message,
                      ProtobufCAllocator *allocator);
/* RefuceApplyTeam methods */
void   refuce_apply_team__init
                     (RefuceApplyTeam         *message);
size_t refuce_apply_team__get_packed_size
                     (const RefuceApplyTeam   *message);
size_t refuce_apply_team__pack
                     (const RefuceApplyTeam   *message,
                      uint8_t             *out);
size_t refuce_apply_team__pack_to_buffer
                     (const RefuceApplyTeam   *message,
                      ProtobufCBuffer     *buffer);
RefuceApplyTeam *
       refuce_apply_team__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   refuce_apply_team__free_unpacked
                     (RefuceApplyTeam *message,
                      ProtobufCAllocator *allocator);
/* ChangeTeamid methods */
void   change_teamid__init
                     (ChangeTeamid         *message);
size_t change_teamid__get_packed_size
                     (const ChangeTeamid   *message);
size_t change_teamid__pack
                     (const ChangeTeamid   *message,
                      uint8_t             *out);
size_t change_teamid__pack_to_buffer
                     (const ChangeTeamid   *message,
                      ProtobufCBuffer     *buffer);
ChangeTeamid *
       change_teamid__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   change_teamid__free_unpacked
                     (ChangeTeamid *message,
                      ProtobufCAllocator *allocator);
/* MatchAnser methods */
void   match_anser__init
                     (MatchAnser         *message);
size_t match_anser__get_packed_size
                     (const MatchAnser   *message);
size_t match_anser__pack
                     (const MatchAnser   *message,
                      uint8_t             *out);
size_t match_anser__pack_to_buffer
                     (const MatchAnser   *message,
                      ProtobufCBuffer     *buffer);
MatchAnser *
       match_anser__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   match_anser__free_unpacked
                     (MatchAnser *message,
                      ProtobufCAllocator *allocator);
/* Follow methods */
void   follow__init
                     (Follow         *message);
size_t follow__get_packed_size
                     (const Follow   *message);
size_t follow__pack
                     (const Follow   *message,
                      uint8_t             *out);
size_t follow__pack_to_buffer
                     (const Follow   *message,
                      ProtobufCBuffer     *buffer);
Follow *
       follow__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   follow__free_unpacked
                     (Follow *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*TeamMemInfo_Closure)
                 (const TeamMemInfo *message,
                  void *closure_data);
typedef void (*TeamApplyAnswer_Closure)
                 (const TeamApplyAnswer *message,
                  void *closure_data);
typedef void (*TeamInfo_Closure)
                 (const TeamInfo *message,
                  void *closure_data);
typedef void (*Teamid_Closure)
                 (const Teamid *message,
                  void *closure_data);
typedef void (*TeamPlayerid_Closure)
                 (const TeamPlayerid *message,
                  void *closure_data);
typedef void (*TeamInvite_Closure)
                 (const TeamInvite *message,
                  void *closure_data);
typedef void (*HandleTeamInvite_Closure)
                 (const HandleTeamInvite *message,
                  void *closure_data);
typedef void (*TeamApplyerList_Closure)
                 (const TeamApplyerList *message,
                  void *closure_data);
typedef void (*DelTeamPlayer_Closure)
                 (const DelTeamPlayer *message,
                  void *closure_data);
typedef void (*HandleTeamApply_Closure)
                 (const HandleTeamApply *message,
                  void *closure_data);
typedef void (*TeamLimited_Closure)
                 (const TeamLimited *message,
                  void *closure_data);
typedef void (*TeamTarget_Closure)
                 (const TeamTarget *message,
                  void *closure_data);
typedef void (*TeamListInfo_Closure)
                 (const TeamListInfo *message,
                  void *closure_data);
typedef void (*TeamList_Closure)
                 (const TeamList *message,
                  void *closure_data);
typedef void (*TeamBeLead_Closure)
                 (const TeamBeLead *message,
                  void *closure_data);
typedef void (*BeLeadAnswer_Closure)
                 (const BeLeadAnswer *message,
                  void *closure_data);
typedef void (*TeamNotifyCd_Closure)
                 (const TeamNotifyCd *message,
                  void *closure_data);
typedef void (*TeamHp_Closure)
                 (const TeamHp *message,
                  void *closure_data);
typedef void (*RefuceApplyTeam_Closure)
                 (const RefuceApplyTeam *message,
                  void *closure_data);
typedef void (*ChangeTeamid_Closure)
                 (const ChangeTeamid *message,
                  void *closure_data);
typedef void (*MatchAnser_Closure)
                 (const MatchAnser *message,
                  void *closure_data);
typedef void (*Follow_Closure)
                 (const Follow *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCEnumDescriptor    team_chose__descriptor;
extern const ProtobufCEnumDescriptor    team_target_type__descriptor;
extern const ProtobufCMessageDescriptor team_mem_info__descriptor;
extern const ProtobufCMessageDescriptor team_apply_answer__descriptor;
extern const ProtobufCMessageDescriptor team_info__descriptor;
extern const ProtobufCMessageDescriptor teamid__descriptor;
extern const ProtobufCMessageDescriptor team_playerid__descriptor;
extern const ProtobufCMessageDescriptor team_invite__descriptor;
extern const ProtobufCMessageDescriptor handle_team_invite__descriptor;
extern const ProtobufCMessageDescriptor team_applyer_list__descriptor;
extern const ProtobufCMessageDescriptor del_team_player__descriptor;
extern const ProtobufCMessageDescriptor handle_team_apply__descriptor;
extern const ProtobufCMessageDescriptor team_limited__descriptor;
extern const ProtobufCMessageDescriptor team_target__descriptor;
extern const ProtobufCMessageDescriptor team_list_info__descriptor;
extern const ProtobufCMessageDescriptor team_list__descriptor;
extern const ProtobufCMessageDescriptor team_be_lead__descriptor;
extern const ProtobufCMessageDescriptor be_lead_answer__descriptor;
extern const ProtobufCMessageDescriptor team_notify_cd__descriptor;
extern const ProtobufCMessageDescriptor team_hp__descriptor;
extern const ProtobufCMessageDescriptor refuce_apply_team__descriptor;
extern const ProtobufCMessageDescriptor change_teamid__descriptor;
extern const ProtobufCMessageDescriptor match_anser__descriptor;
extern const ProtobufCMessageDescriptor follow__descriptor;

PROTOBUF_C_END_DECLS


#endif  /* PROTOBUF_team_2eproto__INCLUDED */
