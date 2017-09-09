package main

import (
	"fmt"
	"strconv"
//	"io/ioutil"
	"regexp"
	"flag"
	"bufio"
	"os"
)

func check(e error) {
	if e != nil {
		panic(e)
	}
}

type player struct {
	fd uint32
	openid uint32
	playerid uint64
	name string
}

var all_msgid map[uint64]string
var all_players map[uint32]*player
var re_player *regexp.Regexp
var re_logout *regexp.Regexp
var re_recvmsg *regexp.Regexp
var re_sendmsg *regexp.Regexp
func main() {
	all_msgid = make(map[uint64]string)	
	all_players = make(map[uint32]*player)
	re_player, _ = regexp.Compile("\\[handle_client_enter.*openid\\[([0-9]*)\\] playerid\\[([0-9]*)\\] fd\\[([0-9]*)\\]")
	//re_logout, _ = regexp.Compile("remove_listen_callback_event: close connect from \\[[0-9a-z]*\\]([0-9]*)")
	re_logout, _ = regexp.Compile("remove_listen_callback_event: close connect from fd \\[[0-9a-z]*\\]([0-9]*)")
	re_recvmsg, _ = regexp.Compile("int conn_node_base::get_one_buf\\(\\) ([0-9]*): get msg\\[([0-9]*)\\] len.*")
	re_sendmsg, _ = regexp.Compile("\\[virtual int conn_node_client::send_one_msg\\(PROTO_HEAD\\*, uint8_t\\): [0-9]*\\]: fd: ([0-9]*): send msg\\[([0-9]*)\\] len.*")
	r, _ := regexp.Compile("(^[0-9]{8} [0-9:.]{12}) ([A-Z]*    .*)")
	//	teststr := "20170908 13:41:00.025 DEBUG    [handle_client_enter:100] openid[111284] playerid[8589937986] fd[21]"
	//	teststr2 := "20170908 13"
	//20170908 13:42:11.544 DEBUG    remove_listen_callback_event: close connect from fd [0x7f5f6fb8b010]21
	//[virtual int conn_node_client::send_one_msg(PROTO_HEAD*, uint8_t): 250]: fd: 21: send msg[10102] len[41], seq[0]	
	//20170908 15:55:29.117 DEBUG    int conn_node_base::get_one_buf() 20: get msg[1000] len[59], seq[65], max_len [59]
	//
	//	matchpos := r.FindStringSubmatchIndex(teststr)
	//	fmt.Println(matchpos)
	//	fmt.Printf("match pos len = %d\n", len(matchpos))
	//
	//	fmt.Printf("|%s| |%s|", teststr[matchpos[2]:matchpos[3]], teststr[matchpos[4]:matchpos[5]])
	//
	//	matchpos := r.FindStringSubmatchIndex(teststr)
	//	fmt.Println(matchpos)
	//	fmt.Printf("match pos len = %d\n", len(matchpos))

	logfile := flag.String("l", "conn_srv.0", "log file name")
	msgfile := flag.String("m", "msgid.h", "msgid file name")	
	flag.Parse()

	load_msg_id_file(msgfile)

	f , err := os.Open(*logfile)
	check(err)

	br := bufio.NewReader(f)
	for {
		line , err := br.ReadString('\n')
		if err != nil {
			break
		}
		matchpos := r.FindStringSubmatchIndex(line)
		if len(matchpos) != 6 {
			continue
		}

		begin := matchpos[4]
		end := matchpos[5]
		pre := line[matchpos[2]:matchpos[3]]

		if check_recv_msg(line[begin:end], pre) {
			continue
		}

		if check_send_msg(line[begin:end], pre) {
			continue
		}

		if check_player_login(line[begin:end], pre) {
			continue
		}
		if check_player_logout(line[begin:end], pre) {
			continue
		}
		
//		fmt.Println(line[begin:end])
	}

	f.Close()

//	for _, player := range all_players {
//		fmt.Printf("openid[%d] playerid[%d] fd[%d]\n", player.openid, player.playerid, player.fd)
//	}
}

