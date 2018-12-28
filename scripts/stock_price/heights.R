library(ggplot2)
library(reshape2)

## Data source
## https://www.e-stat.go.jp/stat-search/files?page=1&layout=datalist&toukei=00400002&tstat=000001011648&cycle=0&tclass1=000001113655&tclass2=000001113656&second2=1

df.m <- read.csv('incoming/male17_height.csv', header=TRUE)
df.f <- read.csv('incoming/female17_height.csv', header=TRUE)
df <- data.frame(m=df.m, f=df.f)

names(df) <- c('male', 'female')
df$male <- df$male / 10.0
df$female <- df$female / 10.0
df$all <- (df$male + df$female) / 2.0
df$height <- 135:194

shapiro.test(df$male)
shapiro.test(df$female)

png(filename='height_mixture.png', width=800, height=480)
df <- df[,c('height', 'male', 'female', 'all')]
df.melt <- melt(df, id.vars=c('height'), variable.name='sex')
g <- ggplot(df.melt, aes(x=height, y=value, fill=sex))
g <- g + geom_bar(position='dodge', stat = 'identity')
g <- g + scale_fill_manual(values = c('royalblue', 'orchid', 'azure4'))
g <- g + xlab('Height [cm]')
g <- g + ylab('Percentage')
g <- g + labs(title='Heights of 17-year-old males and females')
g <- g + theme(legend.position=c(0.9, 0.85), legend.title=element_blank(), legend.text=element_text(size=rel(1)))
plot(g)
dev.off()
