#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kmeans.h"
#include "cluster.h"
#include <sys/time.h>
#include <math.h>
#include <mpi.h>


#define max_iterations 50
#define threshold 0.001

void printErrorMessage()
{
	char *error = "Error using kmeans: Three arguments required\n"
	"First: number of elements\n"
	"Second: number of attributes (dimensions)\n"
	"Third: numder of clusters\n";

	printf("%s", error);
}

void initializeRandomDatapoints(data_struct *data_in)
{
	int i, j = 0;
	int lead_dim = data_in->leading_dim;
	int second_dim = data_in->secondary_dim;
	double *tmp_dataset = data_in->dataset;
	unsigned int *tmp_Index = data_in->members;
	srand(0); 

	for (i = 0; i < second_dim; i++)
	{
		tmp_Index[i] = 0;
		for (j = 0; j < lead_dim; j++)
		{
			tmp_dataset[i * lead_dim + j] = (double) rand() / RAND_MAX;
    	}
  	}
}


void clusterInitialization(data_struct *data_in,data_struct *cluster_in)
{
	int i, j, pick = 0;
	int lead_dim = cluster_in->leading_dim;
	int second_dim = cluster_in->secondary_dim;
	int Objects = data_in->secondary_dim;
	double *tmp_Centroids = cluster_in->dataset;
	double *tmp_dataset = data_in->dataset;
	unsigned int *tmp_Sizes = data_in->members;

	int step = Objects / second_dim;

	/*randomly pick initial cluster centers*/
	for (i = 0; i < second_dim; i++)
	{
		for (j = 0; j < lead_dim; j++)
		{
      		tmp_Centroids[i * lead_dim + j] = tmp_dataset[pick * lead_dim + j];
    	}
		pick += step; 
	}	
}

void print(data_struct* data2print)
{
	int i, j = 0;
	int lead_dim = data2print->leading_dim;
	int second_dim = data2print->secondary_dim;
	double *tmp_dataset = data2print->dataset;

  
	for (i = 0; i < second_dim; i++)
	{
		for (j = 0; j < lead_dim; j++)
		{
      		printf("%f ", tmp_dataset[i * lead_dim + j]);
    	}
    	printf("\n");
  	}
}

void clean(data_struct* data1){

  free(data1->dataset);
  free(data1->members);
}


