echo Skrypt sprawdza dzialanie procesow gdy nie ma procesu init, zakladajac brak obslugi zwracanych wartosci f-cji 'lock()' przez uzytkownika
echo Tworze 4 procesy
./process -u &
PID1=$!
./process -u &
PID2=$!
./process -u &
PID3=$!
./process -u &
PID4=$!
wait $PID1 
wait $PID2
wait $PID3
wait $PID4
echo Koniec testu, oczekiwany wynik\: po 4 probach proces konczy dzialanie, f-cja "lock()" zwraca -1, f-cja "unlock()" zwraca -1 i wypisuje \"zle dane\"
