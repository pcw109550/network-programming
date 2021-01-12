#!/bin/bash

# https://stackoverflow.com/questions/40926789/create-a-file-of-a-specific-size-with-random-printable-strings-in-bash

value_size=8000 # 67108864 # 32MB
echo "value_size=$value_size"

cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $value_size | head -n 1 | tr -d '\n' > ./bigfile_value

printf "ee324=" > ./8kvalue.txt
cat bigfile_value >> 8kvalue.txt
rm bigfile_value