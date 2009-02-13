#!/bin/sh

# $Id: autotool.sh 2408 2008-06-13 12:20:55Z roman_rybalko $

set -e
set -x

aclocal -I m4
libtoolize -c -f
automake -a -c -f
autoconf

rm -Rf config.cache autom4te.cache
