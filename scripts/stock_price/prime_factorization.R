library(numbers)
sapply(2019:2027,function(x){sprintf('%d=%s',x,paste(primeFactors(x),collapse='*'))})

while(TRUE) {
    now <- Sys.time()
    n <- as.integer(now)
    print(now)
    primes <- primeFactors(n)
    if (length(primes) == 1) {
        print(sprintf('Found prime %d!', n))
        break
    }
    print(primes)
    Sys.sleep(1)
}
