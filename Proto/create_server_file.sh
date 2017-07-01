#!/bin/sh

cd `dirname $0`

cur_dir=`pwd`

output_dir=$(dirname $(pwd))/Server/proto/


cd $output_dir
if [ $(ls | grep "\.h$" | wc -l) -gt 0 ]; then
	#echo "remove all .h file"
	rm *.h
fi
if [ $(ls | grep "\.c" | wc -l) -gt 0 ]; then
	#echo "remove all .c file"
	rm *.c
fi

cd $cur_dir
cp *.h $output_dir

for dir in $cur_dir $output_dir;
do
	file_num=$(ls $dir | grep "[a-z A-Z 0-9 _]*.proto$" | wc -l)
	if [ $file_num -eq 0 ]; then
		echo "$dir continue"
		continue
	fi

	cd $dir
	proto_file_list=$(ls $dir | grep "[a-z A-Z 0-9 _]*.proto$")
	#if [ x"" = x$proto_file_list ]; then
	#	continue
	#fi

	proto_file_str=""
	while read name
	do
		proto_file_str=${proto_file_str}" ${name}"
	done <<!
	${proto_file_list}
!

	protoc-c --c_out=${output_dir} ${proto_file_str}
done

cd $cur_dir

