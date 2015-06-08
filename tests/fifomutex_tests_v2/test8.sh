#!/bin/bash
echo 'Test podpiecie 4 procesow (lock, usleep(x), unlock) do niesitniejacego inita'
echo 'Uruchamiam 4 procesy'
./process 1000 &
PID1=$!
./process 1000 &
PID2=$!
./process 1000 &
PID3=$!
./process 1000 &
PID4=$!
wait $PID1
wait $PID2
wait $PID3
wait $PID4
echo 'KONIEC TESTU'
