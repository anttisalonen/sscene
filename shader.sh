#!/bin/bash

set -e
set -u

srcfile=$(echo $1 | sed -e 's/\.h$//')
varname=$(basename $srcfile | sed -e 's/\./_/g')
resfile=${srcfile}.h

sed -e "1 i static const char $varname[] = " -e 's/$/"/;s/^/"/' -e "$ a ;" $srcfile > $resfile
