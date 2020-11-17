library(tidyverse)
library(pals)

## RのintToBitsは、整数を0または1のビット列を格納したrawベクトルにする
## つまりi番目が2^(i-1)桁になる(Rなのでi>=1)
## rawから整数に変換すればよい
## revしているのは、文字列表記するときにMSB firstにするため
to_bits <- function(x) {
    v <- rev(as.integer(intToBits(x)))
    print(v)
    print(paste0(v, collapse=''))

    ## 先頭の0sを除くと見やすいので、最初の1より前を取り除く
    ## ただし全部0だと1が見つからないので、purrr::detect_indexは0を返す。
    ## これを特別扱いしないように上手くやる
    width <- NROW(v)
    paste0(tail(width, ( + 1 - purrr::detect_index(v, ~ .x==1)) %% NROW(width)),
           collapse='')
}

to_bits(0)
to_bits(1)
to_bits(13)
to_bits(30)

## ついでに
## Rで色を塗り分けたいけど、Set3やPairedパレットだと12色しかないし、
## かといってグラデーションでは困るときは、palsパッケージを使うとよい
n <- 32
df <- tibble(x=1:n, y=1, z=factor(1:n))
g <- ggplot(df)
g <- g + geom_tile(aes(x=x, y=y, fill=z))
g <- g + scale_fill_manual(values=pals::glasbey())
plot(g)
