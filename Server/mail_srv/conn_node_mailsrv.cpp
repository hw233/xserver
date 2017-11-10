#include "conn_node_mailsrv.h"
#include "game_event.h"
#include "msgid.h"
#include "error_code.h"
#include "mysql_module.h"
#include "mail_db.pb-c.h"
#include "mail.pb-c.h"
#include "time_helper.h"
#include <vector>


conn_node_mailsrv conn_node_mailsrv::connecter;
static char sql[1024];

static const uint32_t MAIL_KEEP_TIME = (30 * 24 * 60 * 60); //邮件保留时间为30天


conn_node_mailsrv::conn_node_mailsrv()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);

	add_msg_handle(SERVER_PROTO_GAMESRV_START, &conn_node_mailsrv::clear_mail_state);
	add_msg_handle(SERVER_PROTO_MAIL_INSERT, &conn_node_mailsrv::handle_mail_insert);
	add_msg_handle(MSG_ID_MAIL_LIST_REQUEST, &conn_node_mailsrv::handle_mail_list_request);
	add_msg_handle(MSG_ID_MAIL_READ_REQUEST, &conn_node_mailsrv::handle_mail_read_request);
	add_msg_handle(MSG_ID_MAIL_GET_ATTACH_REQUEST, &conn_node_mailsrv::handle_mail_get_attach_request);
	add_msg_handle(SERVER_PROTO_MAIL_GIVE_ATTACH_ANSWER, &conn_node_mailsrv::handle_mail_give_attach_answer);
	add_msg_handle(MSG_ID_MAIL_DEL_REQUEST, &conn_node_mailsrv::handle_mail_del_request);
}

conn_node_mailsrv::~conn_node_mailsrv()
{
}

void conn_node_mailsrv::add_msg_handle(uint32_t msg_id, handle_func func)
{
	connecter.m_handleMap[msg_id] = func;
}

