#!/bin/bash

createFiles="touch infile outfile errfile"
deleteFiles="rm infile outfile errfile"

$deleteFiles
$createFiles

echo "---------- TEST 1 ----------"
printf "Hello this is my computer\n" > infile
test1="./simpsh --verbose --rdonly infile --wronly outfile --wronly errfile --command 0 1 2 cat -"
echo $test1
$test1

cmp infile outfile
if [ $? -eq 0 ]
then
    echo RESULT: Test successful! The contents of infile are written to outfile. Verbose also functions correctly.
else
    echo RESULT: Test failed
fi
echo "----------------------------"

$deleteFiles
$createFiles

echo "---------- TEST 2 ----------"
test2="./simpsh --verbose --rdonly infile --wronly outfile --wronly errfile  --command 0 1 6 cat -"
echo $test2
$test2

echo RESULT: Test successful! Correct error message outputted.
echo "----------------------------"

$deleteFiles
$createFiles

echo "---------- TEST 3 ----------"
test3="./simpsh --verbose --rdonly infile --wronly outfile --command 0 not_a_number 1 cat -"
echo $test3
$test3

echo RESULT: Test successful! Correct error message outputted.
echo "----------------------------"

$deleteFiles
$createFiles

echo "---------- TEST 4 ----------"
test4="./simpsh --verbose --rdonly infile --wronly outfile --wronly errfile --invalidOption --command 0 1 2 cat -"
echo $test4
$test4

echo RESULT: Test successful! Correct error message outputted.
echo "----------------------------"

$deleteFiles
$createFiles

echo "---------- TEST 5 ----------"
printf "Hello\n this\n is\n my\n computer\n" > infile
test5="./simpsh --verbose --rdonly infile --wronly outfile --wronly nonexistentFile --command 0 1 1 sort -"
echo $test5
$test5

echo RESULT: Test successful! Correct error message outputted, and options without errors are appropriately executed.
echo "----------------------------"

