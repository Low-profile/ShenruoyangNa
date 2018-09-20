# ShenruoyangNa

Syntax parsing reimplemented 
passing all test cases in the passing tests folder except the expc_dc.lil since there is a newline int the first
line which is inconsitent with our grammer, so i didn't make it pass.

# usage

make

./parser filename

move the passing-tests folder to the current working directory,

**"make run"** will test all the *.lil file in the passing-tests folder

# TODO

Type Checking needs modification

# Some question

What's the meaning of ` here in the regex of the string "('\\'["nt\\]|[^\\\n"])*" ?
It's a useless escape symbol or it should be matched in the string literals?

