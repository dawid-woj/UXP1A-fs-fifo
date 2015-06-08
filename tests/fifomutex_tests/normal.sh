echo Tworze fifoinit
../../bin/simplefs_init &
INIT_PID=$!
echo Tworze 1 proces
sleep 3s
./process &
PID1=$!
wait $PID1 
echo Zamykam init
sleep 2s
./closeinit
