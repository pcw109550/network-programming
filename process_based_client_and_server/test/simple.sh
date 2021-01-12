#!/bin/bash
set -e

mkdir -p .tmp/client
mkdir -p .tmp/server
cp bin/client .tmp/client/client
cp bin/server .tmp/server/server

cd .tmp/server
./server 54321 &
sleep 1
SERVER_PID=$!
cd ../..

cd .tmp/client
randomstr="$(dd if=/dev/urandom bs=$RANDOM count=1)"
echo $randomstr > input.txt
./client 127.0.0.1 54321 input.txt
cd ../..

sleep 1

kill -9 ${SERVER_PID} 2> /dev/null || true

set +e
diff .tmp/server/input.txt .tmp/client/input.txt
RESULT=$?
rm -rf .tmp

if [ $RESULT -eq 0 ]; then
  echo "Test Passed."
else
  echo "Test Failed."
  exit 1
fi
