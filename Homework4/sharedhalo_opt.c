//Parallelized implementation of the heated plate problem using pthreads and numa library
//Sanil Rao 4/14/17 CS4444

#define _GNU_SOURCE
#include <numa.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


// Define the immutable boundary conditions and the inital cell value
#define TOP_BOUNDARY_VALUE 0.0
#define BOTTOM_BOUNDARY_VALUE 100.0
#define LEFT_BOUNDARY_VALUE 0.0
#define RIGHT_BOUNDARY_VALUE 100.0
#define INITIAL_CELL_VALUE 50.0
#define hotSpotRow 4500
#define hotSptCol 6500
#define hotSpotTemp 1000

// Function prototypes
void print_cells(float **cells, int n_x, int n_y);
void initialize_cells(float **cells, int n_x, int n_y);
void create_snapshot(float **cells, int n_x, int n_y, int id);
float **numa_allocate_cells(int n_x, int n_y); //updated method see defnition at bottom
float **allocate_cells(int n_x, int n_y); //updated method see definition at bottom
void die(const char *error);

//Initializing global variables 
float** cell_pointers; // global pointer to rows within cells allowing data transfer between threads
pthread_barrier_t barrier; // pthread barrier to keep threads in step
int NUM_THREADS; // number of threads to do the computation


//basic struct to pass data needed for computation to each thread
struct data {
	long threadid;
	int iterations;
	int num_cols;
	int num_rows;
	cpu_set_t cpuset;
};


//thread runner method - task of each thread 
void *runner(void *param) {
		struct data* tv = param;
		int y,x,i;
		long threadid = tv->threadid;
		float ** cells[2];
		int num_rows = tv->num_rows;
		int num_cols = tv->num_cols;
		int cur_cells_index = 0;
		int next_cells_index = 1;
		int rows_per_thread = num_rows/NUM_THREADS;
		
		/*
		 * Initialization and execution of each thread
		 */
		cells[0] = numa_allocate_cells(num_cols + 2, rows_per_thread+2);
		cells[1] = numa_allocate_cells(num_cols + 2, rows_per_thread+2);
	
		for (y = 0; y <= rows_per_thread+1; y++) {
			for (x = 0; x <= num_cols; x++) {
				cells[cur_cells_index][y][x] = INITIAL_CELL_VALUE;
			}
		}	
	
		for (y = 0; y <= rows_per_thread+1; y++) cells[0][y][0] = cells[1][y][0] = LEFT_BOUNDARY_VALUE;
		for (y = 0; y <= rows_per_thread+1; y++) cells[0][y][num_cols + 1] = cells[1][y][num_cols + 1] = RIGHT_BOUNDARY_VALUE;
	
		if(threadid == 0) {
			for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = TOP_BOUNDARY_VALUE;
		}
		else if(threadid == NUM_THREADS-1) {
			for (x = 1; x <= num_cols; x++) cells[0][rows_per_thread+1][x] = cells[1][rows_per_thread+1][x] = BOTTOM_BOUNDARY_VALUE;
		}
		else{
			for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = INITIAL_CELL_VALUE;
			for (x = 1; x <= num_cols; x++) cells[0][rows_per_thread][x] = cells[1][rows_per_thread][x] = INITIAL_CELL_VALUE;
		}
	
		if(num_rows >= hotSpotRow) {
			if(threadid == hotSpotRow/(rows_per_thread)) {	
				cells[cur_cells_index][hotSpotRow/NUM_THREADS][hotSptCol]=hotSpotTemp;
			 }
		}
	int c;
	// Traverse the plate, computing the new value of each cell
	for (i = 0; i < tv->iterations; i++) {
		//printf("interation %d for thread %lu\n", i, threadid);
		for (y = 1; y <= rows_per_thread; y++) {
			for (x = 1; x <= num_cols; x++) {
				// The new value of this cell is the average of the old values of this cell's four neighbors
				cells[next_cells_index][y][x] = (cells[cur_cells_index][y][x - 1]  +
						cells[cur_cells_index][y][x + 1]  +
						cells[cur_cells_index][y - 1][x]  +
						cells[cur_cells_index][y + 1][x]) * 0.25;
			}
		}


		/*
		 * copying necessary rows to global variables to be passed between threads
		 */
		for(c =1; c <= rows_per_thread; c+=3) {
			//printf("%lu\n", threadid*rows_per_thread + c);
			cell_pointers[threadid*rows_per_thread + c] = cells[next_cells_index][c];
		}

		// synchronizing threads
		pthread_barrier_wait(&barrier);
		cur_cells_index = next_cells_index;
		next_cells_index = !cur_cells_index;
		

		/*
		 * Row exchange between threads cells
		 */
		for(c = 0; c < NUM_THREADS; c++) {
			if(c == 0) {
				cells[cur_cells_index][rows_per_thread+1] = cell_pointers[(c+1)*rows_per_thread];
			}
			else if(c == NUM_THREADS-1) {
				cells[cur_cells_index][0] = cell_pointers[(c-1)*rows_per_thread + rows_per_thread];
			}
			else {
				cells[cur_cells_index][rows_per_thread+1] = cell_pointers[(c+1)*rows_per_thread];
				cells[cur_cells_index][0] = cell_pointers[(c-1)*rows_per_thread + rows_per_thread];	
			}
		}	
		pthread_barrier_wait(&barrier);
//		printf("Finished iteration\n");
		// Print the current progress
//		printf("Iteration: %d / %d\n", i + 1, iterations);
	}
	
	
	/*
	* final copy back for output purposes commented out for timing after checking for correctness	 
	int final_cells = (tv->iterations % 2 == 0) ? 0 : 1;
	for(c =1; c <= rows_per_thread; c++) {
			printf("copying data to output %lu\n", threadid*rows_per_thread + c);
			cell_pointers[threadid*rows_per_thread + c] = cells[final_cells][c];
		}*/
	pthread_exit(0);
}

