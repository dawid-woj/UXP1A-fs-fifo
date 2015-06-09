#!/bin/bash
echo 'Test init fifo,a potem od razu unmount'
echo 'Tworze fifoinit'
./openinit 
#sleep 1s
echo 'Zamykam init'
./closeinit
echo 'KONIEC TESTU'
