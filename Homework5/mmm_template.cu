// Performance based GPU implementation of Matrix Multiply using NVIDIA CUDA Programming Language
//Sanil Rao 5/8/17 CS4444

#include<stdio.h>
#include<sys/time.h>
#include<stdlib.h>
#include<iostream>

//number of threads per block sent to the kernel
#define threads_per_block 2
using namespace std;

//----------------------------------- Structures and Globals---------------------------------------------
typedef struct {
	int dimension1;
	int dimension2;	
} ArrayMetadata2D;

// metadata variables describing dimensionalities of all data structures involved in the computation
ArrayMetadata2D A_MD, B_MD, C_MD;
// pointers for input and output arrays in the host memory  
float *A, *B, *C, *C_CPU;
// pointers for input and output arrays in the device memory (NVIDIA DRAM)
float *A_GPU, *B_GPU, *C_GPU;

//----------------------------------- host function definitions -----------------------------------------

void allocateAndInitializeAB();
void computeCpuMMM();
void copyMatricesToGPU();
void copyResultFromGPU();
void compareHostAndGpuOutput();
void die(const char *error); 
void check_error(cudaError e);

//----------------------------------- CUDA function definitions -----------------------------------------
//barebones GPU implementation kernel
void GPU_matrix_multiply();
//optimized GPU implementation kernel
void GPU_matrix_multiply_opt();

//-------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
	
	A_MD.dimension1 = (argc > 1) ? atoi(argv[1]) : 100;
	A_MD.dimension2 = (argc > 2) ? atoi(argv[2]) : A_MD.dimension1;
	B_MD.dimension1 = (argc > 3) ? atoi(argv[3]) : A_MD.dimension2;
	B_MD.dimension2 = (argc > 4) ? atoi(argv[4]) : B_MD.dimension1;
	C_MD.dimension1 = A_MD.dimension1;
	C_MD.dimension2 = B_MD.dimension2;

	printf("Matrix A is %d-by-%d\n", A_MD.dimension1, A_MD.dimension2);
	printf("Matrix B is %d-by-%d\n", B_MD.dimension1, B_MD.dimension2);
	printf("Matrix C is %d-by-%d\n", C_MD.dimension1, C_MD.dimension2);

	allocateAndInitializeAB();

	//Timing the CPU implementation that was unmodified

	// matrix matrix multiplication in the CPU
	//	clock_t start = clock();	
	//	computeCpuMMM();
	//	clock_t end = clock();
   
	//	double elapsed = (end - start) / (double) CLOCKS_PER_SEC;
	//	printf("Computation time in the CPU: %f seconds\n", elapsed);
	
	//Timing the barebones GPU implementation
	
	//clock_t startGPU = clock();
	//GPU_matrix_multiply();	
	//clock_t endGPU = clock();

	//Timing the optimized GPU implementation

	clock_t startGPU = clock();
	GPU_matrix_multiply_opt();	
	clock_t endGPU = clock();
	double elapsedGPU = (endGPU-startGPU)/ (double) CLOCKS_PER_SEC;
	printf("Computation time in the GPU: %f seconds\n", elapsedGPU);	
	//double elapsedGPU2 = (endGPU2-startGPU1)/ (double) CLOCKS_PER_SEC;
	//printf("Computation time in the GPU: %f seconds\n", elapsedGPU2);

	//compareHostAndGpuOutput();	
	return 0;
}

