# Omer Zucker
# 200876548

#!/bin/bash
printf "Number of files in the directory that end with .txt is "
find $1 -name '*.txt' | wc -l