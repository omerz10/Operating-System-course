# Omer Zucker
# 200876548

#!/bin/sh

for item in "$1"/*; do
	if test -f "${item}" 
	then
		echo "$(basename $item) is a file"
	else
		echo "$(basename $item) is a directory"
	fi
done