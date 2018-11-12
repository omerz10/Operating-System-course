# Omer Zucker
# 200876548

#!/bin/sh

SUM=0;
while IFS='' read -r line || [[ -n "$line" ]]; do
	if [[ ${line} = *"$1"* ]] ; then
		echo "$line"
		read -r -a arr <<< "$line"
		let "SUM += ${arr[2]}"
	fi
done < "$2"
echo "Total balance:$SUM"
