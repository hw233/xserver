INCLUDES = -I$(SRCROOT)/thirdlib/libevent/include/  -I$(SRCROOT)/thirdlib/libevent/   -I$(SRCROOT)/comm_include  -I$(SRCROOT)/comm_lib -I$(SRCROOT)/proto/ -I$(SRCROOT)/comm_game/
LIBPATH = -L$(SRCROOT)/thirdlib/libevent/.libs/  -L$(SRCROOT)/proto/
MYSQL_CFLAGS = -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing -g
