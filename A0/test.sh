#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make


echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*


echo "Generating test files.."
printf "Hello, World!\n" > test_files/ascii5.input
printf "Hello, World!" > test_files/ascii3.input
printf "" > test_files/empty.input
printf "xd" > test_files/ISO.input
printf "æøå" > test_files/utf8.input
printf "Hi" > test_files/ascii.input
printf "/xasd0" > test_files/2.input
printf "🤨" > test_files/1.input
printf "Hello :DSADK!#!€" > test_files/uni.input
printf "þþ" > test_files/uni1.input
printf "ÿÿÿ" > test_files/uni2.input
printf "pythön!" > test_files/data.input
printf "İnanç Esasları" > test_files/data2.input
printf ":D" > test_files/data3.input

### TODO: Generate more test files ###


echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    ${f} | sed -e 's/ASCII text.*/ASCII text/' \
                         -e 's/UTF-8 Unicode text.*/UTF-8 Unicode text/' \
                         -e 's/ISO-8859 text.*/ISO-8859 text/' \
                         -e 's/writable, regular file, no read permission/cannot determine (Permission denied)/' \
                         > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
exit $exitcode
