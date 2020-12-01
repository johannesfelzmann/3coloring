/**
 * @file supervisor.c
 * @author Johannes Felzmann <e11912368@student.tuwien.ac.at>
 * @date 31.10.2020
 *
 * @brief supervisor program module
 *
 * This program is the supervisor.
 **/

#include "circularbuffer.h"
#include <signal.h>

volatile sig_atomic_t quit = 0;

/**
 * Handles a signal.
 * @brief This function handles a signal.
 * @details The function is called wenn a signal is received, therefore then the 
 * supervisor messages the generator that it should terminate and the semaphore 
 * free_sem gets incremented and the supervisor releases all resources and 
 * terminates itself.
 * @param signal The signal received.
 */
void handle_signal(int signal){
	buffer->terminate_state = 1;
	sem_post(free_sem);
	release_supervisor();
	exit(EXIT_FAILURE);
}

/**
 * Program entry point supervisor.
 * @brief The program starts here.
 * @details The program first checks if there are no arguments given, if there are arguments it terminates.
 * Then the signal handler gets set up and the initialize method for the supervisor gets called to initialize 
 * resources. The main task is to read constant solutions from the circular buffer. If a solution is better then 
 * the solution before it gets printed to the stdout. If a solution with 0 edges gets read, the supervisor terminates
 * and send a message to the generator to terminate aas well.
 * @param argc The number of arguments.
 * @param argv The arguments itself.
 * @return Return 0 if programm terminated successful, > 0 otherwise.
 */
int main(int argc, char *argv[]){

	/* no arguments allowed in supervisor */
	if(argc > 1) {
		fprintf(stderr, "No arguments allowed in supervisor!");
		return EXIT_FAILURE;
	}

	/* set up signal handling */
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	/* set up shared memory
	 set up semaphores
	 initialize circularbuffer */
	initialize_supervisor();

	mygraph solution;
	mygraph best_solution;
	best_solution.cnt = MAX_EDGES;

	while(!quit){

		solution = buffer_read();

		/* if new solution is better than old solution write to stdout */
		if(solution.cnt < best_solution.cnt){

			/* graph is acyclic */
			if(solution.cnt == 0){
				printf("The graph is 3-colorable!\n");
				buffer->terminate_state = 1;
				quit = 1;
				sem_post(free_sem);
			}else{
				best_solution = solution;
				printf("Solution with %d edges: ", best_solution.cnt);
				print_graph(best_solution);
			}
			
		}
	}

	release_supervisor();
	return EXIT_SUCCESS;
}
