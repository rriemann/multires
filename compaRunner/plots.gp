# kate: hl gnuplot;
plot '< grep "regular" < /tmp/output.txt' using 1:4
