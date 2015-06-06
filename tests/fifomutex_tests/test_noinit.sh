echo Skrypt sprawdza dzialanie procesow gdy nie ma procesu init, zakladajac obsluge zwracanych wartosci f-cji 'lock()' przez uzytkownika
echo Tworze 4 procesy
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
echo Koniec testu, oczekiwany wynik\: po 4 probach proces konczy dzialanie, a f-cja lock zwraca -1
