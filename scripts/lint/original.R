library(assertthat)
n <- 1
actual <- 2 ** (4+n)
actual <- 2 ^ (4+n)
expected <- 32
assertthat::assert_that(actual == expected)
print("--------------------------------------------------------------------------------")

f1 <- function(x) {
    g11 <- function(x) {
        if (is.na(x)) {
            print("g11-NA")
            0
        } else if (x == 0) {
            1
        } else if (x == 1) {
            2
        } else if (x == 2) {
            3
        } else {
            x * 10
        }
    }

    g12 <- function(x) {
        if (is.na(x)) {
            print("g12-NA")
            0
        } else if (x == 0) {
            2
        } else if (x == 1) {
            4
        } else if (x == 2) {
            6
        } else {
            x * 20
        }
    }

    g13 <- function(x) {
        if (is.na(x)) {
            print("g13-NA")
            0
        } else if (x == 0) {
            3
        } else if (x == 1) {
            6
        } else if (x == 2) {
            9
        } else {
            x * 30
        }
    }

    g14 <- function(x) {
        if (is.na(x)) {
            print("g14-NA")
            0
        } else if (x == 0) {
            4
        } else if (x == 1) {
            8
        } else if (x == 2) {
            12
        } else {
            x * 40
        }
    }

    g11(x) + g12(x) + g13(x) + g14(x) + g14(x) + g14(x)
}

g21 <- function(x) {
    if (is.na(x)) {
        print("g21-NA")
        0
    } else if (x == 0) {
        1
    } else if (x == 1) {
        2
    } else if (x == 2) {
        3
    } else {
        x * 10
    }
}

g22 <- function(x) {
    if (is.na(x)) {
        print("g22-NA")
        0
    } else if (x == 0) {
        2
    } else if (x == 1) {
        4
    } else if (x == 2) {
        6
    } else {
        x * 20
    }
}

g23 <- function(x) {
    if (is.na(x)) {
        print("g23-NA")
        0
    } else if (x == 0) {
        3
    } else if (x == 1) {
        6
    } else if (x == 2) {
        9
    } else {
        x * 30
    }
}

g24 <- function(x) {
    if (is.na(x)) {
        print("g24-NA")
        0
    } else if (x == 0) {
        4
    } else if (x == 1) {
        8
    } else if (x == 2) {
        12
    } else {
        x * 40
    }
}

f2 <- function(x) {
    g21(x) + g22(x) + g23(x) + g24(x) + g24(x) + g24(x)
}

f1(0)
f1(1)
f1(2)
f1(3)
f1(NA)

f2(0)
f2(1)
f2(2)
f2(3)
f2(NA)
