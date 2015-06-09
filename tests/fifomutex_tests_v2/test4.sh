#!/bin/bash
echo 'Test podpiecie 3 procesow (lock, usleep(x), unlock) do inita, potem umount, potem proba kolejnych 3 procesow'
echo 'Tworze fifoinit'
./openinit 
INIT_PID=$!
#sleep 1s
echo 'Uruchamiam 3 procesy'
./process 5000 &
PID1=$!
./process 4000 &
PID2=$!
./process 2000 &
PID3=$!
echo 'Zamykam init'
./closeinit
echo 'Uruchamiam 3 procesy'
./process 5000 &
PID4=$!
./process 4000 &
PID5=$!
./process 2000 &
PID6=$!
wait $PID1
wait $PID2
wait $PID3
wait $PID4
wait $PID5
wait $PID6
echo 'KONIEC TESTU'
