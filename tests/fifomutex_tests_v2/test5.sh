#!/bin/bash
echo 'Test kilkukrotne wywolanie unmount'
echo 'Tworze fifoinit'
./openinit 
INIT_PID=$!
sleep 1s
echo 'Unmount x3'
./closeinit
./closeinit
./closeinit
