echo Tworze fifoinit
../../bin/simplefs_init &
INIT_PID=$!
echo Tworze 4 procesy
./process &
PID1=$!
./process &
PID2=$!
./process &
PID3=$!
./process &
PID4=$!
sleep 3s
kill $PID1
wait $PID1 
wait $PID2
wait $PID3
wait $PID4
echo Zamykam init
sleep 2s
./closeinit
