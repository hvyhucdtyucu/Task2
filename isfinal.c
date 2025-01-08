#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "header.h"

Product catalog[PRODUCT_COUNT]; //declare catalog

int main() {
    int parent_to_client[CLIENT_COUNT][2], client_to_parent[CLIENT_COUNT][2];
    pid_t pids[CLIENT_COUNT];

    srand(time(NULL));
    initialize_catalog(catalog);

    for (int i = 0; i < CLIENT_COUNT; i++) {
        pipe(parent_to_client[i]); // create pipe for parent-to-client 
        pipe(client_to_parent[i]); // create pipe for client-to-parent
// write = 1 read =0
        if ((pids[i] = fork()) == 0) { // child process
            close(parent_to_client[i][1]); //close unused write in child
            close(client_to_parent[i][0]); // unused read in child

            for (int j = 0; j < ORDERS_PER_CLIENT; j++) {
                srand(time(NULL) ^ getpid());
                int product_id = rand() % PRODUCT_COUNT;
                write(client_to_parent[i][1], &product_id, sizeof(int)); //sends item id to parent

                char response[100];
                read(parent_to_client[i][0], response, sizeof(response)); // reads response from parent
                printf("Customer %d: %s\n", i + 1, response);

                sleep(1);
            }
             //close remaining pipes in child
            close(parent_to_client[i][0]);
            close(client_to_parent[i][1]);
            exit(0);
        } else { //parent
            close(parent_to_client[i][0]); //close unsuded read in parent
            close(client_to_parent[i][1]); // close unused write in parent
        }
    }

    for (int i = 0; i < CLIENT_COUNT * ORDERS_PER_CLIENT; i++) {
        int product_id;
        read(client_to_parent[i % CLIENT_COUNT][0], &product_id, sizeof(int)); //read product id from client
        process_order(product_id, "Some Client", parent_to_client[i % CLIENT_COUNT], catalog);
    }
//wait for all child processes to finish
    for (int i = 0; i < CLIENT_COUNT; i++) {
        wait(NULL);
    }

    print_report(catalog);

    return 0;
}
