/usr/bin/clang++ -g -c main.cpp -o obj/debug/main.o
/usr/bin/clang++ -g -c loadProblem.cpp -o obj/debug/loadProblem.o
clang++ -fsanitize=address -fno-omit-frame-pointer -fdiagnostics-color=always obj/debug/main.o obj/debug/loadProblem.o -o bin/debug/ACO17

