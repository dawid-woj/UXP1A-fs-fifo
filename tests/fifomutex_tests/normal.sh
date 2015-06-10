echo Tworze fifoinit
./openinit
INIT_PID=$!
sleep 3s
./process &
./process &
./process &
sleep 1s
./process &
./process &
PID1=$!
wait $PID1 
echo Zamykam init
sleep 2s
./closeinit
