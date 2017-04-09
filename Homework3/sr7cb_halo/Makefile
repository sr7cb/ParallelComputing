all: halo_optimized halo_snap halo_serial

halo_optimized:  
	mpicc -O3 halo_optimized.c -o halo_optimized

halo_snap:	
	mpicc -O3 halo.c -o halo_snap

halo_serial:
	gcc -O3 heated_plate.c -o halo_serial

clean:
	rm -f halo_optimized halo_snap halo_serial
