#!/bin/bash
echo 'Test init fifo,a potem od razu unmount'
echo 'Tworze fifoinit'
../../bin/simplefs_init &
#sleep 1s
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
