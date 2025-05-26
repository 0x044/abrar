g++ -v fleetXpress.cpp fleet.cpp utilityFunctions.cpp dateTimeFunctions.cpp -o fleetapp -lcrypto -lsqlite3
./fleetapp
rm -f fleetapp
rm -f fleet.db