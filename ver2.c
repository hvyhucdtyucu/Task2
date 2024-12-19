#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define productCOUNT 20
#define clientCOUNT 5
#define ordersPerCLIENT 10

typedef struct {
    int product_id;
    int total_requests;
    int total_sold;
    int not_received;
} Update;

typedef struct {
    char desc[50];
    float price;
    int item_count;
    int total_requests;
    int total_sold;
    int not_received;
} Product;

Product catalog[productCOUNT];

void initialize_catalog() {
    for (int i = 0; i < productCOUNT; i++) {
        strcpy(catalog[i].desc, "something something");
        catalog[i].price = (float)((rand() % 1000) / 10.0); // 0.0-99.9
        catalog[i].item_count = 2;
        catalog[i].total_requests = 0;
        catalog[i].total_sold = 0;
        catalog[i].not_received = 0;
    }
}

void print_report() {
    int total_orders = 0;
    int total_sold = 0;
    int total_failure = 0;
    float total_income = 0;

    for (int i = 0; i < productCOUNT; i++) {
        printf("Product: %d\n", i);
        printf("%s: ", catalog[i].desc);
        printf("Requests: %d ", catalog[i].total_requests);
        printf("Sales: %d ", catalog[i].total_sold);
        printf("Failed Requests: %d \n", catalog[i].not_received);

        total_orders += catalog[i].total_requests;
        total_sold += catalog[i].total_sold;
        total_failure += catalog[i].not_received;
        total_income += catalog[i].total_sold * catalog[i].price;
    }

    printf("Total orders: %d\n", total_orders);
    printf("Total successful orders: %d\n", total_sold);
    printf("Total failed orders: %d\n", total_failure);
    printf("Total income: %f\n", total_income);
}

int main() {
    int father_to_child[clientCOUNT][2], child_to_father[clientCOUNT][2];
    pid_t pid;

    initialize_catalog();

    srand(time(NULL));

    for (int i = 0; i < clientCOUNT; i++) {
        pipe(father_to_child[i]);
        pipe(child_to_father[i]);

        pid = fork();

        if (pid == 0) {
            // Child process
            close(father_to_child[i][1]); // Close write end of parent-to-child pipe
            close(child_to_father[i][0]); // Close read end of child-to-parent pipe

            for (int j = 0; j < ordersPerCLIENT; j++) {
                int product_id = rand() % productCOUNT;

                // Send product request to parent
                write(child_to_father[i][1], &product_id, sizeof(int));

                // Receive response from parent
                char response[100];
                read(father_to_child[i][0], response, sizeof(response));
                printf("Customer %d: %s\n", i + 1, response);
            }

            close(father_to_child[i][0]);
            close(child_to_father[i][1]);
            exit(0);
        }
    }

    // Parent process
    for (int i = 0; i < clientCOUNT; i++) {
        close(father_to_child[i][0]); // Close read end of parent-to-child pipe
        close(child_to_father[i][1]); // Close write end of child-to-parent pipe
    }

    for (int i = 0; i < clientCOUNT * ordersPerCLIENT; i++) {
        int product_id;
        Update update;

        // Read product request from child
        read(child_to_father[i % clientCOUNT][0], &product_id, sizeof(int));

        // Process the order
        update.product_id = product_id;
        update.total_requests = 1;
        if (catalog[product_id].item_count > 0) {
            catalog[product_id].item_count--;
            update.total_sold = 1;
            update.not_received = 0;
            strcpy(update.desc, "Successful purchase\n");
        } else {
            update.total_sold = 0;
            update.not_received = 1;
            strcpy(update.desc, "Out of stock\n");
        }

        // Send response to child
        write(father_to_child[i % clientCOUNT][1], update.desc, strlen(update.desc) + 1);

        // Update catalog in the parent
        catalog[product_id].total_requests += update.total_requests;
        catalog[product_id].total_sold += update.total_sold;
        catalog[product_id].not_received += update.not_received;
    }

    for (int i = 0; i < clientCOUNT; i++) {
        wait(NULL);
    }

    for (int i = 0; i < clientCOUNT; i++) {
        close(father_to_child[i][1]);
        close(child_to_father[i][0]);
    }

    print_report();

    return 0;
}
