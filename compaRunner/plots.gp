#!/usr/bin/gnuplot
# kate: hl gnuplot;

f(x) = m*x + n
# fit f(x) '< grep "regular" < /tmp/output.dat' using (log($2)):(log($4)) via m,n
# plot '< grep "regular" < data/output_regular_triangle.dat' using (log($2)):(log($4)), f(x) with lines
# print m
# print n

# set term qt 1

# fit f(x) '< grep "regular" < /tmp/output.dat' using (log($2)):(log($4)) via m,n
# plot '< grep "regular" < data/output_regular_gauss.dat' using (log($2)):(log($4)), f(x) with lines
# print m
# print n

#fit f(x) '< grep "multires" < /tmp/output.dat' using log(1):log(4) via m,n
#plot '< grep "multires" < /tmp/output.dat' using log(1):log(4), f(x) with lines


# fit f(x) '< grep "multires" < /tmp/output.dat' using (log($3)):(log($4)) via m,n
# plot '< grep "multires" < /tmp/output.dat' using (log($3)):(log($4)), f(x) with lines

# fit f(x) '< grep "multires" < /tmp/output.dat' using (log($3)):(log($4)) via m,n
# plot '< grep "regular" < /tmp/output.dat' using 1:(log($4)) t "regular" w lp ls 1, \
#      '< grep "1.000000e-05 .* multires" < /tmp/output.dat' using 1:(log($4)) t "multi, eps=1e4" w l, \
#      '< grep "2.000000e-05 .* multires" < /tmp/output.dat' using 1:(log($4)) t "multi, eps=2e4" w l, \
#      '< grep "4.000000e-05 .* multires" < /tmp/output.dat' using 1:(log($4)) t "multi, eps=4e4" w l, \
#      '< grep "8.000000e-05 .* multires" < /tmp/output.dat' using 1:(log($4)) t "multi, eps=8e4" w l
     
set term qt 1

set xlabel "N" # offset -3,-2
set ylabel "epsilon" # offset 3,-2
set zlabel "norm" # offset -5

set dgrid3d 20,7 splines
set style data lines

# splot '/tmp/output.dat' using (log($2)):(log($3)):(log($4))
# splot '< grep "multires" < /tmp/output.dat' using 1:(log($3)):(log($4)), \
#      '< grep "regular" < /tmp/output.dat' using 1:(log($3)):(log($4))
splot '< grep "multires" < /tmp/output.dat' using 1:(log($3)):(log($4))

pause -1
