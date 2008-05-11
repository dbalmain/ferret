set yrange [80:100]
set xdata time
set timefmt "%Y-%m-%d"
set xrange ["2008-01-01":"2008-05-01"]
set xtics "2008-01-01",2592000,"2008-05-01"
set format x "%b %d"
plot '.rake/gcov_history.data' using 1:2 with linespoints title 'C Coverage'
set terminal jpeg medium size 320,240 \
      xffffff x000000 x404040         \
      xff0000 xffa500 x66cdaa xcdb5cd \
      xadd8e6 x0000ff xdda0dd x9500d3
set output '.rake/gcov_history.jpg'
replot
