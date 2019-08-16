library(shiny)

ui <- fluidPage(verticalLayout(
    fluidRow(column(width = 11)),
    column(width = 7, wellPanel(fluidRow(
                          actionButton(inputId = 'digit0', label='0'),
                          actionButton(inputId = 'digit1', label='1'),
                          actionButton(inputId = 'digit2', label='2'),
                          actionButton(inputId = 'digit3', label='3'),
                          actionButton(inputId = 'digit4', label='4'),
                          actionButton(inputId = 'digit5', label='5'),
                          actionButton(inputId = 'digit6', label='6'),
                          actionButton(inputId = 'digit7', label='7'),
                          actionButton(inputId = 'digit8', label='8'),
                          actionButton(inputId = 'digit9', label='9'),
                          actionButton(inputId = 'period', label='.'),
                          actionButton(inputId = 'clear', label='CE')
                      ))),
    column(width = 7, wellPanel(fluidRow(
                          actionButton(inputId = 'add', label='+'),
                          actionButton(inputId = 'sub', label='-'),
                          actionButton(inputId = 'mul', label='*'),
                          actionButton(inputId = 'div', label='/'),
                          actionButton(inputId = 'left', label='('),
                          actionButton(inputId = 'right', label=')'),
                          actionButton(inputId = 'eval', label='='),
                          actionButton(inputId = 'erase', label='C')
                      ))),
    column(width = 7, wellPanel(fluidRow(
                          actionButton(inputId = 'sqrt', label='sqrt'),
                          actionButton(inputId = 'inv', label='1/x'),
                          actionButton(inputId = 'sign', label='-x')
                      ))),

    textOutput('digits'),
    textOutput('expr'),
    textOutput('all_expr'),
    textOutput('result'),
    tags$head(tags$style('#digits{color: royalblue; font-size: 32px; font-style: bold; }' )),
    tags$head(tags$style('#expr{color: royalblue; font-size: 24px; font-style: bold; }' )),
    tags$head(tags$style('#expr{color: navy; font-size: 24px; font-style: bold; }' )),
    tags$head(tags$style('#result{color: navy; font-size: 32px; font-style: bold; }' ))
))

