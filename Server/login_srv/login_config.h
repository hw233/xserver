#ifndef __LOGIN_CONFIG_H__
#define __LOGIN_CONFIG_H__

#include <map>
#include <vector>
#include <stdint.h>
#include "excel_data.h"
#include "lua_template.h"

extern std::map<uint64_t, struct SceneResTable *> scene_res_config; //阻挡，寻路数据
extern std::map<uint64_t, struct ActorFashionTable *> fashion_config; //时装配置
extern std::map<uint64_t, struct ParameterTable *> parameter_config; //参数配置
extern std::map<uint64_t, struct ActorTable *> actor_config;

int read_all_excel_data();

#endif /* __LOGIN_CONFIG_H__ */
