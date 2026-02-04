#ifndef MATRIX_IO_H
#define MATRIX_IO_H

typedef struct {
    int M;              
    int N;               
    int nz;             
    int is_symmetric;
    int *I, *J;         
    double *val;        
    int *prefixSum;     
    int *sorted_J;      
    double *sorted_val; 
} Matrix;

Matrix* read_matrix(const char *filename);
void free_matrix(Matrix *mat);

#endif
