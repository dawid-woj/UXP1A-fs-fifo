#!/bin/bash
echo 'Test normalne podpiecie 5 procesow nastepujaco po sobie (lock, usleep(x), unlock) do inita'
echo 'Tworze fifoinit'
../../bin/simplefs_init &
INIT_PID=$!
#sleep 1s
echo 'Uruchamiam 5 procesow po kolei'
./process 400
./process 400
./process 400
./process 400
./process 400
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
