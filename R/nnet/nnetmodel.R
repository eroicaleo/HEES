library(neuralnet)

NormalizeTrainData <- function(x) {
  (x - min(x)) / (max(x) - min(x))
}

NormalizeTestData <- function(x, min, max) {
  (x - min) / (max - min)
}

# Read the training and testing data
hees.trn <- read.delim("data/trn_10.txt.0100", header = FALSE, sep = "")
names(hees.trn) <- c("solar", "endE", "len", "initE")

hees.tst <- read.delim("data/tst_10.txt.0100", header = FALSE, sep = "")
names(hees.tst) <- c("solar", "endE", "len", "initE")

# Drop the len column
hees.trn <- hees.trn[c("solar", "endE", "initE")]
hees.tst <- hees.tst[c("solar", "endE", "initE")]

# Unique it
hees.trn.unique <- unique(hees.trn)

# Get the min and max, so to revert test data back
trn.min <- lapply(hees.trn.unique, min)
trn.max <- lapply(hees.trn.unique, max)

# Normalize the data
hees.trn.normal <- as.data.frame(lapply(hees.trn.unique, NormalizeTrainData))
hees.tst.normal <- as.data.frame(mapply(NormalizeTestData, hees.tst, trn.min, trn.max))

summary(hees.trn.normal)
tail(hees.trn.normal)
head(hees.trn.normal)

# Train the model
hees.model <- neuralnet(endE ~ solar + initE,
                        data = hees.trn.normal,
                        hidden = 7)

plot(hees.model)

# Compute the prediction error
hees.model.results <- compute(hees.model, hees.tst.normal[c("solar", "initE")])
hees.tst.pred <- hees.model.results$net.result

cor(hees.tst.normal$endE, hees.tst.pred)
plot(hees.tst.normal$endE, hees.tst.pred)
