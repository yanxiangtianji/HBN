#only label the x 4 segments with 2 future slot
set xtics ('fu-1-0' 0,'fu-1-1' 1,'fu-1-2' 2,'fu-1-3' 3,'fu-2-0' 4,'fu-2-1' 5,'fu-2-2' 6,'fu-2-3' 7)
set key bottom
set grid
set yrange [0:1]
set ytics 0.1

cd 'D:/My Program/Projects/bayesian/Data/17'
#only works on 2 states

set title 'dataset 17 p1000'
plot 'p1000/test_result_line.txt' u 0:(($2+$5)/$10) w lp title 'Accuracy on all', '' u 0:($2/$8) w lp title 'Acc. on Normal(TN)', '' u 0:($5/$9) w lp title 'Acc. on Slow(TP)', '' u 0:($3/$9) w lp title 'Wrong on Slow(FP)', '' u 0:($4/$7) w lp title 'Miss on Slow(FN)'

plot 'p1000/test_result_line.txt' u 0:(($2+$5)/$10) w lp title 'Accuracy on all', '' u 0:($2/$8) w lp title 'Acc. on Normal(TN)', '' u 0:($5/$9) w lp title 'Acc. on Slow(TP)'



unset xtics
set xtics
set xlabel 'False Positive'  
set ylabel 'False Negative'  
unset key
#relation between fn and fp
plot 'D:/My Program/Projects/bayesian/12/p5/test_result_line.txt' u ($3/$9):($4/$7) w p pt 5

cd 'D:/My Program/Projects/bayesian/12/'
plot for [filename in 'p5 p50'] filename.'/test_result_line.txt' u ($3/$9):($4/$7) s u title filename

plot 'p5/test_result_line.txt' u ($3/$9):($4/$7) w p notitle lt 1, '' u ($3/$9):($4/$7) w l s u lt 1 t 'p5'
replot 'p50/test_result_line.txt' u ($3/$9):($4/$7) w p notitle lt 2, '' u ($3/$9):($4/$7) w l s u lt 2 t 'p50'
replot 'p200/test_result_line.txt' u ($3/$9):($4/$7) w p notitle lt 3, '' u ($3/$9):($4/$7) w l s u lt 3 t 'p200'
replot 'p500/test_result_line.txt' u ($3/$9):($4/$7) w p notitle lt 4, '' u ($3/$9):($4/$7) w l s u lt 4 t 'p500'



#eps

set terminal postscript eps color solid linewidth 2 "Helvetica" 24

set output "xxx.eps"
#plot ....
set output