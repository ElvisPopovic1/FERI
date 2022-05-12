#ifndef SUPERVISOR_H
#define SUPERVISOR_H

#include "ACO.h"

void beginSupervisor(const char* filename);
void endSupervisor();
void writeHeaders();

void writeFinalPrice();

void writeProblemData();
void writeInitData();
void writeResult(const char* filename);
void writeBestData(uint iteration, std::chrono::duration<double> cur_time, uint startNode, ant *bestAnt, bool solutionStall);

#endif