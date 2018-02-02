//#include "player_manager.h"
#include "game_event.h"
#include <signal.h>

static void  srv_signal_handler(int signo);

typedef struct {
	int     signo;
	char   *signame;
	char   *name;
	void  (*handler)(int signo);
}srv_signal_t;


static const srv_signal_t  signals[] = {
	{ SIGUSR1, (char*)"SIGUSR1", (char*)"reopen",srv_signal_handler },
	{ SIGUSR2,(char*)"SIGUSR2",(char*)"",srv_signal_handler },

	{ SIGHUP,(char*)"SIGHUP",(char*)"reload",srv_signal_handler },	
	{ SIGWINCH, (char*)"SIGWINCH", (char*)"",srv_signal_handler },
	{ SIGTERM, (char*)"SIGTERM", (char*)"stop",srv_signal_handler },
	{ SIGQUIT, (char*)"SIGQUIT", (char*)"quit",srv_signal_handler },
	
	{ SIGALRM,  (char*)"SIGALRM", (char*)"", srv_signal_handler },
	{ SIGINT, (char*)"SIGINT", (char*)"", srv_signal_handler },
	{ SIGIO, (char*)"SIGIO", (char*)"", srv_signal_handler },
	{ SIGCHLD, (char*)"SIGCHLD", (char*)"", srv_signal_handler },
	{ SIGSYS, (char*)"SIGSYS, SIG_IGN", (char*)"", SIG_IGN },
	{ SIGPIPE, (char*)"SIGPIPE, SIG_IGN", (char*)"", SIG_IGN },
	{ 0, NULL, (char*)"", NULL }
};


void srv_signal_handler(int signo) {
	const srv_signal_t* sig = NULL;
	bool isOK = false;
	for (sig = signals; sig->signo != 0; sig++) {
		if (sig->signo == signo) {
			isOK = true;
			break;
		}
	}

	if (!isOK) {
		LOG_ERR("[%s : %d]: signal not register, signo: %d", __FUNCTION__, __LINE__, signo);
		return;
	}

	switch (signo)
	{
	case SIGUSR1:
	case SIGUSR2:
		{
			// std::map<uint64_t, player_struct *>::iterator it = player_manager_all_players_id.begin();
			// for (; it!=player_manager_all_players_id.end(); ++it) {
			// 	player_struct* player = it->second;
			// 	if (!player)
			// 		continue;
//				player->process_kick_player();
//			}
		}break;
	case SIGHUP:
	case SIGWINCH:
	case SIGQUIT:
	case SIGALRM:
	case SIGINT:
	case SIGIO:
	case SIGCHLD:
	case SIGSYS:
	case SIGPIPE:
		{

		}break;
	case SIGTERM:
		return;

	default:
		break;
	}

	

	exit(0);
}

int init_signals() {
	const srv_signal_t      *sig;
	struct sigaction   sa;

	for (sig = signals; sig->signo != 0; sig++) {
		memset(&sa, 0, sizeof(struct sigaction));
		sa.sa_handler = sig->handler;
		sigemptyset(&sa.sa_mask);
		if (sigaction(sig->signo, &sa, NULL) == -1) {
			return -1;
		}
	}

	return 0;
}



