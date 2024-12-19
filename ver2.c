#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define PRODUCT_COUNT 20
#define CLIENT_COUNT 5
#define ORDERS_PER_CLIENT 10

typedef struct {
    char desc[50];
    float price;
    int item_count;
    int total_requests;
    int total_sold;
    int not_received;
} Product;

Product catalog[PRODUCT_COUNT];

void initialize_catalog() {
    srand(time(NULL)); // Initialize random seed
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        snprintf(catalog[i].desc, 50, "Product %d", i + 1);
        catalog[i].price = (float)((rand() % 1000) / 10.0); // Prices from 0.0 to 99.9
        catalog[i].item_count = 2; // Initial stock
        catalog[i].total_requests = 0;
        catalog[i].total_sold = 0;
        catalog[i].not_received = 0;
    }
}

void print_report() {
    int total_orders = 0;
    int total_sold = 0;
    int total_failed = 0;
    float total_income = 0;

    printf("\n=== Final Report ===\n");
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        printf("Product: %s\n", catalog[i].desc);
        printf("  Requests: %d\n", catalog[i].total_requests);
        printf("  Sales: %d\n", catalog[i].total_sold);
        printf("  Failed Requests: %d\n", catalog[i].not_received);

        total_orders += catalog[i].total_requests;
        total_sold += catalog[i].total_sold;
        total_failed += catalog[i].not_received;
        total_income += catalog[i].total_sold * catalog[i].price;
    }

    printf("\nSummary:\n");
    printf("  Total Orders: %d\n", total_orders);
    printf("  Successful Orders: %d\n", total_sold);
    printf("  Failed Orders: %d\n", total_failed);
    printf("  Total Income: $%.2f\n", total_income);
}

int main() {
    int father_to_child[CLIENT_COUNT][2], child_to_father[CLIENT_COUNT][2];
    pid_t pid;

    initialize_catalog();
    srand(time(NULL));

    for (int i = 0; i < CLIENT_COUNT; i++) {
        pipe(father_to_child[i]);
        pipe(child_to_father[i]);

        pid = fork();

        if (pid == 0) { // Child process
            close(father_to_child[i][1]); // Close unused write end
            close(child_to_father[i][0]); // Close unused read end

            for (int j = 0; j < ORDERS_PER_CLIENT; j++) {
                int product_id = rand() % PRODUCT_COUNT;
                write(child_to_father[i][1], &product_id, sizeof(int)); // Send order

                char response[100];
                read(father_to_child[i][0], response, sizeof(response)); // Receive response
                printf("Customer %d: %s", i + 1, response);

                sleep(1); // Simulate time between orders
            }

            close(father_to_child[i][0]);
            close(child_to_father[i][1]);
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process
    for (int i = 0; i < CLIENT_COUNT; i++) {
        close(father_to_child[i][0]); // Close unused read end
        close(child_to_father[i][1]); // Close unused write end
    }

    for (int i = 0; i < CLIENT_COUNT * ORDERS_PER_CLIENT; i++) {
        for (int j = 0; j < CLIENT_COUNT; j++) {
            int product_id;
            if (read(child_to_father[j][0], &product_id, sizeof(int)) > 0) {
                catalog[product_id].total_requests++;
                char response[100];

                if (catalog[product_id].item_count > 0) {
                    catalog[product_id].item_count--;
                    catalog[product_id].total_sold++;
                    snprintf(response, sizeof(response), "Order successful! Total cost: $%.2f\n", catalog[product_id].price);
                } else {
                    catalog[product_id].not_received++;
                    snprintf(response, sizeof(response), "Order failed! Product %d is out of stock.\n", product_id);
                }

                write(father_to_child[j][1], response, strlen(response) + 1); // Send response
            }
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < CLIENT_COUNT; i++) {
        wait(NULL);
    }

    for (int i = 0; i < CLIENT_COUNT; i++) {
        close(father_to_child[i][1]);
        close(child_to_father[i][0]);
    }

    print_report();
    return 0;
}

