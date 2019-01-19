library(ggplot2)
draw <- function(df, ns) {
    colnames(df) <- ns
    print(df)
    plot(df)
    try({g <- ggplot(df, aes_string(x=colnames(df)[1], y=colnames(df)[2]))
        g <- g + geom_point()
        plot(g)})
}

draw(data.frame(a=c(1,2), b=c(3,3)), c('x','y'))
draw(data.frame(a=c(1,2), b=c(3,4)), c('1st','y'))
draw(data.frame(a=c(1,2), b=c(4,3)), c('x','_y'))

n <- 10
df <- data.frame(y=rep(0,n), a=rep(1,n), b=rep(2,n), c=c(rep('a', 3), rep('b', n - 3)))
colnames(df) <- c('Y','1x','x2','_x3')
model <- model.matrix(as.formula('Y ~ .'), data=df)
colnames(model)
