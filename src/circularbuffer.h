/**
 * @file circularbuffer.h
 * @author Johannes Felzmann <e11912368@student.tuwien.ac.at>
 * @date 31.10.2020
 *
 * @brief circularbuffer header module
 *
 * This module contains functions to implement a circular buffer, shared memory and semaphores.
 **/

#ifndef CIRCULARBUFFER_H__ /* prevent multiple inclusion */
#define CIRCULARBUFFER_H__

#define MAX_BUFFER 8
#define MAX_EDGES 8
#define SHM_NAME "/11912368mybuffershm"
#define SEM_FREE "/11912368semfree"
#define SEM_WRITE "/11912368semwrite"
#define SEM_USED "/11912368semused"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

/* structs */
typedef struct mynode{
	int num;
	int color; /* 0: red, 1: green, 2: blue */
} mynode;

typedef struct myedge{
	mynode n1;
	mynode n2;
} myedge;

typedef struct mygraph{
	myedge edges[MAX_EDGES];
	int cnt;
} mygraph;

typedef struct myBuffer{
	mygraph graphs[MAX_BUFFER];
	int write_position;
	int read_position;
	int terminate_state;
} myBuffer;

struct myBuffer *buffer;

int shmfd;

/* semaphores */
sem_t *free_sem;
sem_t *write_sem;
sem_t *used_sem;

/**
 * Initializes shared memory and semaphores for the supervisor.
 * @brief This function creates the shared memory and opens semaphores.
 * @details The shared memory object is created, the size of the shared memor gets set, the shared memory gets mapped, all semaphores get opened.
 *  Also error handling for any specific case is realised.
 */
void initialize_supervisor();

/**
 * Releases shared memory and semaphores for the supervisor.
 * @brief This function closes and unlinks the shared memory and semaphores.
 * @details The shared memory gets unmaped and closed, also the shared memory gets unlinked, all sempahores get closed and unlinked.
 * Also error handling for any specific case is realised.
 */
void release_supervisor();

/**
 * Initializes shared memory and semaphores for the generator.
 * @brief This function opens the shared memory and opens semaphores.
 * @details The shared memory is opened and mapped, also all semaphores are opened.
 * Error handling when needed in any specific case.
 * @return A return value to check in the generator if he should terminate and free all dynamic memory.
 */
int initialize_generator();

/**
 * Releases shared memory and semaphores for the generator.
 * @brief This function closes the shared memory and semaphores.
 * @details  The shared memor gets unmapped and closed, all semaphores are closed.
 * Also error handling for any specific case is realised. 
 */
void release_generator();

/**
 * Writes graph to the buffer.
 * @brief This function writes a solution to the ciruclar buffer.
 * @details The function writes a solution in form of a graph struct to the circular buffer. 
 * A solution consists of the edges wich are removed to make to graph 3-colorable. 
 * There are semaphores to synchronize the writing process. The semaphore write_sem is there 
 * to guarantee that only one generator writes at a time. The semophore free_sem is for the free space in the buffer.
 * The write position gets incremented by one after writing and if its greater then the buffer size it starts again at 0.
 * The semaphore used_sem gets incremented so that an element can be read from the buffer.
 * Also there is error handling for the sem_wait()-function.
 * @param val The solution as a graph which gets written to the circular buffer. 
 */
void buffer_write(mygraph val);

/**
 * Reads graph from the buffer.
 * @brief This function reads a solution from the circular buffer.
 * @details The function reads a solution from the ciurcular buffer and returns the value. A semaphore used_sem is used for 
 * synchronizing the reading process. A solution gets selected from the buffer, then the read position gets incremented 
 * by one and if its greater then the buffer size it starts again at 0. Then the semaphore free_sem gets incremented,
 * so the buffer can write a solution again on this position. In the end the solution from the buffer gets returned.
 * Also there is error handling for the sem_wait()-function.
 * @return The graph which gets read.
 */
mygraph buffer_read();

/**
 * Prints graph
 * @brief This function prints a graph.
 * @details The function prints all edges of the graph in one line like (d-d).
 * @param g The graph which should be printed.
 */
void print_graph(mygraph g);

#endif /* CIRCULARBUFFER_H__ */