server <- function(input, output) {
    initial_digits <- '0'
    initial_expr <- c()
    initial_result <- 0
    initial_ops <- c()
    induction_mark <- '=>'

    digits <- reactiveVal(initial_digits)
    expr <- reactiveVal(initial_expr)
    all_expr <- reactiveVal(initial_expr)
    result <- reactiveVal(initial_result)
    ops <- reactiveVal(initial_ops)
    clear_prepared <- FALSE

    reset_digits <- function() {
        digits(initial_digits)
    }

    clear_all <- function() {
        digits(initial_digits)
        expr(initial_expr)
        all_expr(initial_expr)
        result(initial_result)
        ops(initial_ops)
    }

    clear_if_prepared <- function() {
        if (clear_prepared) {
            # Keep the last result
            expr(initial_expr)
            all_expr(initial_expr)
            result(initial_result)
            ops(initial_ops)
            clear_prepared <<- FALSE
        }
    }

    add_digit <- function(arg) {
        clear_if_prepared()
        word <- digits()
        word <- if ((word == '0') && (arg != '.')) {
                    ## Overwrite 0
                    arg
                } else {
                    paste0(word, arg)
                }
        digits(word)
    }

    add_period <- function(arg) {
        clear_if_prepared()
        word <- digits()
        if (NROW(grep(pattern=arg, x=word, fixed=TRUE)) == 0) {
            add_digit(arg)
        }
    }

    ## Eval the last (expr) if it exists
    eval_tail <- function(arg_expr) {
        left <- NA
        new_expr <- arg_expr

        if ((NROW(arg_expr) > 0) && (tail(arg_expr, 1) == ')')) {
            left <- tail(which('(' == arg_expr), 1)
        }

        part_result <- NA
        if (NROW(left) == 1) {
            part_expr <- arg_expr[left:NROW(arg_expr)]
            try({
                part_result <- eval(parse(text=paste(part_expr, collapse=' ')))
            }, silent=TRUE)
        }

        if (!is.na(part_result)) {
            new_expr <- if (left > 1) {
                            arg_expr[1:(left-1)]
                        } else {
                            c()
                        }
            new_expr <- c(new_expr, part_result)
            return(list(new_digits=initial_digits, new_expr=new_expr, new_result=part_result))
        }

        NA
    }

    open_brackets <- function(arg) {
        all_new_expr <- c(all_expr(), arg)
        all_expr(all_new_expr)
        new_expr <- c(expr(), arg)
        expr(new_expr)
    }

    close_brackets <- function(arg) {
        all_new_expr <- c(all_expr(), digits(), arg)
        new_expr <- c(expr(), digits(), arg)
        expr(new_expr)

        eval_result <- eval_tail(new_expr)
        if (!is.na(eval_result)) {
            digits(eval_result$new_digits)
            expr(eval_result$new_expr)
            result(eval_result$new_result)
            all_new_expr <- c(all_new_expr, induction_mark, eval_result$new_result)
        } else {
            expr(new_expr)
        }

        all_expr(all_new_expr)
    }

    skip_induction <- function(arg_all_new_expr, arg_new_expr, new_arg, digits) {
        all_new_expr <- arg_all_new_expr
        new_expr <- arg_new_expr
        skipped <- FALSE

        n_all_new_expr <- NROW(all_new_expr)
        if ((n_all_new_expr >= 2) && (all_new_expr[n_all_new_expr - 1] == induction_mark)) {
            all_new_expr <- c(all_new_expr[1:(n_all_new_expr - 2)], new_arg)
            skipped <- TRUE
        } else {
            all_new_expr <- c(all_new_expr, digits, new_arg)
            new_expr <- c(new_expr, digits)
        }

        list(all_new_expr=all_new_expr, new_expr=new_expr, skipped=skipped)
    }

    add_unary_operator <- function(arg, func) {
        clear_if_prepared()
        expr_set <- skip_induction(all_expr(), expr(), arg, digits())
        all_new_expr <- expr_set$all_new_expr
        new_expr <- expr_set$new_expr

        if (expr_set$skipped) {
            print(new_expr)
            try({
                value <- as.character(func(as.numeric(tail(new_expr, 1))))
                expr(head(new_expr, -1))
                digits(value)
                all_new_expr <- c(all_new_expr, induction_mark)
            })
        } else {
            try({
                digits(as.character(func(as.numeric(digits()))))
                all_new_expr <- c(all_new_expr, induction_mark)
            })
        }

        all_expr(all_new_expr)
    }

    add_binary_operator <- function(arg) {
        clear_if_prepared()
        expr_set <- skip_induction(all_expr(), expr(), arg, digits())
        all_new_expr <- expr_set$all_new_expr
        new_expr <- expr_set$new_expr
        all_expr(all_new_expr)

        evaluated <- FALSE
        try({
            result(eval(parse(text=paste(new_expr, collapse=' '))))
            evaluated <- TRUE
        }, silent=TRUE)

        new_expr <- c(new_expr, arg)
        expr(new_expr)


        need_to_clear <- FALSE
        if (evaluated && (arg == '')) {
            digits(as.character(result()))
            need_to_clear <- TRUE
        } else {
            reset_digits()
        }
        need_to_clear
    }

    observeEvent(input$digit0, { add_digit('0') })
    observeEvent(input$digit1, { add_digit('1') })
    observeEvent(input$digit2, { add_digit('2') })
    observeEvent(input$digit3, { add_digit('3') })
    observeEvent(input$digit4, { add_digit('4') })
    observeEvent(input$digit5, { add_digit('5') })
    observeEvent(input$digit6, { add_digit('6') })
    observeEvent(input$digit7, { add_digit('7') })
    observeEvent(input$digit8, { add_digit('8') })
    observeEvent(input$digit9, { add_digit('9') })
    observeEvent(input$period, { add_period('.') })
    observeEvent(input$clear,  { reset_digits() })

    observeEvent(input$add, { add_binary_operator('+') })
    observeEvent(input$sub, { add_binary_operator('-') })
    observeEvent(input$mul, { add_binary_operator('*') })
    observeEvent(input$div, { add_binary_operator('/') })
    observeEvent(input$left, { open_brackets('(') })
    observeEvent(input$right, { close_brackets(')') })

    observeEvent(input$eval, {
        add_binary_operator('')
        clear_prepared <<- TRUE
    })

    observeEvent(input$erase, { clear_all() })

    observeEvent(input$sqrt, { add_unary_operator('sqrt', sqrt) })
    observeEvent(input$inv, { add_unary_operator('inv', function(x) 1/x) })
    observeEvent(input$sign, { add_unary_operator('sign', function(x) 1/x) })

    output$digits <- renderText(digits())
    output$expr <- renderText(paste(expr()))
    output$all_expr <- renderText(paste(all_expr()))
    output$result <- renderText(paste(result()))
}

shinyApp(ui = ui, server = server)
