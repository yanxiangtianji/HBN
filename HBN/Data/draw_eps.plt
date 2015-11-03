#only label the x 4 segments with 2 future slot
#set xtics ('fu-1-0' 0,'fu-1-1' 1,'fu-1-2' 2,'fu-1-3' 3,'fu-2-0' 4,'fu-2-1' 5,'fu-2-2' 6,'fu-2-3' 7)
set xtics ('1-12' 0,'1-21' 1,'1-34' 2,'1-43' 3,'2-12' 4,'2-21' 5,'2-34' 6,'2-43' 7)
set xtics ('1-0' 0,'1-1' 1,'1-2' 2,'1-3' 3,'2-0' 4,'2-1' 5,'2-2' 6,'2-3' 7)

set key bottom
set grid
set yrange [0:1]
set ytics 0.2

set xlabel 'Node (future slot - segment id)'
set ylabel 'Accuracy'

cd 'D:\My Program\Projects\bayesian\Data\27'
#only works on 2 states

set terminal postscript eps color solid linewidth 2 "Arial" 26

set output "xx_xxx.eps"

#accuarcy
plot 'p4000/test_result_line.txt' u 0:(($2+$5)/$10) w lp lw 2 title 'Accuracy on all', '' u 0:($2/$8) w lp lw 2 title 'Acc. on Normal(TN)', '' u 0:($5/$9) w lp lw 2 title 'Acc. on Slow(TP)'


#all
plot 'p1000/test_result_line.txt' u 0:(($2+$5)/$10) w lp title 'Accuracy on all', '' u 0:($2/$8) w lp title 'Acc. on Normal(TN)', '' u 0:($5/$9) w lp title 'Acc. on Slow(TP)', '' u 0:($3/$9) w lp title 'Wrong on Slow(FP)', '' u 0:($4/$7) w lp title 'Miss on Slow(FN)'

#compare the influence of online formation
cd 'D:\My Program\Projects\bayesian\Data\'
set key at -0.5,0.01

plot '49/p200/test2_result_line.txt' u 0:($5/$9) w lp lw 2 title 'Traffic only', '51/p200/test1_result_line.txt' u 0:($5/$9) w lp lw 2 title 'T+Weather', '52/p200/test1_result_line.txt' u 0:($5/$9) w lp lw 2 title 'T+Events', '53/p200/test1_result_line.txt' u 0:($5/$9) w lp lw 2 title 'T+Sp. days', '27/p4000/test_result_line.txt' u 0:($5/$9) w lp lw 2 title 'All Info.';
