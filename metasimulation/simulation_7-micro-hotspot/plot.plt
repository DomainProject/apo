set terminal pdfcairo enhanced color size 5in,2.5in font "Linux Libertine, 12"
set output 'output.pdf'
set xlabel 'Simulation Time'
set ylabel 'Throughput (task/msec)'
set grid y
set key center below
plot 'ddm.dat' using 1:2 with lines title 'ddm', 'metis-hete-asplike.dat' using 1:2 with lines title 'metis-hete-asplike', 'metis-hete-comm.dat' using 1:2 with lines title 'metis-hete-comm', 'metis-homo-comm.dat' using 1:2 with lines title 'metis-homo-comm', 'metis-homo-node.dat' using 1:2 with lines title 'metis-homo-node', 'random.dat' using 1:2 with lines title 'random'
