library("irace")

parametri <- readParameters("parameters.txt")
scenarij <- readScenario("scenario.txt")
irace(scenario = scenarij, parameters=parametri)

testing.main(logFile="irace.Rdata")

