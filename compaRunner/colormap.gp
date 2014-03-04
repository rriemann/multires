#!/usr/bin/gnuplot
# kate: hl gnuplot;

# http://www.gnuplotting.org/code/default.plt
load 'default.plt'

set size square
set dgrid3d 128,,16
# set style data lines
set pm3d map
# splot [0:1] [0:1] [0:1] '/tmp/regular.dat' using 1:2:3
splot [0:1] [0:1] [0:1] '/tmp/output.txt' using 1:2:3


pause -1