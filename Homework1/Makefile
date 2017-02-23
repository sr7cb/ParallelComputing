all: optimized optimized1 optimized2 optimized3

optimized: 
	gcc -lm optimized.c -o optimized

optimized1: 
	gcc -lm -O1 optimized.c -o optimized1

optimized2: 
	gcc -lm -O2 optimized.c -o optimized2

optimized3: 
	gcc -lm -O3 optimized.c -o optimized3
clean:
	rm -f optimized optimized1 optimized2 optimized3
