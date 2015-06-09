#!/bin/bash
echo 'Test kilkukrotne wywolanie inita'
echo 'Tworze fifoinit x3'
./openinit &
./openinit 
./openinit 
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
