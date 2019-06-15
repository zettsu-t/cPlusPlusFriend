## Rのifelseは、testと同じshapeを返す
a<-c(1,2,3)
b<-c(4,5,6)
ifelse(TRUE,a,b)
if(FALSE){a}else{b}
ifelse(c(TRUE,NA,FALSE),a,b)
ifelse(rep(FALSE,6),a,b)
