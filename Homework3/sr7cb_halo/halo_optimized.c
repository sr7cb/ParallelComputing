// This program simulates the flow of heat through a two-dimensional plate.
// The number of grid cells used to model the plate as well as the number of
//  iterations to simulate can be specified on the command-line as follows:
// ./halo <columns> <rows> <iterations_per_cell> <iterations_per_snapshot> <iterations> <boundary_thickness> SR 3/19/17
// Sanil Rao Halo 3/19/17

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

// Define the immutable boundary conditions and the inital cell value
#define TOP_BOUNDARY_VALUE 0.0
#define BOTTOM_BOUNDARY_VALUE 100.0
#define LEFT_BOUNDARY_VALUE 0.0
#define RIGHT_BOUNDARY_VALUE 100.0
#define INITIAL_CELL_VALUE 50.0
#define hotSpotRow 4500
#define hotSptCol 6500
#define hotSpotTemp 1000;


// Function prototypes
void print_cells(float **cells, int n_x, int n_y);
void initialize_cells(float **cells, int n_x, int n_y);
void create_snapshot(float **cells, int n_x, int n_y, int id);
float **allocate_cells(int n_x, int n_y);
void die(const char *error);
float * copybuffer(float **cell, int tag, int num_rows, int numprocs, int boundary_thickness); // function transfer buffer from one computer to another SR 3/10/17


