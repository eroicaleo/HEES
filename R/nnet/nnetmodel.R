library(neuralnet)

NormalizeTrainData <- function(x) {
  (x - min(x)) / (max(x) - min(x))
}

NormalizeTestData <- function(x, min, max) {
  (x - min) / (max - min)
}

DeNormalizeData <- function(x, min, max) {
  min + (max - min) * x
}

# Read the training and testing data
hees.trn <- read.delim("data/trn_10.txt.0100", header = FALSE, sep = "")
names(hees.trn) <- c("solar", "initE", "len", "endE")

hees.tst <- read.delim("data/tst_10.txt.0100", header = FALSE, sep = "")
names(hees.tst) <- c("solar", "initE", "len", "endE")

# Drop the len column
hees.trn <- hees.trn[c("solar", "initE", "endE")]
hees.tst <- hees.tst[c("solar", "initE", "endE")]

# Drop some training data
threshold <- 1.0
hees.trn <- hees.trn[hees.trn$solar > threshold,]
hees.tst <- hees.tst[hees.tst$solar > threshold,]

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
hees.tst.pred.normal <- hees.model.results$net.result
hees.tst.pred <- DeNormalizeData(hees.tst.pred.normal, trn.min$endE, trn.max$endE)

hees.model.results.trn <- compute(hees.model, hees.trn.normal[c("solar", "initE")])
hees.trn.pred.normal <- hees.model.results.trn$net.result
hees.trn.pred.error.normal <- hees.trn.pred.normal - hees.trn.normal$endE

cor(hees.tst.normal$endE, hees.tst.pred.normal)
plot(hees.tst.normal$endE, hees.tst.pred.normal)

cor(hees.tst$endE, hees.tst.pred)
plot(hees.tst$endE, hees.tst.pred)

hees.tst.pred.error <- hees.tst.pred - hees.tst$endE
hist(hees.tst.pred.error)
plot(hees.tst.pred.error, lty = 1, col = "red", type = "l")
