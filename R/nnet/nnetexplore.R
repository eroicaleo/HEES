hees.trn <- read.delim("data/trn_10.txt.0100", header = FALSE, sep = "")
names(hees.trn) <- c("solar", "endE", "len", "initE")

hees.trn <- hees.trn[c("solar", "endE", "initE")]
solar.factor <- as.factor(hees.trn$solar)
hees.endE <- split(hees.trn$endE, solar.factor)

trn.min <- lapply(hees.trn.unique, min)
trn.max <- lapply(hees.trn.unique, max)

plot(
  hees.endE[[1]],
  type = "n",
  ylab = "Energy",
  ylim = c(trn.min$endE, trn.max$endE)
)
for (i in 1:length(hees.endE)) {
  lines(hees.endE[[i]], col = i)
}
legend("topleft", legend = names(hees.endE), col = 1:length(hees.endE), lty = 1)

ix <- hees.trn$solar > 1.0
hees.trn.part <- hees.trn[ix,]
tail(hees.trn.part)
names(hees.endE)