func load_msg_id_file(msgfile *string) {
	re_msg, _ := regexp.Compile("#define[ 	]*[^ 	]*[ 	]*([0-9]*)[ 	]*//(.*)")	
	f , err := os.Open(*msgfile)
	check(err)
	defer f.Close()

	i := 0

	br := bufio.NewReader(f)
	for {
		line , err := br.ReadString('\n')
		if err != nil {
			break
		}
		i = i + 1
		t := re_msg.FindStringSubmatchIndex(line)
		if len(t) != 6 {
//			fmt.Printf("%d: len = %d, msg = %s\n", i, len(t), line)
			continue
		}
		strmsgid := line[t[2]:t[3]]
		comment := line[t[4]:t[5]]		
		imsgid, _ := strconv.ParseUint(strmsgid, 10, 64)
		all_msgid[imsgid] = comment
//		fmt.Printf("%d: get msgid[%s][%d][%s]\n", i, strmsgid, imsgid, comment)
	}	
}

func check_player_login(s string, pre string) bool {
	t := re_player.FindStringSubmatchIndex(s)

	if len(t) == 8 {
//		l1 := line[begin:end]
		openid := s[t[2]:t[3]]
		playerid := s[t[4]:t[5]]
		fd := s[t[6]:t[7]]

		var i uint64
		var player player
		i, _ = strconv.ParseUint(openid, 10, 32)
		player.openid = uint32(i)			
		player.playerid, _ = strconv.ParseUint(playerid, 10, 64)
		i, _ = strconv.ParseUint(fd, 10, 32)
		player.fd = uint32(i)

		all_players[player.fd] = &player;

		fmt.Printf("%s:	player[%d] login fd[%d]\n", pre, player.playerid, player.fd)		
		return true
	}
	return false
//	fmt.Println("ttt")
}

func check_player_logout(s string, pre string) bool {
	t := re_logout.FindStringSubmatchIndex(s)

	if len(t) == 4 {
		strfd := s[t[2]:t[3]]
		var i uint64
		i, _ = strconv.ParseUint(strfd, 10, 32)
		ifd := uint32(i)
		t_player, ok := all_players[ifd]		
		if !ok {
//			fmt.Printf("%s:	err logout, can not find fd[%d]\n", pre, ifd)
//			os.Exit(0)			
			return true
		}
		
		fmt.Printf("%s:	player[%d] logout fd[%d]\n", pre, t_player.playerid, ifd)
		delete(all_players, ifd)
	}
	return false
}

func check_send_msg(s string, pre string) bool {
	t := re_sendmsg.FindStringSubmatchIndex(s)

	if len(t) == 6 {
		strfd := s[t[2]:t[3]]
		strmsgid := s[t[4]:t[5]]

		var i uint64
		i, _ = strconv.ParseUint(strfd, 10, 32)
		ifd := uint32(i)

		imsgid, _ := strconv.ParseUint(strmsgid, 10, 64)

		t_player, ok := all_players[ifd]		
		if !ok {
//			fmt.Printf("err send, can not find fd[%d] msg[%d]\n", ifd, imsgid)
//			os.Exit(0)			
			return true
		}
		t_msg, ok := all_msgid[imsgid]
		if !ok {
//			fmt.Printf("recv from player[%d] msg[%d]\n", t_player.playerid, imsgid)					
			return true
		}

		fmt.Printf("%s:	send  to  player[%d] msg[%d][%s]\n", pre, t_player.playerid, imsgid, t_msg)		

		return true
	}
	return false
}

func check_recv_msg(s string, pre string) bool {
	t := re_recvmsg.FindStringSubmatchIndex(s)

	if len(t) == 6 {
		strfd := s[t[2]:t[3]]
		strmsgid := s[t[4]:t[5]]

		var i uint64
		i, _ = strconv.ParseUint(strfd, 10, 32)
		ifd := uint32(i)

		imsgid, _ := strconv.ParseUint(strmsgid, 10, 64)

		t_player, ok := all_players[ifd]
		if !ok {
//			fmt.Printf("err recv, can not find fd[%d] msg[%d]\n", ifd, imsgid)
//			os.Exit(0)
			return true
		}
		t_msg, ok := all_msgid[imsgid]
		if !ok {
//			fmt.Printf("recv from player[%d] msg[%d]\n", t_player.playerid, imsgid)					
			return true
		}

		fmt.Printf("%s:	recv from player[%d] msg[%d][%s]\n", pre, t_player.playerid, imsgid, t_msg)		

		return true
	}
	return false
}
