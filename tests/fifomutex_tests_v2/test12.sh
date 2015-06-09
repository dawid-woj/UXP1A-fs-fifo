#!/bin/bash
echo 'Test podpiecie 3 procesow robiacych rozne rzeczy do inita'
echo 'Tworze fifoinit'
./openinit 
INIT_PID=$!
echo 'Tworze 3 procesy'
./proc1 &
PID1=$!
./proc2 &
PID2=$!
./proc3 &
PID3=$!
wait $PID1 
wait $PID2
wait $PID3
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
