/**
 * @file circularbuffer.c
 * @author Johannes Felzmann <e11912368@student.tuwien.ac.at>
 * @date 31.10.2020
 *
 * @brief circularbuffer program module
 *
 * This program contains the implementation of a circular buffer with shared memory and semaphores.
 * Also some other functions which are used by generator and supervisor.
 **/

#include "circularbuffer.h"

void buffer_write(mygraph val){
	if (sem_wait(write_sem) == -1) {
		if (errno != EINTR){
			fprintf(stderr, "ERROR: sem_wait write_sem failed\n");
			exit(EXIT_FAILURE);
		}
	}
	if (sem_wait(free_sem) == -1) {
		if (errno != EINTR){
			fprintf(stderr, "ERROR: sem_wait free_sem failed\n");
			exit(EXIT_FAILURE);
		}
	}
	buffer->graphs[buffer->write_position] = val;
	sem_post(used_sem);
	buffer->write_position += 1;
	buffer->write_position %= MAX_BUFFER;
	sem_post(write_sem);
}

mygraph buffer_read(){
	if (sem_wait(used_sem) == -1) {
		if (errno != EINTR){
			fprintf(stderr, "ERROR: sem_wait used_sem failed\n");
			exit(EXIT_FAILURE);
		}
	}
	mygraph ret = buffer->graphs[buffer->read_position];
	sem_post(free_sem);
	buffer->read_position += 1;
	buffer->read_position %= MAX_BUFFER;
	return ret;
}

void initialize_supervisor(){

	/* create and/or open the shared memory object */
	shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

	/* error handling create/open*/
	if(shmfd == -1){
		fprintf(stderr, "ERROR: create/open shared memory failed\n");
		exit(EXIT_FAILURE);
	}

	/* set size of the shared memory and error handling */
	if( ftruncate(shmfd, sizeof(struct myBuffer)) < 0){
		fprintf(stderr, "ERROR: ftrucate failed\n");
		exit(EXIT_FAILURE);
	}

	/* map shared memory */
	buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	/* error handling mapping */
	if(buffer == MAP_FAILED){
		fprintf(stderr, "ERROR: mapping failed\n");
		exit(EXIT_FAILURE);
	}

	/* create semaphores and error handling open semaphores */
	free_sem = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_BUFFER);
	if(free_sem == SEM_FAILED){
		fprintf(stderr, "ERROR: semaphore free open failed\n");
		exit(EXIT_FAILURE);
	}

	used_sem = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
	if(used_sem == SEM_FAILED){
		fprintf(stderr, "ERROR: semaphore used open failed\n");
		exit(EXIT_FAILURE);
	}

	write_sem = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
	if(write_sem == SEM_FAILED){
		fprintf(stderr, "ERROR: semaphore write open failed\n");
		exit(EXIT_FAILURE);
	}

	buffer->terminate_state = 0;
	buffer->write_position = 0;
	buffer->read_position = 0;

}

int initialize_generator(){

	/* create and/or open the shared memory object */
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);

	/* error handling create/open*/
	if(shmfd == -1){
		fprintf(stderr, "ERROR: open shared memory failed\n");
		return 1;
	}

	/* map shared memory */
	buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	/* error handling mapping */
	if(buffer == MAP_FAILED){
		fprintf(stderr, "ERROR: mapping failed\n");
		return 1;
	}

	/* create semaphores and error handling open semaphores */
	/* if open failed, supervisor is not running */
	free_sem = sem_open(SEM_FREE, MAX_BUFFER);
	if(free_sem == SEM_FAILED){
		return 1;
	}

	used_sem = sem_open(SEM_USED, 0);
	if(used_sem == SEM_FAILED){
		return 1;
	}

	write_sem = sem_open(SEM_WRITE, 1);
	if(write_sem == SEM_FAILED){
		return 1;
	}

	return 0;
}

void release_supervisor(){

	/* unmap shared memory */
	if (munmap(buffer, sizeof(*buffer)) == -1){
		fprintf(stderr, "ERROR: unmap shared memory failed\n");
		exit(EXIT_FAILURE);
	}

	if (close(shmfd) == -1){
		fprintf(stderr, "ERROR: shmfd closing failed\n");
		exit(EXIT_FAILURE);
	}

	/* remove shared memory object */
	if (shm_unlink(SHM_NAME) == -1){
		fprintf(stderr, "ERROR: unlinking shared memory failed!\n");
		exit(EXIT_FAILURE);
	}

	/* close and unlink semaphores */
	if(sem_close(free_sem) == -1){
		fprintf(stderr, "ERROR: free_sem closing failed\n");
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM_FREE) == -1){
		fprintf(stderr, "ERROR: unlinking sem_free failed!\n");
		exit(EXIT_FAILURE);
	}

	if(sem_close(used_sem) == -1){
		fprintf(stderr, "ERROR: used_sem closing failed\n");
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM_USED) == -1){
		fprintf(stderr, "ERROR: unlinking sem_used failed!\n");
		exit(EXIT_FAILURE);
	}

	if(sem_close(write_sem) == -1){
		fprintf(stderr, "ERROR: write_sem closing failed\n");
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(SEM_WRITE) == -1){
		fprintf(stderr, "ERROR: unlinking sem_write failed!\n");
		exit(EXIT_FAILURE);
	}

}

void release_generator(){

	/* unmap shared memory */
	if (munmap(buffer, sizeof(*buffer)) == -1){
		fprintf(stderr, "ERROR: unmap shared memory failed!\n");
		exit(EXIT_FAILURE);
	}

	if (close(shmfd) == -1){
		fprintf(stderr, "ERROR: shmfd closing failed\n");
		exit(EXIT_FAILURE);
	}

	/* close and unlink semaphores */
	if(sem_close(free_sem) == -1){
		fprintf(stderr, "ERROR: free_sem closing failed!\n");
		exit(EXIT_FAILURE);
	}

	if(sem_close(used_sem) == -1){
		fprintf(stderr, "ERROR: used_sem closing failed!\n");
		exit(EXIT_FAILURE);
	}

	if(sem_close(write_sem) == -1){
		fprintf(stderr, "ERROR: write_sem closing failed!\n");
		exit(EXIT_FAILURE);
	}

}

void print_graph(mygraph g){
    for (int i = 0; i < g.cnt; i++){
        printf("(%d-%d) ",g.edges[i].n1.num, g.edges[i].n2.num);
    }
    printf("\n");
}