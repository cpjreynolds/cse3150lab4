# CSE3150 - Lab 4

`make all` to compile

`make check` to compile and run tests.

`make fullcheck` to compile and run tests **including convergence tests which
are slow**.

`make run` to compile and run algorithm to convergence with default parameters.

## note:

due to the number of unique balanced lists growing combinatorially with `n`,
testing for convergence (i.e. testing that the distribution of lists is uniform)
is essentially impossible for large `n`.