int main(int argc, char **argv) {

	// Extract the input parameters from the command line arguments
	// Number of columns in the grid (default = 1,000)
	int num_cols = (argc > 1) ? atoi(argv[1]) : 1000;
	// Number of rows in the grid (default = 1,000)
	int num_rows = (argc > 2) ? atoi(argv[2]) : 1000;
	// Number of iterations to simulate (default = 100)
	int iterations = (argc > 3) ? atoi(argv[3]) : 100;
	// Number of threads	
	NUM_THREADS = (argc > 4) ? atoi(argv[4]) : 1;
	// Output the simulation parameters
	printf("Grid: %dx%d, Iterations: %d,  Number Threads: %d\n ", num_cols, num_rows, iterations, NUM_THREADS);

	
	cell_pointers = allocate_cells(num_cols+2, num_rows+2);
	pthread_t tid[NUM_THREADS];
	struct data **threadarray = malloc(NUM_THREADS*sizeof(struct data));
	int i;
	//copying the data to be passed to the threads
	for(i = 0; i < NUM_THREADS; i++) {
		threadarray[i] = malloc(sizeof(struct data));
		threadarray[i]->iterations = iterations;
		threadarray[i]->num_cols = num_cols;
		threadarray[i]->num_rows = num_rows;
	}
	//intialization of global thread pointer boundary
	int x,y;
	for (x = 1; x <= num_cols; x++) cell_pointers[0][x] = TOP_BOUNDARY_VALUE;
	for (x = 1; x <= num_cols; x++) cell_pointers[num_rows + 1][x] = BOTTOM_BOUNDARY_VALUE;
	for (y = 1; y <= num_rows; y++) cell_pointers[y][0] = LEFT_BOUNDARY_VALUE;
	for (y = 1; y <= num_rows; y++) cell_pointers[y][num_cols + 1] = RIGHT_BOUNDARY_VALUE;

	//Initializing Pthreads environment
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);

	//determining cpuset in order to correctly pin the threads
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	for(i =0; i < 16; i++) {
		CPU_SET(i, &cpuset);
	}

	// Record the start time of the program
	time_t start_time = time(NULL);

	long t;
	int rc;
	//Thread creation and pinning
	for(t = 0; t < NUM_THREADS; t++) {
		threadarray[t]->threadid = t;
		threadarray[t]->cpuset = cpuset;
		rc = pthread_create(&tid[t], &attr, runner, threadarray[t]);
		if(rc) {
			printf("Something went wrong");
		}
		int p = pthread_setaffinity_np(tid[t], sizeof(cpu_set_t), &cpuset);
		if(p) {
			printf("wrong\n");
		}
	}
	//Thread joining after completion
	for(t =0; t < NUM_THREADS; t++) {
		pthread_join(tid[t],NULL);
	}

	// Output a snapshot of the final state of the plate
	//int final_cells = (iterations % 2 == 0) ? 0 : 1;
	//create_snapshot(cell_pointers, num_cols, num_rows, iterations);

	// Compute and output the execution time
	time_t end_time = time(NULL);
	printf("\nExecution time: %d seconds\n", (int) difftime(end_time, start_time));
	pthread_exit(0);
	return 0;
}

//Allocates the cells grid on a specific node that the thread is running on 
float **numa_allocate_cells(int num_cols, int num_rows) {
	float **array = (float **) numa_alloc_local((size_t)(num_rows * sizeof(float *)));
	if (array == NULL) die("Error allocating array!\n");

	array[0] = (float *) numa_alloc_local((size_t)(num_rows * num_cols * sizeof(float)));
	if (array[0] == NULL) die("Error allocating array!\n");

	int i;
	for (i = 1; i < num_rows; i++) {
		array[i] = array[0] + (i * num_cols);
	}

	return array;
}
// Allocates and returns a pointer to a 2D array of floats in the the main node
float **allocate_cells(int num_cols, int num_rows) {
	float **array = (float **) numa_alloc((size_t)(num_rows * sizeof(float *)));
	if (array == NULL) die("Error allocating array!\n");

	array[0] = (float *) numa_alloc((size_t)(num_rows * num_cols * sizeof(float)));
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
	printf("This worked???\n");
	// Close the file
	fclose(out);
}


// Prints the specified error message and then exits
void die(const char *error) {
	printf("%s", error);
	exit(1);
}
