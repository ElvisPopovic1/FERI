#!/bin/bash
#good ones 
#./bin/Release/ACO16 -d inputData/Londrina100n.car -r1 0.05 -r2 0.1 -a1 1.1 -b1 2.0 -a2 1.1 -b2 2.0 -na 400 -ni 10000 -sn 0 -nexec 1 -nfav 100 -st 100 --write 1397 1368
#./bin/Release/ACO16 -d inputData/Londrina100n.car -r1 0.05 -r2 0.1 -a1 1.05 -b1 2.0 -a2 1.05 -b2 2.0 -na 500 -ni 10000 -sn 0 -nexec 11 -nfav 100 -st 300 --write 1342
#./bin/Release/ACO16 -d inputData/Londrina100n.car -r1 0.05 -r2 0.1 -a1 1.05 -b1 2.0 -a2 1.05 -b2 2.0 -na 500 -ni 10000 -sn 0 -pnr 0.00002 -pcr 0.0005 -nexec 11 -nfav 100 -st 300 --write 

# no write
./bin/Release/ACO16 -d inputData/BrasilRJ14n.car -r1 0.08 -r2 0.08 -a1 2.5 -b1 2.5 -a2 2.5 -b2 2.5 -na 2600 -ni 1000 -sn 0 -pnr 0.02 -pcr 0.05 -nexec 51 -nfav -1 -st 20 --write 