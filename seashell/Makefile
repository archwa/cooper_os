.PHONY: clean debug run

seashell: seashell.o
	@echo "Building 'seashell'..."
	@gcc seashell.o -o seashell

seashell.o: src/seashell.c
	@echo "Building 'seashell.o'..."
	@gcc -c src/seashell.c

clean:
	@echo "Cleaning all built files..."
	@rm -f *.o seashell

debug:
	@echo "Building seashell with debugging capabilities..."
	@gcc -g src/seashell.c -o seashell

run:
	@./seashell
