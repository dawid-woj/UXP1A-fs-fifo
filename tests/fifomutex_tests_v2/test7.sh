#!/bin/bash
echo 'Test podpiecie 1 procesu (lock, usleep(x), unlock) do niesitniejacego inita'
echo 'Uruchamiam proces'
./process 5000 &
PID=$!
wait $PID
echo 'KONIEC TESTU'
