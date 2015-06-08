#!/bin/bash
echo 'Test normalne podpiecie 1 procesu (lock, usleep(x), unlock) do inita'
echo 'Tworze fifoinit'
../../bin/simplefs_init &
INIT_PID=$!
#sleep 1s
echo 'Uruchamiam proces'
./process 400 &
PID=$!
wait $PID
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
