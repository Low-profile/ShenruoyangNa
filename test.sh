#!/bin/sh

echo "" > result.out;
for file in ./passing-tests/*lil; do ./parser "$file" >> result.out; done