//Parallelized implementation of the heated plate problem using pthreads
//Sanil Rao 4/14/17 CS4444

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
float **allocate_cells(int n_x, int n_y);
void die(const char *error);

//Initializing global variables so that one only has to pass thread id to each thread
int NUM_THREADS, iterations, num_cols, num_rows; //cur_cells_index, next_cells_index;
float** cells[2];
pthread_barrier_t barrier;

//thread runner method - task of each thread 
void *runner(void *param) {
	int cur_cells_index = 0;
	int next_cells_index = 1;
	long threadid = (long)(param);
//	printf("The thread id is %lu\n", threadid);
	int x,y;
	int rows_per_thread = num_rows/NUM_THREADS;
	int start = threadid * rows_per_thread +1;	
	//printf("start %d\n", start);
	int end = start + rows_per_thread -1;
	//printf("end %d\n", end);
//	printf("Initialized variables %d and % d\n", start ,end);
	// Traverse the plate, computing the new value of each cell
	int i;
	for (i = 0; i < iterations; i++) {
		for (y = start; y <= end; y++) {
			for (x = 1; x <= num_cols; x++) {
				// The new value of this cell is the average of the old values of this cell's four neighbors
				cells[next_cells_index][y][x] = (cells[cur_cells_index][y][x - 1]  +
						cells[cur_cells_index][y][x + 1]  +
						cells[cur_cells_index][y - 1][x]  +
						cells[cur_cells_index][y + 1][x]) * 0.25;
//				printf("value %f and thread id %lu\n ", cells[cur_cells_index][y][x], threadid);
			}
		}
			
		pthread_barrier_wait(&barrier);
		cur_cells_index = next_cells_index;
		next_cells_index = !cur_cells_index;

	
//		printf("Finished iteration\n");
		// Print the current progress
//		printf("Iteration: %d / %d\n", i + 1, iterations);
	}
}

int main(int argc, char **argv) {
	// Record the start time of the program

	// Extract the input parameters from the command line arguments
	// Number of columns in the grid (default = 1,000)
	num_cols = (argc > 1) ? atoi(argv[1]) : 1000;
	// Number of rows in the grid (default = 1,000)
	num_rows = (argc > 2) ? atoi(argv[2]) : 1000;
	// Number of iterations to simulate (default = 100)
	iterations = (argc > 3) ? atoi(argv[3]) : 100;
	// Number of threads
	NUM_THREADS = (argc > 4) ? atoi(argv[4]) : 1;
	// Output the simulation parameters
	printf("Grid: %dx%d, Iterations: %d\n", num_cols, num_rows, iterations);

	// We allocate two arrays: one for the current time step and one for the next time step.
	// At the end of each iteration, we switch the arrays in order to avoid copying.
	// The arrays are allocated with an extra surrounding layer which contains
	//  the immutable boundary conditions (this simplifies the logic in the inner loop).
	cells[0] = allocate_cells(num_cols + 2, num_rows + 2);
	cells[1] = allocate_cells(num_cols + 2, num_rows + 2);
	int  cur_cells_index = 0; 
	int  next_cells_index = 1;

	// Initialize the interior (non-boundary) cells to their initial value.
	// Note that we only need to initialize the array for the current time
	//  step, since we will write to the array for the next time step
	//  during the first iteration.
	initialize_cells(cells[0], num_cols, num_rows);

	// Set the immutable boundary conditions in both copies of the array
	int x, y, i;
	for (x = 1; x <= num_cols; x++) cells[0][0][x] = cells[1][0][x] = TOP_BOUNDARY_VALUE;
	for (x = 1; x <= num_cols; x++) cells[0][num_rows + 1][x] = cells[1][num_rows + 1][x] = BOTTOM_BOUNDARY_VALUE;
	for (y = 1; y <= num_rows; y++) cells[0][y][0] = cells[1][y][0] = LEFT_BOUNDARY_VALUE;
	for (y = 1; y <= num_rows; y++) cells[0][y][num_cols + 1] = cells[1][y][num_cols + 1] = RIGHT_BOUNDARY_VALUE;

	if(num_rows >= hotSpotRow && num_cols >= hotSptCol) 
		cells[cur_cells_index][hotSpotRow][hotSptCol]=hotSpotTemp;
	//Initializing Pthreads environment
	pthread_t tid[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);
	time_t start_time = time(NULL);
	long t;
	int rc;
	for(t = 0; t < NUM_THREADS; t++) {
		rc = pthread_create(&tid[t], &attr, runner, (void*) t);
		if(rc) {
			printf("Something went wrong");
		}
	}

	for(t =0; t < NUM_THREADS; t++) {
		pthread_join(tid[t],NULL);
	}
	// Output a snapshot of the final state of the plate
	//int final_cells = (iterations % 2 == 0) ? 0 : 1;
	//create_snapshot(cells[final_cells], num_cols, num_rows, iterations);

	// Compute and output the execution time
	time_t end_time = time(NULL);
	printf("\nExecution time: %d seconds\n", (int) difftime(end_time, start_time));
	pthread_exit(0);
	return 0;
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
