all: sharedhalo_optimized sharedhalo_base 

sharedhalo_optimized:  
	clang -O3 -lnuma -pthread sharedhalo_opt.c -o sharedhalo_optimized

sharedhalo_base:
	gcc -O3 -pthread sharedhalo_base.c -o sharedhalo_base

clean:
	rm -f sharedhalo_optimized sharedhalo_base 
