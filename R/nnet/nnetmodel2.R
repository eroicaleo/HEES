library(nnet)

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

# change the initE and endE by substracting the initE
hees.trn <- transform(hees.trn, initE = initE - 125.0, endE = endE - 125.0)
hees.tst <- transform(hees.tst, initE = initE - 125.0, endE = endE - 125.0)

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
hees.model <- nnet(endE ~ solar + initE,
                        data = hees.trn.normal,
                        maxit = 5000,
                        size = 7,
                        decay = 0,
                        abstol = 1e-15,
                        reltol = 1e-15,
                   )

plot(hees.model)

# predict the prediction error
hees.model.results <- predict(hees.model, hees.tst.normal[c("solar", "initE")])
hees.tst.pred.normal <- hees.model.results$net.result
hees.tst.pred <- DeNormalizeData(hees.tst.pred.normal, trn.min$endE, trn.max$endE)

hees.model.results.trn <- predict(hees.model, hees.trn.normal[c("solar", "initE")])
hees.trn.pred.normal <- hees.model.results.trn
hees.trn.pred.error.normal <- hees.trn.pred.normal - hees.trn.normal$endE

cor(hees.tst.normal$endE, hees.tst.pred.normal)
plot(hees.tst.normal$endE, hees.tst.pred.normal)

cor(hees.tst$endE, hees.tst.pred)
plot(hees.tst$endE, hees.tst.pred)

hees.tst.pred.error <- hees.tst.pred - hees.tst$endE
hist(hees.tst.pred.error)
plot(hees.tst.pred.error, lty = 1, col = "red", type = "l")

# Verify the training error
0.5 * sum(hees.trn.pred.error.normal^2)
mean(hees.trn.pred.error.normal^2)
max(abs(hees.trn.pred.normal - as.numeric(unlist(hees.model$net.result))))
