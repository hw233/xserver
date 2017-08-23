function handle_login_answer(msg_body, player_id)
    local msg_data = login_pb.LoginAnswer()
    msg_data:ParseFromString(msg_body)
    print(msg_data.result, msg_data.openid)
end

function init_login()
--    msg_handle[10001] = function(msg_body, player_id)
--	local msg_data = login_pb.LoginAnswer()
--	msg_data:ParseFromString(msg_body)
--	print(msg_data.result, msg_data.openid)
    --    end
    msg_handle[10001] = handle_login_answer
end

init_login()
