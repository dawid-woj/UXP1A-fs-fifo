echo Skrypt sprawdza typowy przypadek\: tworze init, on swoja kolejke, procesy sie dolaczaja i koncza sie poprawnie po wykonaniu "unlink", na koniec procesy usuwaja swoje "fifo" z systemu plikow linuxa
echo Tworze fifoinit
../../bin/simplefs_init &
INIT_PID=$!
echo Tworze 4 procesy
sleep 3s
./process &
PID1=$!
./process &
PID2=$!
./process &
PID3=$!
./process &
PID4=$!
wait $PID1 
wait $PID2
wait $PID3
wait $PID4
echo Zamykam init
sleep 2s
./closeinit
