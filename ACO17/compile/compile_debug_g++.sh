/usr/bin/g++ -D DEBUG -Wall -fexceptions -g  -c "main.cpp" -o obj/debug/main.o
/usr/bin/g++ -Wall -fexceptions -g  -c "loadProblem.cpp" -o obj/debug/loadProblem.o
/usr/bin/g++ -Wall -fexceptions -g  -c "display.cpp" -o obj/debug/display.o
/usr/bin/g++ -Wall -fexceptions -g  -c "solution.cpp" -o obj/debug/solution.o
/usr/bin/g++ -Wall -fexceptions -g  -c "setParameters.cpp" -o obj/debug/setParameters.o
/usr/bin/g++ obj/debug/main.o obj/debug/loadProblem.o obj/debug/solution.o obj/debug/setParameters.o obj/debug/display.o -ldl -o bin/debug/ACO17

# dynamic library compilation, fPIC position independent code - old algo
/usr/bin/g++ -Wall -g -fPIC -c oldAlgo/createSolution.cpp -o obj/debug/createSolution_o.o 
/usr/bin/g++ -Wall -g -fPIC -c oldAlgo/ant_run.cpp -o obj/debug/ant_run_o.o 
/usr/bin/g++ -Wall -g -fPIC -c oldAlgo/probability.cpp -o obj/debug/probability_o.o 
/usr/bin/g++ -shared obj/debug/createSolution_o.o obj/debug/probability_o.o obj/debug/ant_run_o.o \
-o bin/debug/createSolution_old.so

# dynamic library compilation, fPIC position independent code - new algo
/usr/bin/g++ -Wall -g -fPIC -c newAlgo/createSolution.cpp -o obj/debug/createSolution_n.o 
/usr/bin/g++ -Wall -g -fPIC -c newAlgo/ant_run.cpp -o obj/debug/ant_run_n.o 
/usr/bin/g++ -Wall -g -fPIC -c newAlgo/probability.cpp -o obj/debug/probability_n.o 
/usr/bin/g++ -shared obj/debug/createSolution_n.o obj/debug/probability_n.o obj/debug/ant_run_n.o \
-o bin/debug/createSolution_new.so