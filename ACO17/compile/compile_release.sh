/usr/bin/clang++ -O2 -c main.cpp -o obj/release/main.o
/usr/bin/clang++ -O2 -c loadProblem.cpp -o obj/release/loadProblem.o
clang++ obj/release/main.o obj/release/loadProblem.o -o bin/release/ACO17
