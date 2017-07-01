#ifndef _CMD_IMPL_H__
#define _CMD_IMPL_H__

class cmd_impl
{
public:
	int do_cmd_main(int argc, char *argv[]);
private:
	int show_help();
	int move_main(int argc, char *argv[]);	
};

#endif
