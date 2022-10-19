/usr/bin/clang++ -O2 -c main.cpp -o obj/release/main.o
/usr/bin/clang++ -O2 -c loadProblem.cpp -o obj/release/loadProblem.o
/usr/bin/clang++ -O2 -c display.cpp -o obj/release/display.o
/usr/bin/clang++ -O2 -c solution.cpp -o obj/release/solution.o
/usr/bin/clang++ -O2 -c setParameters.cpp -o obj/release/setParameters.o
/usr/bin/clang++ -ldl obj/release/main.o obj/release/loadProblem.o obj/release/solution.o obj/release/setParameters.o obj/release/display.o -o bin/release/ACO17

# dynamic library compilation, fPIC position independent code - old algo
/usr/bin/clang++ -Wall -fPIC -c oldAlgo/createSolution.cpp -o obj/release/createSolution_o.o 
/usr/bin/clang++ -Wall -fPIC -c oldAlgo/ant_run.cpp -o obj/release/ant_run_o.o 
/usr/bin/clang++ -Wall -fPIC -c oldAlgo/probability.cpp -o obj/release/probability_o.o 
/usr/bin/clang++ -shared obj/release/createSolution_o.o obj/release/probability_o.o obj/release/ant_run_o.o \
-o bin/release/createSolution_old.so

# dynamic library compilation, fPIC position independent code - new algo
/usr/bin/clang++ -Wall -fPIC -c newAlgo/createSolution.cpp -o obj/release/createSolution_n.o 
/usr/bin/clang++ -Wall -fPIC -c newAlgo/ant_run.cpp -o obj/release/ant_run_n.o 
/usr/bin/clang++ -Wall -fPIC -c newAlgo/probability.cpp -o obj/release/probability_n.o 
/usr/bin/clang++ -shared obj/release/createSolution_n.o obj/release/probability_n.o obj/release/ant_run_n.o \
-o bin/release/createSolution_new.so