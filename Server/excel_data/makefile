all: excel_data

excel_data: proto.sproto
	rm -f 1.spb
	./spc.sh proto.sproto 1.spb
	./generate_head 1.spb  > excel_data.h

generate_head: generate_head.c ../comm_lib/sproto.cpp
	g++ -g -O0 -I./ -o generate_head generate_head.c ../comm_lib/sproto.cpp

init_id:
	sed 's/	/\n/g' 1.log > 2.log
	awk -f init_id.awk < 2.log

