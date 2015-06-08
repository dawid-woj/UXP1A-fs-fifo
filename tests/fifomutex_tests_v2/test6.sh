#!/bin/bash
echo 'Test kilkukrotne wywolanie inita'
echo 'Tworze fifoinit x3'
../../bin/simplefs_init &
../../bin/simplefs_init &
../../bin/simplefs_init &
sleep 3s
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