//Optimzed GPU kernel using tiling, shared memory, and transposed matricies
//2 dimensional grid and block size to ease with computation overhead I think better in 2D for a 2D problem
__global__ void mm_kernel_opt(float *A_GPU,float *B_GPU, float * C_GPU, ArrayMetadata2D A_MD) {
	//determing which block that is being computed
	 int block_id_row = blockIdx.y;
	 int block_id_col = blockIdx.x;

	 //resultant value to placed into the output matrix
	 float val;
	 val = 0;

	//determing specific row and column value for each thread
	 int row = threadIdx.y;
	 int col = threadIdx.x;
		
	//declaring shared memory to be used during computation
	__shared__ float  A[threads_per_block][threads_per_block];
	__shared__ float  B[threads_per_block][threads_per_block];

	//looping over the size of each tile and computnig the value
	 for(int j = 0; j < (threads_per_block + A_MD.dimension1 -1)/threads_per_block; j++) {

			//thread out of bounds check
			if((j * threads_per_block + col) < A_MD.dimension1 &&(block_id_row*threads_per_block+row) < A_MD.dimension1) 
				//intital values to be placed into shared memory
				A[row][col] = A_GPU[(block_id_row*threads_per_block + row) * A_MD.dimension1 + j*threads_per_block + col];
			else
				//excess values used to not impact computation
				A[row][col] = 0.0;
			if((j*threads_per_block + row) < A_MD.dimension1 && (block_id_col*threads_per_block+col) < A_MD.dimension1) 
				//intital values to be placed into shared memory this case transposed 
				B[row][col] = B_GPU[(block_id_col*threads_per_block+col) * A_MD.dimension1 +(j*threads_per_block + row)];
			else
				//excess values used to not impact computation
				B[row][col] = 0.0;
		
		//barrier to make sure copying was completed before computing
		__syncthreads();

		//computation phase
		for(int c = 0; c < threads_per_block; c++) {
			val += A[row][c] * B[c][col];
		}
		//barrier to make sure computation phase was done correctly
		__syncthreads();
		
		//final bounds check 
		if((block_id_row*threads_per_block+row) < A_MD.dimension1 && (block_id_col*threads_per_block + col) < A_MD.dimension1)
			//placing the value into the output matrix
			C_GPU[((block_id_row * blockDim.y + row) * A_MD.dimension1) + (block_id_col * blockDim.x) + col] = val;

	}
}

//barebones GPU kernel 1 dimension grid and block size
__global__ void mm_kernel(float *A_GPU, float*B_GPU, float *C_GPU, ArrayMetadata2D A_MD) {

	//definitions to aid in GPU programming SR 4/30/17
	//block id gives value of each indivdual block which together make the whole grid
	//block dim gives the value of the block size
	// thread id which gives each thread within the block. 

	 //determing each threads block and thread number
	 int block_id = blockIdx.x;
     int global_thread_id = blockDim.x * block_id + threadIdx.x;

	 int k, i;
	 float val;
	
	//computing over the matricies using the global thread number as the column as it was inputed in 1 dimension
	 for(i = 0; i < A_MD.dimension1; i++) {
		val = 0;
		for(k = 0; k < A_MD.dimension2; k++) {
			 val += A_GPU[i*A_MD.dimension2 + k] * B_GPU[k * A_MD.dimension2 + global_thread_id];
			 C_GPU[i*A_MD.dimension2 + global_thread_id] = val;
		 }
	 }
}

//host code to launch the optimized kernel
void GPU_matrix_multiply_opt() {
	copyMatricesToGPU();
	dim3 block_size(threads_per_block, threads_per_block);	
	dim3 grid_size(ceil(((float)A_MD.dimension1)/threads_per_block),ceil(((float)A_MD.dimension1)/threads_per_block));
	mm_kernel_opt<<<grid_size, block_size>>> (A_GPU, B_GPU, C_GPU, A_MD);
	copyResultFromGPU();
}

//  host code to launch the base kernel 
void GPU_matrix_multiply() {
	copyMatricesToGPU();
	dim3 grid_size((A_MD.dimension2 + threads_per_block*threads_per_block -1)/(threads_per_block*threads_per_block));
	dim3 block_size(threads_per_block * threads_per_block);
	mm_kernel <<<grid_size, block_size>>> (A_GPU,B_GPU,C_GPU,A_MD);
	copyResultFromGPU();
}

// allocate and initialize A and B using a random number generator
void allocateAndInitializeAB() {
	
	size_t sizeofA = A_MD.dimension1 * A_MD.dimension2 * sizeof(float);
	A = (float*) malloc(sizeofA);
	
	srand(time(NULL));
  	for (int i = 0; i < A_MD.dimension1; i++) {
		for (int j = 0; j < A_MD.dimension2; j++) {
			int index = i * A_MD.dimension2 + j;
			A[index] = (rand() % 1000) * 0.001; 
		}
	}
			
	size_t sizeofB = B_MD.dimension1 * B_MD.dimension2 * sizeof(float);
	B = (float*) malloc(sizeofB);
  	for (int i = 0; i < B_MD.dimension1; i++) {
		for (int j = 0; j < B_MD.dimension2; j++) {
			int index = i * B_MD.dimension2 + j;
			B[index] = (rand() % 1000) * 0.001; 
		}
	}
}

