load("irace.Rdata")
iraceResults$parameters
iraceResults$experiments
iraceResults$allConfigurations

library("irace")
parallelCoordinatesPlot(configurations=getFinalElites(iraceResults), parameters=iraceResults$parameters, hierarchy=FALSE, filename="pcoordinates")
parameterFrequency(configurations=iraceResults$allConfigurations, parameters=iraceResults$parameters, filename="frequency")

library(ggplot2)
source("utils/boxplot.R")
boxplot_train(iraceResults, filename="boxplot_train.png")
boxplot_test(iraceResults, filename="boxplot.png")
