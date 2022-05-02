#!/bin/bash
#./bin/Debug/ACO16 -d inputData/Teresina200n.car -r1 0.05 -r2 0.05 -a1 1.5 -b1 1.5 -a2 1.5 -b2 1.5 -na 150 -ni 200 -sn 0 --write

# no write
./bin/Release/ACO16 -d inputData/Curitiba300n.car -r1 0.2 -r2 0.15 -a1 2.4 -b1 3.7 -a2 2.3 -b2 2.3 -na 600 -ni 1000 -sn 0 -nexec 1 -nfav 30 -st 5 --write