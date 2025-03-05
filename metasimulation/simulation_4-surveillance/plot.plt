set terminal pdfcairo enhanced color size 5in,2.5in font "Linux Libertine, 12"
set output 'output.pdf'
set xlabel 'Simulation Time'
set ylabel 'Throughput (task/msec)'
set grid y
set key center below
set arrow from graph 0, first 38.6525 to graph 1, first 38.6525 nohead
#T = 5000
#max = 50000
#do for [i = 0:floor(max/T)] {
#  set arrow from i*T, graph 0 to i*T, graph 1 nohead dt 2
#}
plot 'ddm.dat' using 1:2 with lines title 'ddm', 'metis-hete-comm.dat' using 1:2 with lines title 'metis-hete-comm', 'metis-homo-comm.dat' using 1:2 with lines title 'metis-homo-comm', 'metis-homo-node.dat' using 1:2 with lines title 'metis-homo-node', 'random.dat' using 1:2 with lines title 'random'
