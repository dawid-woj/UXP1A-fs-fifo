#!/bin/bash
echo 'Test podpiecie 4 procesow (lock, usleep(x), unlock) do inita i ubicie jednego w trakcie'
echo 'Tworze fifoinit'
./openinit 
INIT_PID=$!
sleep 1s
echo 'Tworze 4 procesy i ubijam jakiegos'
./process 2000000 &
PID1=$!
./process 2000000 &
PID2=$!
./process 2000000 &
PID3=$!
./process 2000000 &
PID4=$!
kill $PID2
wait $PID1 
wait $PID2
wait $PID3
wait $PID4
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