int main(int argc, char **argv)
{
	int i, processors, rank; 
	struct timeval first, second, lapsed;
	struct timezone tzp;
	
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &processors);
	
	if (argc<4 && rank == 0)
	{
		printErrorMessage();
		return 0;
	}
	if(rank == 0)
	printf("\n======================Report======================\n");
	int numObjects = atoi(argv[1]);
	int numAttributes = atoi(argv[2]);
	int numClusters = atoi(argv[3]);

	data_struct data_in;
	data_struct clusters;

	//Memory Allocation
	data_in.leading_dim = numAttributes;
	data_in.secondary_dim = numObjects;
	data_in.dataset = (double*)malloc(numObjects * numAttributes * sizeof(double));
	data_in.members = (unsigned int*)malloc(numObjects * sizeof(unsigned int));

	clusters.leading_dim = numAttributes;
	clusters.secondary_dim = numClusters;
	clusters.dataset = (double*)malloc(numClusters * numAttributes * sizeof(double));
	clusters.members = (unsigned int*)malloc(numClusters * sizeof(unsigned int)); 


	//Initialize
	initializeRandomDatapoints(&data_in);

	if (rank == 0)
	{
		clusterInitialization(&data_in, &clusters);
 	 }
	
	MPI_Barrier(MPI_COMM_WORLD);

	// Broadcasts a message from the process with rank "root" to all other processes of the communicator 	
	MPI_Bcast(clusters.dataset, numClusters*numAttributes, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	data_struct p_data;
	p_data.leading_dim = numAttributes;
	double n_split =  numObjects / processors;
	double p_objects = ceil(n_split);
	int n_temp = p_objects *  processors;
	if (rank != 0)
	{
		p_data.secondary_dim = p_objects;
		p_data.dataset = (double*)malloc(p_objects * numAttributes * sizeof(double));
		p_data.members = (unsigned int*)malloc(p_objects * sizeof(unsigned int)); 
	}
	else
	{
		p_data.secondary_dim = p_objects + (numObjects - n_temp);
		p_data.dataset = (double*)malloc(p_data.secondary_dim * numAttributes * sizeof(double));
		p_data.members = (unsigned int*)malloc(p_data.secondary_dim * sizeof(unsigned int)); 	
	}
	printf("\nProcess %d and size: %d", rank, p_data.secondary_dim);

	// Here, every process creates a sub-dataset of its elements
	for (i = 0; i < p_data.secondary_dim * p_data.leading_dim;i++)
	{ 
		p_data.dataset[i] = data_in.dataset[rank * p_data.secondary_dim * p_data.leading_dim+i]; 
	}
	if (rank == 0)
	{
		gettimeofday(&first, &tzp);
	}

	int iter, j, k;

	double SumOfDist = 0, new_SumOfDist=0, temp_SumOfDist=0;
	double* newCentroids;
	int* temp_clusterSize;
	unsigned int*temp_dataMembers;
	temp_clusterSize = (int*)malloc(numClusters * sizeof(int));
	temp_dataMembers = (unsigned int*)malloc(numObjects * sizeof(unsigned int));

	// Our new centroids (used in processkMeans)
	newCentroids = (double*)malloc(numAttributes * numClusters * sizeof(double));

	// Initialize all cluster sizes to zero
	for (i = 0; i < numClusters; i++)
	{
		temp_clusterSize[i] = 0;
	}

	// Make sure that all processes start clustering at the same time
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
	{
		printf("\n\Clustering.....!\n");
	}
	// Start clustering
	for (iter = 0; iter < max_iterations; iter++)
	{
		new_SumOfDist = 0;
		temp_SumOfDist = 0;

		for (i = 0; i < numClusters * numAttributes; i++)
		{
			newCentroids[i] = 0;
		}

		processkMeans(&p_data, &clusters, newCentroids, &new_SumOfDist);

		// "Pass" centroids to the struct
		MPI_Allreduce(newCentroids, clusters.dataset, numClusters*numAttributes, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
		MPI_Allreduce(clusters.members, temp_clusterSize, numClusters, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);  

		// "Pass" cluster sizes to the struct
		for (i = 0; i < numClusters; i++)
		{
       		clusters.members[i] = temp_clusterSize[i];
		}

		// Update cluster centers
		for (i = 0; i < numClusters; i++)
		{
			for (j = 0; j < numAttributes; j++)
			{
				clusters.dataset[i * numAttributes + j] /= (double) clusters.members[i];
			}
		}
		MPI_Allreduce(&new_SumOfDist, &temp_SumOfDist, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

		// If we have reached threshold, stop clustering
		if (fabs(SumOfDist - temp_SumOfDist) < threshold)
		{
			break;
		}
		SumOfDist = temp_SumOfDist;
		if (rank == 0)
		{
			printf("\nSum of Distances of iteration %d: %f", iter + 1, SumOfDist);
		}
	}
	free(newCentroids);
	free(temp_clusterSize);
	
	MPI_Barrier(MPI_COMM_WORLD);
	for (i = 0; i < p_data.secondary_dim; i++) 
	{ 
		temp_dataMembers[rank * p_data.secondary_dim + i] = p_data.members[i]; 
	} 

	// Merge the above data
	MPI_Allreduce(temp_dataMembers, data_in.members, numObjects, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);

	free(temp_dataMembers);

	// Wait for all processes to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0)
	{
		printf("\n\nFinished after %d iterations\n", iter);
  
		gettimeofday(&second, &tzp);

		if (first.tv_usec > second.tv_usec)
		{
			second.tv_usec += 1000000;
			second.tv_sec--;
		}
  
		lapsed.tv_usec = second.tv_usec - first.tv_usec;
		lapsed.tv_sec = second.tv_sec - first.tv_sec;

		printf("Time elapsed: %d.%06dsec\n\n", (int)lapsed.tv_sec, (int)lapsed.tv_usec); 

		printf("Cluster sizes\n");
		for (i = 0; i < numClusters; i++)
		{
       		printf("Cluster%d: %d\n", i,clusters.members[i]);
		}
		printf("\n");
	}

	/*============clean memory===========*/
	clean(&p_data);	
	clean(&data_in);
	clean(&clusters);
	if(rank == 0)
	{
		printf("\n===================================================\n");
	}
	MPI_Finalize();
}
