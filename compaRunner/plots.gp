#!/usr/bin/gnuplot -p
# kate: hl gnuplot;

f(x) = m*x + n
fit f(x) '< grep "regular" < /tmp/output.dat' using log(1):log(4) via m,n
plot '< grep "regular" < /tmp/output.dat' using log(1):log(4), f(x) with lines

#set term qt 1
#fit f(x) '< grep "multires" < /tmp/output.dat' using log(1):log(4) via m,n
#plot '< grep "multires" < /tmp/output.dat' using log(1):log(4), f(x) with lines