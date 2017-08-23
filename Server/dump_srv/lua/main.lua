package.path = string.format("%s;%s", package.path, "./lua/?.lua;./lua/protobuf/?.lua;./lua/Protol/?.lua")
--print("package.path: " .. package.path)
package.cpath = string.format("%s;%s", package.cpath, "./lua/protobuf/?.so")
--print("package.cpath: " .. package.cpath)
require "login_pb"

msg_handle = {}

function dispatch_message(msg_id, msg_body, player_id)
--    print("msg_id:" .. msg_id)
    if (msg_handle[msg_id] ~= nil) then
	msg_handle[msg_id](msg_body, player_id)
    end
end

require "login"
