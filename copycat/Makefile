.PHONY: all clean

all: copycat submission.pdf

copycat: copycat.o
	@echo "Building 'copycat'..."
	@gcc -o copycat copycat.o

copycat.o: src/copycat.c
	@echo "Building 'copycat.o'..."
	@gcc -c src/copycat.c

img/results.pdf: test/results.dat
	@echo "Building 'img/results.pdf'..."
	@gnuplot -e 'set terminal pdf; set output "img/results.pdf"; set title "Performance of copycat"; set xtics axis nomirror out; set ytics axis nomirror out; set xlabel "Buffer Size [in log(Bytes)]"; set ylabel "Throughput [in MB/s]"; plot "test/results.dat" with points pointtype 7 lc rgb "#ee000000"'

submission.pdf: doc/submission.tex src/copycat.c img/error-handling.png img/results.pdf
	@echo "Building 'submission.pdf'..."
	@pdflatex doc/submission.tex > /dev/null
	@pdflatex doc/submission.tex > /dev/null
	@rm -f submission.aux submission.log submission.lof submission.toc texput.log

test/results.dat: copycat
	@echo "Building 'test/results.dat'..."
	@test/test.sh
	
clean:
	@echo "Cleaning up built files..."
	@rm -f copycat copycat.o submission.aux submission.log submission.lof submission.toc texput.log img/results.pdf test/results.dat
