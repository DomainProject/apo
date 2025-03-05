set terminal pdfcairo enhanced color size 5in,2.5in font "Linux Libertine, 12"
set output 'output.pdf'
set title 'Plot'
set xlabel 'WT'
set ylabel 'TH (com per msec)'
set yrange [0:50]
set arrow from graph 0, first 38.6525 to graph 1, first 38.6525 nohead
plot 'ddm.dat' using 1:2 with lines title 'ddm', 'metis-hete-comm.dat' using 1:2 with lines title 'metis-hete-comm', 'metis-homo-comm.dat' using 1:2 with lines title 'metis-homo-comm', 'metis-homo-node.dat' using 1:2 with lines title 'metis-homo-node', 'random.dat' using 1:2 with lines title 'random'
