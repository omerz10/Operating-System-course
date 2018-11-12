# Omer Zucker
# 200876548

#!/bin/sh

if [ $# -ne 1 ] ; then
	echo "error: only one argument is allowed"
	exit 1;
fi

if [ ! -f $1 ] ; then
	echo "error: there is no such file"
	exit 1;
else
	if [ ! -d safe_rm_dir ] ; then
		mkdir -p safe_rm_dir;
fi
	cp $1 safe_rm_dir/
	rm $1
	echo "done!"
fi
