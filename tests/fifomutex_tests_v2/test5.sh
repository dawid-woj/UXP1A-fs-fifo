#!/bin/bash
echo 'Test kilkukrotne wywolanie unmount'
echo 'Tworze fifoinit'
../../bin/simplefs_init &
INIT_PID=$!
sleep 1s
echo 'Unmount x3'
./closeinit
./closeinit
./closeinit
