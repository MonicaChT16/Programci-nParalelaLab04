#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

void imprimir_matriz(int *matriz, int n) {
    for(int i=0; i<n; i++) {
        for(int j=0; j<n; j++) {
            printf("%d ", matriz[i*n+j]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int n=4; // Tamaño matriz n x n
    int *matriz=NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size<2) {
        if(rank==0) printf("Ejecutar con al menos 2 procesos.\n");
        MPI_Finalize();
        return 1;
    }

    if(rank==0) {
        // Crear matriz ejemplo 4x4 como arreglo unidimensional
        matriz=(int*) malloc(n*n*sizeof(int));
        for(int i=0; i<n*n; i++) {
            matriz[i]=i;
        }

        printf("Matriz original (proceso 0):\n");
        imprimir_matriz(matriz, n);

        // Construcción del tipo derivado MPI_Type_indexed para parte triangular superior
        int count=n; // número de bloques (filas)
        int *blocklengths=(int*) malloc(count*sizeof(int));
        int *displacements=(int*) malloc(count*sizeof(int));

        for(int i=0; i<n; i++) {
            // En la fila i, la parte triangular superior tiene n - i elementos
            blocklengths[i]=n - i;
            // desplazamiento en el arreglo 1D=fila*i+columna i (diagonal)
            displacements[i]=i*n+i;
        }

        MPI_Datatype tipo_triangular;
        MPI_Type_indexed(count, blocklengths, displacements, MPI_INT, &tipo_triangular);
        MPI_Type_commit(&tipo_triangular);

        // Enviar solo la parte triangular superior al proceso 1
        MPI_Send(matriz, 1, tipo_triangular, 1, 0, MPI_COMM_WORLD);

        MPI_Type_free(&tipo_triangular);
        free(blocklengths);
        free(displacements);
        free(matriz);
    }

    if(rank==1) {
        // Proceso 1 debe recibir la parte triangular superior y mostrarla
        // Total de elementos en la parte triangular superior=n+(n-1)+(n-2)+...+1=n*(n+1)/2
        int elementos=n*(n+1)/2;
        int *buffer=(int*) malloc(elementos*sizeof(int));

        MPI_Recv(buffer, elementos, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("\nParte triangular superior recibida (proceso 1):\n");
        for(int i=0; i<elementos; i++) {
            printf("%d ", buffer[i]);
        }
        printf("\n");

        free(buffer);
    }

    MPI_Finalize();
    return 0;
}