int main(int argc, char **argv) {

	// Extract the input parameters from the command line arguments
	// Number of columns in the grid (default = 1,000)
	int num_cols = (argc > 1) ? atoi(argv[1]) : 1000;
	// Number of rows in the grid (default = 1,000)
	int num_rows = (argc > 2) ? atoi(argv[2]) : 1000;
	// Number of calcuations to be done per cell SR 3/10/17
	int iters_per_cell = (argc > 3) ? atoi(argv[3]) : 1;
	// Number of calcuations per output file SR 3/10/17
	int iters_per_snapshot = (argc > 4) ? atoi(argv[4]) : 1000;
	// Number of iterations to simulate (default = 100)
	int iterations = (argc > 5) ? atoi(argv[5]) : 100;
	// Number of ghostc cells to be used during computation SR 3/10/17
	int boundary_thickness = (argc > 6) ? atoi(argv[6]) : 1;

	// Output the simulation parameters
//	printf("Grid: %dx%d, Iterations per cell: %d\n, Iterations per snapshot: %d\n, Iterations: %d\n, Boundary Thickness: %d\n", num_cols, num_rows, iters_per_cell, iters_per_cell, iterations, boundary_thickness);

	
	//Initializing MPI environment SR 3/10/17
	int numprocs, rank, namelen;
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Get_processor_name(processor_name, &namelen);
	MPI_Request request;


	// We allocate two arrays: one for the current time step and one for the next time step.
	// At the end of each iteration, we switch the arrays in order to avoid copying.
	// The arrays are allocated with an extra surrounding layer which contains
	//  the immutable boundary conditions (this simplifies the logic in the inner loop).
	float **cells[2];
	cells[0] = allocate_cells(num_cols + 2, (num_rows/numprocs)+ boundary_thickness*2);
	cells[1] = allocate_cells(num_cols + 2, (num_rows/numprocs)+ boundary_thickness*2);
	int cur_cells_index = 0, next_cells_index = 1;

	// Initialize the interior (non-boundary) cells to their initial value.
	// Note that we only need to initialize the array for the current time
	//  step, since we will write to the array for the next time step
	//  during the first iteration.
	//printf("Rank=%d: Preinitialization\n", rank);
	initialize_cells(cells[0], num_cols, num_rows/numprocs);
	
	// Set the immutable boundary conditions in both copies of the array
	int x, y, i, j;
	// setting the left and right intital coditions. Same across all regions SR 3/10/17
	for (y = boundary_thickness; y < (num_rows/numprocs) + boundary_thickness; y++) cells[0][y][0] = cells[1][y][0] = LEFT_BOUNDARY_VALUE;
	for (y = boundary_thickness; y < (num_rows/numprocs) + boundary_thickness; y++) cells[0][y][num_cols + 1] = cells[1][y][num_cols + 1] = RIGHT_BOUNDARY_VALUE;

	//Setting the top and bottom boundaries as those depend on the processor rank SR 3/10/17
	if(rank == 0){	
		for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = TOP_BOUNDARY_VALUE;
		for (x = 1; x <= num_cols; x++) cells[0][num_rows/numprocs+ boundary_thickness][x] = cells[1][num_rows/numprocs + boundary_thickness][x] = INITIAL_CELL_VALUE;
	} else if (rank == numprocs - 1) {
		for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = INITIAL_CELL_VALUE;
		for (x = 1; x <= num_cols; x++) cells[0][num_rows/numprocs+ boundary_thickness][x] = cells[1][num_rows/numprocs + boundary_thickness][x] = BOTTOM_BOUNDARY_VALUE;
	} else{
		for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = INITIAL_CELL_VALUE;
		for (x = 1; x <= num_cols; x++) cells[0][num_rows/numprocs+ boundary_thickness][x] = cells[1][num_rows/numprocs + boundary_thickness][x] = INITIAL_CELL_VALUE;
	}

	//Initialize the special hotspot cell by first finding the processor which it is located on and then putting it in the right location SR 3/19/17
	if(num_rows >= hotSpotRow) {
		if(rank == hotSpotRow/(num_rows/numprocs)) {	
			cells[cur_cells_index][hotSpotRow/numprocs][hotSptCol]=hotSpotTemp;
		 }
	}
	
	// Record the start time of the program
	MPI_Barrier(MPI_COMM_WORLD);
	time_t ranks;
	time_t start_time = time(&ranks);
	for (i = 0; i < iterations; i++) {
	//	printf("rank=%d: Iteration start = %d\n",rank, i);
		// Traverse the plate, computing the new value of each cell
		for (y = boundary_thickness; y < (num_rows/numprocs) + boundary_thickness; y++) {
			for (x = 1; x <= num_cols; x++) {
				// The new value of this cell is the average of the old values of this cell's four neighbors
				for(j = 1; j <=iters_per_cell; j++) {
					cells[next_cells_index][y][x] = (cells[cur_cells_index][y][x - 1]  +
							cells[cur_cells_index][y][x + 1]  +
							cells[cur_cells_index][y - 1][x]  +
							cells[cur_cells_index][y + 1][x]) * 0.25;
				}
			}
		}

	//	printf("rank=%d Communication start\n", rank);
	
		// Communication phase sending the appropriate information across nodes synchronous in this code  3/10/17
		if(rank != numprocs-1) {
	//		printf("rank %d sending to rank %d\n", rank, rank+1);
			MPI_Isend(copybuffer(cells[cur_cells_index], 1, num_rows, numprocs, boundary_thickness), boundary_thickness * (num_cols +2), MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD, &request);
		} 
		if (rank != 0) {
	//		printf("rank %d sending to rank %d\n", rank, rank-1);
			MPI_Isend(copybuffer(cells[cur_cells_index], 0, num_rows, numprocs, boundary_thickness), boundary_thickness * (num_cols+2), MPI_FLOAT, rank - 1, 0, MPI_COMM_WORLD, &request);
		}	
		if(rank != 0) {
	//		printf("rank %d receiving to rank %d\n", rank, rank-1);
			MPI_Recv(cells[cur_cells_index][0], boundary_thickness * (num_cols+2), MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} 
		if (rank != numprocs-1) {
	//		printf("rank %d receiving to rank %d\n", rank, rank+1);
			MPI_Recv(cells[cur_cells_index][(num_rows/numprocs)+boundary_thickness], boundary_thickness * (num_cols+2), MPI_FLOAT, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		// Swap the two arrays
		cur_cells_index = next_cells_index;
		next_cells_index = !cur_cells_index;
	
		// Print the current progress
		//	printf("Rank=%d: Iteration: %d / %d\n",rank, i + 1, iterations);
	}
	//	printf("Rank %d: all iterations finished\n", rank);
		
		// Compute and output the execution time
		MPI_Barrier(MPI_COMM_WORLD);
		time_t end_time = time(&ranks);
		printf("\nrank %d: execution time: %d seconds\n", rank, (int) difftime(end_time, start_time));
	
	
		//Stitching the final grid back together by sending all grids back to the master processor which puts those cells in the right location	
		int final_cells;
		final_cells = (iterations % 2 == 0) ? 0 : 1;
		if(rank == 0) {	
			int k;
			float **output = allocate_cells(num_cols + 2, num_rows + 2);
			int x , y;
			printf("rank %d is copying its own buffer\n", rank);
			for(x = boundary_thickness; x < (num_rows/numprocs) + boundary_thickness; x++) {
				for(y = 0; y < (num_cols +2); y++) {
					output[x+1-boundary_thickness][y] = cells[final_cells][x][y];
				}
			}
			for(k = 1; k < numprocs; k++) {
				printf("rank %d is receiving the array from rank %d\n", rank, k);
				MPI_Recv(output[(num_rows/numprocs) * k + 1], (num_rows/numprocs)*(num_cols+2), MPI_FLOAT, k, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}

			// Output a snapshot of the final state of the plate
			create_snapshot(output, num_cols, num_rows, iterations);
		}
		if(rank != 0) {
			printf("rank %d is sending its array to rank 0\n", rank);
			MPI_Isend(cells[final_cells][boundary_thickness], (num_rows/numprocs)*(num_cols+2), MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &request);
		}
	
		MPI_Finalize();
		return 0;
}
/*
 * Method to determine the starting point of the buffer to be spent to each other processor. SR 3/19/17 
 * 
 */
float * copybuffer(float **cell, int tag, int num_rows, int numprocs, int boundary_thickness) {
	if(tag == 1) {
		return cell[(num_rows/numprocs)];
	}
	else {
		return cell[boundary_thickness];
	}	
}

// Allocates and returns a pointer to a 2D array of floats
float **allocate_cells(int num_cols, int num_rows) {
	float **array = (float **) malloc(num_rows * sizeof(float *));
	if (array == NULL) die("Error allocating array!\n");

	array[0] = (float *) malloc(num_rows * num_cols * sizeof(float));
	if (array[0] == NULL) die("Error allocating array!\n");

	int i;
	for (i = 1; i < num_rows; i++) {
		array[i] = array[0] + (i * num_cols);
	}

	return array;
}


// Sets all of the specified cells to their initial value.
// Assumes the existence of a one-cell thick boundary layer.
void initialize_cells(float **cells, int num_cols, int num_rows) {
	int x, y;
	for (y = 1; y <= num_rows; y++) {
		for (x = 1; x <= num_cols; x++) {
			cells[y][x] = INITIAL_CELL_VALUE;
		}
	}
}


// Creates a snapshot of the current state of the cells in PPM format.
// The plate is scaled down so the image is at most 1,000 x 1,000 pixels.
// This function assumes the existence of a boundary layer, which is not
//  included in the snapshot (i.e., it assumes that valid array indices
//  are [1..num_rows][1..num_cols]).
void create_snapshot(float **cells, int num_cols, int num_rows, int id) {
	int scale_x, scale_y;
	scale_x = scale_y = 1;

	// Figure out if we need to scale down the snapshot (to 1,000 x 1,000)
	//  and, if so, how much to scale down
	if (num_cols > 1000) {
		if ((num_cols % 1000) == 0) scale_x = num_cols / 1000;
		else {
			die("Cannot create snapshot for x-dimensions >1,000 that are not multiples of 1,000!\n");
			return;
		}
	}
	if (num_rows > 1000) {
		if ((num_rows % 1000) == 0) scale_y = num_rows / 1000;
		else {
			printf("Cannot create snapshot for y-dimensions >1,000 that are not multiples of 1,000!\n");
			return;
		}
	}

	// Open/create the file
	char text[255];
	sprintf(text, "snapshot.%d.ppm", id);
	FILE *out = fopen(text, "w");
	// Make sure the file was created
	if (out == NULL) {
		printf("Error creating snapshot file!\n");
		return;
	}

	// Write header information to file
	// P3 = RGB values in decimal (P6 = RGB values in binary)
	fprintf(out, "P3 %d %d 100\n", num_cols / scale_x, num_rows / scale_y);

	// Precompute the value needed to scale down the cells
	float inverse_cells_per_pixel = 1.0 / ((float) scale_x * scale_y);

	// Write the values of the cells to the file
	int x, y, i, j;
	for (y = 1; y <= num_rows; y += scale_y) {
		for (x = 1; x <= num_cols; x += scale_x) {
			float sum = 0.0;
			for (j = y; j < y + scale_y; j++) {
				for (i = x; i < x + scale_x; i++) {
					sum += cells[j][i];
				}
			}
			// Write out the average value of the cells we just visited
			int average = (int) (sum * inverse_cells_per_pixel);
			fprintf(out, "%d 0 %d\t", average, 100 - average);
		}
		fwrite("\n", sizeof(char), 1, out);
	}

	// Close the file
	fclose(out);
}


// Prints the specified error message and then exits
void die(const char *error) {
	printf("%s", error);
	exit(1);
}