int conn_node_mailsrv::recv_func(evutil_socket_t fd)
{
	EXTERN_DATA *extern_data;
	PROTO_HEAD *head;	
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
			head = (PROTO_HEAD *)buf_head();
			extern_data = get_extern_data(head);
			int cmd = get_cmd();
			LOG_DEBUG("[%s:%d] cmd: %u, player_id: %lu", __FUNCTION__, __LINE__, cmd, extern_data->player_id);
			if (cmd == SERVER_PROTO_GAMESRV_START)
			{
				LOG_INFO("[%s:%d] game_srv start notify.", __FUNCTION__, __LINE__);
			}

			uint64_t times = time_helper::get_micro_time();
			time_helper::set_cached_time(times / 1000);

			HandleMap::iterator iter = m_handleMap.find(cmd);
			if (iter != m_handleMap.end())
			{
				(this->*(iter->second))(extern_data);
			}
			else
			{
				LOG_ERR("[%s:%d] cmd %u has no handler", __FUNCTION__, __LINE__, cmd);
			}
		}
		
		if (ret < 0) {
			LOG_INFO("%s: connect closed from fd %u, err = %d", __FUNCTION__, fd, errno);
			exit(0);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

int conn_node_mailsrv::clear_mail_state(EXTERN_DATA *extern_data)
{
	snprintf(sql, sizeof(sql), "update mail set state = 0 where state = 1;");
	query(sql, 1, NULL);

	return 0;
}

int conn_node_mailsrv::handle_mail_insert(EXTERN_DATA *extern_data)
{
	PROTO_MAIL_INSERT *proto = (PROTO_MAIL_INSERT*)get_head();
	size_t data_size = proto->data_size;
	MailDBInfo *mail_data = mail_dbinfo__unpack(NULL, data_size, (uint8_t*)proto->data);
	if (!mail_data)
	{
		LOG_ERR("[%s:%d] mail_dbinfo__unpack failed", __FUNCTION__, __LINE__);
		return -1;
	}

	uint64_t player_id = proto->player_id;

	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	int len = 0;
	char *p = NULL;
	uint64_t effect = 0;
	do
	{
		sprintf(sql, "select count(mail_id) from mail where player_id = %lu", player_id);
		res = query(sql, 1, NULL);
		if (!res)
		{
			LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
			break;
		}

		row = fetch_row(res);
		if (!row)
		{
			LOG_ERR("[%s:%d] mysql fetch row failed, sql:%s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint32_t mail_num = atoi(row[0]);
		free_query(res);
		res = NULL;

		//如果邮件已满，删除最早的
		if (mail_num >= MAX_MAIL_NUM)
		{
			snprintf(sql, sizeof(sql), "delete from mail where player_id = %lu order by mail_id asc limit %u;", player_id, (mail_num - MAX_MAIL_NUM + 1));
			query(sql, 1, NULL);
		}

		//插入新邮件
		len = sprintf(sql, "insert into mail set player_id = %lu, time = now(), data = \'", player_id);
		p = sql + len;
		p += escape_string(p, (char*)proto->data, data_size);
		*p++ = '\'';
		*p = '\0';
		
//		query(const_cast<char*>("set names utf8"), 1, NULL);
		query(sql, 1, &effect);
		if (effect != 1)
		{
			LOG_ERR("[%s:%d] mysql insert mail failed, player_id:%lu", __FUNCTION__, __LINE__, player_id);
			break;
		}

		//获取信息
		snprintf(sql, sizeof(sql), "select mail_id, UNIX_TIMESTAMP(time) from mail where mail_id = LAST_INSERT_ID();");
		res = query(sql, 1, NULL);
		if (!res)
		{
			LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
			break;
		}

		row = fetch_row(res);
		if (!row)
		{
			LOG_ERR("[%s:%d] mysql fetch row failed, sql:%s", __FUNCTION__, __LINE__, sql);
			break;
		}

		uint64_t mail_id = strtoull(row[0], NULL, 0);
		uint32_t send_time = strtoul(row[1], NULL, 0);

		free_query(res);
		res = NULL;

		//通知客户端
		MailData nty;
		mail_data__init(&nty);

		ItemData item_data[MAX_MAIL_ATTACH_NUM];
		ItemData* item_data_point[MAX_MAIL_ATTACH_NUM];

		nty.id = mail_id;
		nty.type = mail_data->type;
		nty.title = mail_data->title;
		nty.sendername = mail_data->sendername;
		nty.content = mail_data->content;
		nty.n_args = mail_data->n_args;
		nty.args = mail_data->args;
		nty.sendtime = send_time;
		nty.n_attach = mail_data->n_attach;
		for (uint32_t i = 0; i < mail_data->n_attach; ++i)
		{
			item_data_point[i] = &item_data[i];
			item_data__init(&item_data[i]);
			item_data[i].id = mail_data->attach[i]->id;
			item_data[i].num = mail_data->attach[i]->num;
		}
		nty.attach = item_data_point;
		nty.read = 0;
		nty.extract = 0;

		EXTERN_DATA ext_data;
		ext_data.player_id = player_id;

		fast_send_msg(&connecter, &ext_data, MSG_ID_MAIL_INSERT_NOTIFY, mail_data__pack, nty);
	} while (0);

	if (mail_data)
	{
		mail_dbinfo__free_unpacked(mail_data, NULL);
	}
	if (res)
	{
		free_query(res);
		res = NULL;
	}

	return 0;
}

int conn_node_mailsrv::handle_mail_list_request(EXTERN_DATA *extern_data)
{
	MailListAnswer resp;
	mail_list_answer__init(&resp);

	MailData mail_data[MAX_MAIL_NUM];
	MailData* mail_data_point[MAX_MAIL_NUM];
	ItemData item_data[MAX_MAIL_NUM][MAX_MAIL_ATTACH_NUM];
	ItemData* item_data_point[MAX_MAIL_NUM][MAX_MAIL_ATTACH_NUM];

	resp.n_mails = 0;
	resp.mails = mail_data_point;
	
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	unsigned long *lengths = NULL;

	int ret = 0;
	std::vector<MailDBInfo*> mail_details;
	do
	{
		std::vector<uint64_t> del_ids;
		uint32_t now = time_helper::get_cached_time() / 1000;
		snprintf(sql, sizeof(sql), "select mail_id, extract, `read`, UNIX_TIMESTAMP(time), data from mail where player_id = %lu", extern_data->player_id);
		query(const_cast<char*>("set names utf8"), 1, NULL);
		res = query(sql, 1, NULL);
		if (!res)
		{
			ret = ERROR_ID_MYSQL_QUERY;
			LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
			break;
		}

		while (true)
		{
			row = fetch_row(res);
			if (!row)
			{
				break;
			}

			lengths = mysql_fetch_lengths(res);

			uint64_t mail_id = strtoull(row[0], NULL, 0);
			uint32_t extract = atoi(row[1]);
			uint32_t read = atoi(row[2]);
			uint32_t send_time = strtoul(row[3], NULL, 0);

			//超过30天的邮件删掉
			if (now - send_time >= MAIL_KEEP_TIME)
			{
				del_ids.push_back(mail_id);
				continue;
			}

			MailDBInfo *detail = mail_dbinfo__unpack(NULL, (size_t)lengths[4], (uint8_t*)row[4]);
			if (!detail)
			{
				LOG_ERR("[%s:%d] mail_dbinfo__unpack failed, player_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id);
				continue;
			}

			mail_data_point[resp.n_mails] = &mail_data[resp.n_mails];
			mail_data__init(&mail_data[resp.n_mails]);
			mail_data[resp.n_mails].id = mail_id;
			mail_data[resp.n_mails].type = detail->type;
			mail_data[resp.n_mails].title = detail->title;
			mail_data[resp.n_mails].sendername = detail->sendername;
			mail_data[resp.n_mails].content = detail->content;
			mail_data[resp.n_mails].n_args = detail->n_args;
			mail_data[resp.n_mails].args = detail->args;
			mail_data[resp.n_mails].sendtime = send_time;
			mail_data[resp.n_mails].read = read;
			mail_data[resp.n_mails].extract = extract;
			mail_data[resp.n_mails].n_attach = detail->n_attach;
			mail_data[resp.n_mails].attach = item_data_point[resp.n_mails];
			for (size_t i = 0; i < detail->n_attach && i < MAX_MAIL_ATTACH_NUM; ++i)
			{
				item_data_point[resp.n_mails][i] = &item_data[resp.n_mails][i];
				item_data__init(&item_data[resp.n_mails][i]);
				item_data[resp.n_mails][i].id = detail->attach[i]->id;
				item_data[resp.n_mails][i].num = detail->attach[i]->num;
			}

			resp.n_mails++;

			mail_details.push_back(detail);
		}

		free_query(res);
		res = NULL;

		size_t del_size = del_ids.size();
		if (del_size > 0)
		{
			int len = sprintf(sql, "delete from mail where player_id = %lu and mail_id in (", extern_data->player_id);
			char *p = sql + len;

			for (size_t i = 0; i < del_size; ++i)
			{
				uint64_t mail_id = del_ids[i];
				if (i == del_size - 1)
				{
					len = sprintf(p, "%lu);", mail_id);
				}
				else
				{
					len = sprintf(p, "%lu,", mail_id);
				}
				p += len;
			}
			query(sql, 1, NULL);
		}
	} while(0);

	resp.result = ret;

	fast_send_msg(&connecter, extern_data, MSG_ID_MAIL_LIST_ANSWER, mail_list_answer__pack, resp);

	for (std::vector<MailDBInfo*>::iterator iter = mail_details.begin(); iter != mail_details.end(); ++iter)
	{
		mail_dbinfo__free_unpacked(*iter, NULL);
	}

	return 0;
}

int conn_node_mailsrv::handle_mail_read_request(EXTERN_DATA *extern_data)
{
	MailCommRequest *req = mail_comm_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t mail_id = req->mail_id;
	mail_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	do
	{
		snprintf(sql, sizeof(sql), "update mail set `read` = 1 where player_id = %lu and mail_id = %lu;", extern_data->player_id, mail_id);
		query(sql, 1, NULL);
	} while(0);

	MailCommAnswer resp;
	mail_comm_answer__init(&resp);

	resp.result = ret;
	resp.mail_id = mail_id;

	fast_send_msg(&connecter, extern_data, MSG_ID_MAIL_READ_ANSWER, mail_comm_answer__pack, resp);

	return 0;
}

int conn_node_mailsrv::handle_mail_get_attach_request(EXTERN_DATA *extern_data)
{
	MailCommRequest *req = mail_comm_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t mail_id = req->mail_id;
	mail_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	unsigned long *lengths = NULL;
	int len = 0;
	char *p = NULL;

	std::vector<std::pair<uint64_t, MailDBInfo *> > mail_details;

	do
	{
		if (mail_id > 0) //领取指定邮件附件
		{
			snprintf(sql, sizeof(sql), "select state, extract, data from mail where player_id = %lu and mail_id = %lu", extern_data->player_id, mail_id);
			res = query(sql, 1, NULL);
			if (!res)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			row = fetch_row(res);
			if (!row)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql fetch row failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			lengths = mysql_fetch_lengths(res);

			uint32_t state = strtoul(row[0], NULL, 0);
			uint32_t extract = atoi(row[1]);
			if (extract != 0)
			{
				ret = ERROR_ID_MAIL_ATTACH_HAS_GOT;
				LOG_ERR("[%s:%d] player[%lu] attach has got, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}

			if (state != 0)
			{
				ret = ERROR_ID_MAIL_ATTACH_GETTING;
				LOG_ERR("[%s:%d] player[%lu] attach is getting, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}

			MailDBInfo *detail = mail_dbinfo__unpack(NULL, lengths[2], (uint8_t*)row[2]);
			if (!detail)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] player[%lu] mail_dbinfo__unpack failed, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}

			if (detail->n_attach == 0)
			{
				ret = ERROR_ID_MAIL_NO_ATTACH;
				LOG_ERR("[%s:%d] player[%lu] no attach, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}

			free_query(res);
			res = NULL;

			mail_details.push_back(std::make_pair(mail_id, detail));

			//标记领取中
			snprintf(sql, sizeof(sql), "update mail set state = 1 where player_id = %lu and mail_id = %lu", extern_data->player_id, mail_id);
			query(sql, 1, NULL);
		}
		else //一键领取
		{
			snprintf(sql, sizeof(sql), "select mail_id, data from mail where player_id = %lu and state = 0 and extract = 0;", extern_data->player_id);
			res = query(sql, 1, NULL);
			if (!res)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			std::vector<uint64_t> no_attach_mails;
			while (true)
			{
				row = fetch_row(res);
				if (!row)
				{
					break;
				}

				lengths = mysql_fetch_lengths(res);

				uint64_t mail_id = strtoull(row[0], NULL, 0);

				MailDBInfo *detail = mail_dbinfo__unpack(NULL, lengths[1], (uint8_t*)row[1]);
				if (!detail)
				{
					LOG_ERR("[%s:%d] player[%lu] mail_dbinfo__unpack failed, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
					continue;
				}

				if (detail->n_attach == 0)
				{
					no_attach_mails.push_back(mail_id);
					mail_dbinfo__free_unpacked(detail, NULL);
				}
				else
				{
					mail_details.push_back(std::make_pair(mail_id, detail));
				}
			}

			free_query(res);
			res = NULL;

			//没有附件的，直接标记成已领
			size_t no_attach_size = no_attach_mails.size();
			if (no_attach_size > 0)
			{
				len = sprintf(sql, "update mail set extract = 1, `read` = 1 where player_id = %lu and mail_id in (", extern_data->player_id);
				p = sql + len;

				for (size_t i = 0; i < no_attach_size; ++i)
				{
					uint64_t mail_id = no_attach_mails[i];
					if (i == no_attach_size - 1)
					{
						len = sprintf(p, "%lu);", mail_id);
					}
					else
					{
						len = sprintf(p, "%lu,", mail_id);
					}
					p += len;
				}
				query(sql, 1, NULL);
			}

			//有附件的，标记领取中
			size_t has_attach_size = mail_details.size();
			if (has_attach_size > 0)
			{
				len = sprintf(sql, "update mail set state = 1, `read` = 1 where player_id = %lu and mail_id in (", extern_data->player_id);
				p = sql + len;
				for (size_t i = 0; i < has_attach_size; ++i)
				{
					uint64_t mail_id = mail_details[i].first;
					if (i == has_attach_size - 1)
					{
						len = sprintf(p, "%lu);", mail_id);
					}
					else
					{
						len = sprintf(p, "%lu,", mail_id);
					}
					p += len;
				}

				query(sql, 1, NULL);
			}
		}
	} while(0);

	if (res)
	{
		free_query(res);
		res = NULL;
	}

	if (ret == 0 && mail_details.size() > 0)
	{
		MailGiveAttachRequest give_req;
		mail_give_attach_request__init(&give_req);

		MailAttachGiveInfo give_data[MAX_MAIL_NUM];
		MailAttachGiveInfo* give_data_point[MAX_MAIL_NUM];
		MailAttach attach_data[MAX_MAIL_NUM][MAX_MAIL_ATTACH_NUM];
		MailAttach* attach_data_point[MAX_MAIL_NUM][MAX_MAIL_ATTACH_NUM];

		give_req.n_mails = mail_details.size();
		give_req.mails = give_data_point;
		for (size_t i = 0; i < mail_details.size() && i < MAX_MAIL_NUM; ++i)
		{
			uint64_t mail_id = mail_details[i].first;
			MailDBInfo *detail = mail_details[i].second;
			give_data_point[i] = &give_data[i];
			mail_attach_give_info__init(&give_data[i]);
			give_data[i].mailid = mail_id;
			give_data[i].statisid = detail->statisid;
			give_data[i].n_attachs = detail->n_attach;
			give_data[i].attachs = attach_data_point[i];
			for (size_t j = 0; j < detail->n_attach; ++j)
			{
				attach_data_point[i][j] = &attach_data[i][j];
				mail_attach__init(&attach_data[i][j]);
				attach_data[i][j].id = detail->attach[j]->id;
				attach_data[i][j].num = detail->attach[j]->num;
			}

			mail_dbinfo__free_unpacked(detail, NULL);
		}
		fast_send_msg(&connecter, extern_data, SERVER_PROTO_MAIL_GIVE_ATTACH_REQUEST, mail_give_attach_request__pack, give_req);
	}
	else
	{
		MailMultiAnswer resp;
		mail_multi_answer__init(&resp);

		resp.result = ret;

		fast_send_msg(&connecter, extern_data, MSG_ID_MAIL_GET_ATTACH_ANSWER, mail_multi_answer__pack, resp);
	}

	return 0;
}

int conn_node_mailsrv::handle_mail_give_attach_answer(EXTERN_DATA *extern_data)
{
	MailGiveAttachAnswer *req = mail_give_attach_answer__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	int len = 0;
	char *p = NULL;
	int ret = 0;

	//领取成功的，清除标记，并变成已领取
	if (req->n_successids > 0)
	{
		len = sprintf(sql, "update mail set state = 0, extract = 1 where player_id = %lu and mail_id in (", extern_data->player_id);
		p = sql + len;
		for (size_t i = 0; i < req->n_successids; ++i)
		{
			uint64_t mail_id = req->successids[i];
			if (i == req->n_successids - 1)
			{
				len = sprintf(p, "%lu);", mail_id);
			}
			else
			{
				len = sprintf(p, "%lu,", mail_id);
			}
			p += len;
		}
		query(sql, 1, NULL);
	}

	//领取失败的，清除标记
	if (req->n_failids > 0)
	{
		ret = 190500081;
		len = sprintf(sql, "update mail set state = 0 where player_id = %lu and mail_id in (", extern_data->player_id);
		p = sql + len;
		for (size_t i = 0; i < req->n_failids; ++i)
		{
			uint64_t mail_id = req->failids[i];
			if (i == req->n_failids - 1)
			{
				len = sprintf(p, "%lu);", mail_id);
			}
			else
			{
				len = sprintf(p, "%lu,", mail_id);
			}
			p += len;
		}
		query(sql, 1, NULL);
	}

	MailMultiAnswer resp;
	mail_multi_answer__init(&resp);

	resp.result = ret;
	resp.mail_id = req->successids;
	resp.n_mail_id = req->n_successids;

	fast_send_msg(&connecter, extern_data, MSG_ID_MAIL_GET_ATTACH_ANSWER, mail_multi_answer__pack, resp);

	mail_give_attach_answer__free_unpacked(req, NULL);
	
	return 0;
}

int conn_node_mailsrv::handle_mail_del_request(EXTERN_DATA *extern_data)
{
	MailCommRequest *req = mail_comm_request__unpack(NULL, get_data_len(), get_data());
	if (!req)
	{
		LOG_ERR("[%s:%d] player[%lu] unpack failed", __FUNCTION__, __LINE__, extern_data->player_id);
		return -1;
	}

	uint64_t mail_id = req->mail_id;
	mail_comm_request__free_unpacked(req, NULL);

	int ret = 0;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row = NULL;
	unsigned long *lengths = NULL;
	int len = 0;
	char *p = NULL;

	std::vector<uint64_t> del_ids;
	std::vector<MailDBInfo*> mail_details;
	do
	{
		if (mail_id > 0) //删除指定邮件
		{
			snprintf(sql, sizeof(sql), "select extract, data from mail where player_id = %lu and mail_id = %lu", extern_data->player_id, mail_id);
			res = query(sql, 1, NULL);
			if (!res)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			row = fetch_row(res);
			if (!row)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql fetch row failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			lengths = mysql_fetch_lengths(res);

			uint32_t extract = atoi(row[0]);

			MailDBInfo *detail = mail_dbinfo__unpack(NULL, lengths[1], (uint8_t*)row[1]);
			if (!detail)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] player[%lu] mail_dbinfo__unpack failed, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}
			mail_details.push_back(detail);

			if (detail->n_attach > 0 && extract == 0)
			{
				ret = ERROR_ID_MAIL_ATTACH_NOT_GET;
				LOG_ERR("[%s:%d] player[%lu] attach not get, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
				break;
			}

			free_query(res);
			res = NULL;

			del_ids.push_back(mail_id);

			//删除邮件
			snprintf(sql, sizeof(sql), "delete from mail where player_id = %lu and mail_id = %lu", extern_data->player_id, mail_id);
			query(sql, 1, NULL);
		}
		else //一键删除
		{
			snprintf(sql, sizeof(sql), "select mail_id, extract, data from mail where player_id = %lu and `read` = 1;", extern_data->player_id);
			res = query(sql, 1, NULL);
			if (!res)
			{
				ret = ERROR_ID_MYSQL_QUERY;
				LOG_ERR("[%s:%d] mysql query failed, sql:%s", __FUNCTION__, __LINE__, sql);
				break;
			}

			while (true)
			{
				row = fetch_row(res);
				if (!row)
				{
					break;
				}

				lengths = mysql_fetch_lengths(res);

				uint64_t mail_id = strtoull(row[0], NULL, 0);
				uint32_t extract = atoi(row[1]);

				MailDBInfo *detail = mail_dbinfo__unpack(NULL, lengths[2], (uint8_t*)row[2]);
				if (!detail)
				{
					LOG_ERR("[%s:%d] player[%lu] mail_dbinfo__unpack failed, mail_id:%lu", __FUNCTION__, __LINE__, extern_data->player_id, mail_id);
					continue;
				}
				mail_details.push_back(detail);

				if (!(detail->n_attach > 0 && extract == 0))
				{
					del_ids.push_back(mail_id);
				}
			}

			free_query(res);
			res = NULL;

			//删除邮件
			size_t del_size = del_ids.size();
			if (del_size > 0)
			{
				len = sprintf(sql, "delete from mail where player_id = %lu and mail_id in (", extern_data->player_id);
				p = sql + len;

				for (size_t i = 0; i < del_size; ++i)
				{
					uint64_t mail_id = del_ids[i];
					if (i == del_size - 1)
					{
						len = sprintf(p, "%lu);", mail_id);
					}
					else
					{
						len = sprintf(p, "%lu,", mail_id);
					}
					p += len;
				}
				query(sql, 1, NULL);
			}
		}
	} while(0);

	if (res)
	{
		free_query(res);
		res = NULL;
	}

	MailMultiAnswer resp;
	mail_multi_answer__init(&resp);

	resp.result = ret;
	resp.mail_id = &del_ids[0];
	resp.n_mail_id = del_ids.size();

	fast_send_msg(&connecter, extern_data, MSG_ID_MAIL_DEL_ANSWER, mail_multi_answer__pack, resp);

	for (std::vector<MailDBInfo*>::iterator iter = mail_details.begin(); iter != mail_details.end(); ++iter)
	{
		mail_dbinfo__free_unpacked(*iter, NULL);
	}

	return 0;
}

