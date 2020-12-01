/**
 * @file generator.c
 * @author Johannes Felzmann <e11912368@student.tuwien.ac.at>
 * @date 31.10.2020
 *
 * @brief generator program module
 *
 * This program is the generator.
 **/

#include "circularbuffer.h"
#include <time.h>
#include <regex.h>

/**
 * Program entry point generator.
 * @brief The program starts here.
 * @details The program needs more than two arguments, otherwise it terminates. The generator first 
 * opens all resources and then creates a random coloring of the input edges and therefore writes 
 * a solution, which is 3-colorable as a graph to the circular buffer as long as it gets no message 
 * from the supervisor to terminate. When it terminates it releases all resources.
 * @param argc The number of arguments.
 * @param argv The arguments itself.
 * @return Return 0 if programm terminated successful, > 0 otherwise.
 */
int main(int argc, char *argv[]){

    /* check if arguments are given */
    if(argc < 2){
        fprintf(stderr,"ERROR: no valid arguments given!\n");
        return EXIT_FAILURE;
    }

    char *regexString = "[0-9]+-[0-9]+";
    regex_t regexCompiled;

    if (regcomp(&regexCompiled, regexString, REG_EXTENDED|REG_NOSUB)){
        fprintf(stderr, "ERROR: could not compile regular expression.\n");
        return EXIT_FAILURE;
    };

    int edgecnt = argc-1;
    int nodecnt = 0;
    mygraph graph;
    graph.cnt = 0;
    int writeflag = 1;

    myedge *edges;
    edges = malloc(edgecnt * sizeof(myedge));

    int rcol;

    /* initialize random number generator, use current time as seed */
    srand(time(0));

    int n1 = 0;
    int n2 = 0;
    char c = '-';
    /* get edges */
    for(int i = 1; i < argc ; ++i){

        if(regexec(&regexCompiled, argv[i], 0, NULL, 0)){
            fprintf(stderr, "ERROR: please use right input format!\n");
            free(edges);
            return EXIT_FAILURE;
        }

        sscanf(argv[i],"%d %c %d",&n1, &c, &n2);

        /* get number of nodes */
        if(n1 > nodecnt){
            nodecnt = n1;
        }
        if(n2 > nodecnt){
            nodecnt = n2;
        }

        edges[i-1].n1.num = n1;
        edges[i-1].n2.num = n2;
    }
    nodecnt++;

    int ret = initialize_generator();

    if(ret > 0){
        free(edges);
        return EXIT_FAILURE;
    }

    /* start with 3coloring algortihm */ 
    while(!buffer->terminate_state){
        graph.cnt = 0;
        writeflag = 1;


        /* generate random coloring */
        for (int i = 0; i < nodecnt; i++){
            rcol = rand()%3;

            for (int j = 0; j < edgecnt; j++){
                if(edges[j].n1.num == i){
                    edges[j].n1.color = rcol;
                }
                if(edges[j].n2.num == i){
                    edges[j].n2.color = rcol;
                }
            }
        }

        /* testing coloring */
        /*for (size_t i = 0; i < edgecnt; i++){
            printf("(%d[%d]-%d[%d])   ", edges[i].n1.num, edges[i].n1.color, edges[i].n2.num, edges[i].n2.color);
        }
        printf("\n");*/
        

        /* remove identical edges */
        for (int i = 0; i < edgecnt; i++){
           if(edges[i].n1.color == edges[i].n2.color){
               if(graph.cnt < MAX_EDGES){
                   graph.edges[graph.cnt] = edges[i];
                   graph.cnt++;
               }else{
                   writeflag = 0;
               }
           }
        }

        /* dont write if all nodes are removed*/
        if(nodecnt == graph.cnt){
            writeflag = 0;
        }

        /*print_graph(graph);*/

        /* write if solution has not more than 8 edges */
        if(writeflag){
            buffer_write(graph);
        }

    }

    free(edges);
    release_generator();

    return EXIT_SUCCESS;
}

