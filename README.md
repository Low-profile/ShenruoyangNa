# Compiler for the tiny language

test cases should be passed are all passed
test all the file in the benchmark, but have to add extra newline at the end of each file to make my parser happy.
an extra newline is needed after each function in our current grammer, so i didn't make it pass without the newline.

# usage

cmake .
make

./little filename

# TODO

type checking still needs modification, some failling test won't failed until the executable generating process.  

