all:
	mpicc kmeans.c kmeansUsingMPI.c -o kmeans -lm -w
	@echo echo mpiexec -np n ./kmeans data dim cluster
clean:
	rm -f *.o *.out *.bin  kmeans
