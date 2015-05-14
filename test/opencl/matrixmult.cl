int get_global_id(int a); 


void matrixMul( long ∗ C,  long ∗ B,  long ∗ A, uint wA, uint wB)
{

	int tx = get_global_id(0); 
	int ty = get_global_id(1);
	￼
	long value = 0;

	for (int k = 0 ; k < wA; k++) {
		long As = A[ty ∗ wA + k]; 
		long Bs = B[k ∗ wB + tx]; 
		value += As ∗ Bs;
	}

	C[ ty ∗ wA + tx ] = value;

}