// allocate memory in the GPU for all matrices, and copy A and B content from the host CPU memory to the GPU memory
//if barebones kernel is to be launched dont transpose the matrix comment out the transpose
void copyMatricesToGPU() {
	
	size_t sizeofA = A_MD.dimension1 * A_MD.dimension2 * sizeof(float);
	check_error(cudaMalloc((void **) &A_GPU, sizeofA));
	check_error(cudaMemcpy(A_GPU, A, sizeofA, cudaMemcpyHostToDevice));
	
	size_t sizeofB = B_MD.dimension1 * B_MD.dimension2 * sizeof(float);
	check_error(cudaMalloc((void **) &B_GPU, sizeofB));
	int i,j;
	/*for(i = 0; i < B_MD.dimension1; i++) {
		for(j = 0; j < B_MD.dimension2; j++) {
			int index = i * B_MD.dimension2 + j;
			printf("%f  ", B[index]);
		}
		printf("\n");
	}*/
	for(i = 0; i < B_MD.dimension1; i++) {
			int fi = i * B_MD.dimension2;
		for(j = 0; j < B_MD.dimension2; j++) {
			if(j > i) {
			int index = fi +j;
			int newindex = j * B_MD.dimension2 + i;
			float tmp = B[index];
			B[index] = B[newindex];
			B[newindex] = tmp;
			}
		}
	} 
	/*for(i = 0; i < B_MD.dimension1; i++) {
		for(j = 0; j < B_MD.dimension2; j++) {
			int index = i * B_MD.dimension2 + j;
			printf("%f  ", B[index]);
		}
		printf("\n");
	}*/
	check_error(cudaMemcpy(B_GPU, B, sizeofB, cudaMemcpyHostToDevice));
	
	size_t sizeofC = C_MD.dimension1 * C_MD.dimension2 * sizeof(float);
	check_error(cudaMalloc((void **) &C_GPU, sizeofC));
}

// copy results from C_GPU which is in GPU card memory to C_CPU which is in the host CPU for result comparison
void copyResultFromGPU() {
	size_t sizeofC = C_MD.dimension1 * C_MD.dimension2 * sizeof(float);
	C_CPU = (float*) malloc(sizeofC);
	check_error(cudaMemcpy(C_CPU, C_GPU, sizeofC, cudaMemcpyDeviceToHost));
}

// do a straightforward matrix-matrix multiplication in the CPU
// notice that this implementation can be massively improved in the CPU by doing proper cache blocking but we are
// not providing you the efficient CPU implementation as that reveals too much about the ideal GPU implementation
void computeCpuMMM() {
	
	// allocate the result matrix for the CPU computation
	size_t sizeofC = C_MD.dimension1 * C_MD.dimension2 * sizeof(float);
	C = (float*) malloc(sizeofC);
	
	// compute C[i][j] as the sum of A[i][k] * B[k][j] for all columns k of A
	for (int i = 0; i < A_MD.dimension1; i++) {
		int a_i = i * A_MD.dimension2;
		int c_i = i * C_MD.dimension2;
		for (int j = 0; j < B_MD.dimension2; j++) {
			int c_index = c_i + j;
			C[c_index] = 0;
			for (int k = 0; k < B_MD.dimension1; k++) {
				int a_index = a_i + k;
				int b_index = k * B_MD.dimension2 + j;
				C[c_index] += A[a_index] * B[b_index];
			}
		}
	}
}

// function to determine if the GPU computation is done correctly by comparing the output from the GPU with that
// from the CPU
void compareHostAndGpuOutput() {
	int totalElements = C_MD.dimension1 * C_MD.dimension2;
	int missmatchCount = 0;
	for (int i = 0; i < totalElements; i++) {
		if (fabs(C[i] - C_CPU[i]) > 0.01) {
			missmatchCount++;
			printf("mismatch at index %i: %f\t%f\n", i, C[i], C_CPU[i]);
		}
	}
	if (missmatchCount > 0) {
		printf("Computation is incorrect: outputs do not match in %d indexes\n", missmatchCount);
	} else {
		printf("Computation is correct: CPU and GPU outputs match\n");
	}
}

// Prints the specified error message and then exits
void die(const char *error) {
        printf("%s", error);
        exit(1);
}

// If the specified error code refers to a real error, report it and quit the program
void check_error(cudaError e) {
        if (e != cudaSuccess) {
                printf("\nCUDA error: %s\n", cudaGetErrorString(e));
                exit(1);
        }
}

