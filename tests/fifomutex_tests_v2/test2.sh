#!/bin/bash
echo 'Test normalne podpiecie 4 procesow (lock, usleep(x), unlock) do inita'
echo 'Tworze fifoinit'
../../bin/simplefs_init &
INIT_PID=$!
#sleep 1s
echo 'Uruchamiam 4 procesy'
./process 300 &
PID1=$!
./process 300 &
PID2=$!
./process 400 &
PID3=$!
./process 500 &
PID4=$!
wait $PID1
wait $PID2
wait $PID3
wait $PID4
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
