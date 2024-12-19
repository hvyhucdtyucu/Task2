#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PRODUCT_COUNT 20
#define CLIENT_COUNT 5
#define ORDERS_PER_CLIENT 10

// Δομή προϊόντος
typedef struct {
    char description[50];
    float price;
    int item_count;
    int total_requests;
    int total_sold;
    char unsatisfied_clients[100][50];
    int unsatisfied_count;
} Product;

Product catalog[PRODUCT_COUNT];

void initialize_catalog() {
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        snprintf(catalog[i].description, sizeof(catalog[i].description), "Product %d", i + 1);
        catalog[i].price = (float)((rand() % 1000) / 10.0); // Τιμή από 0.0 έως 99.9
        catalog[i].item_count = 2; // Αρχικό στοκ
        catalog[i].total_requests = 0;
        catalog[i].total_sold = 0;
        catalog[i].unsatisfied_count = 0;
    }
}

void print_report() {
    int total_orders = 0, total_success = 0, total_failure = 0;
    float total_revenue = 0;

    printf("\n--- Συγκεντρωτική Αναφορά ---\n");
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        printf("%s:\n", catalog[i].description);
        printf("  Αιτήματα: %d\n", catalog[i].total_requests);
        printf("  Πωλήσεις: %d\n", catalog[i].total_sold);
        printf("  Μη εξυπηρετημένοι: ");
        for (int j = 0; j < catalog[i].unsatisfied_count; j++) {
            printf("%s ", catalog[i].unsatisfied_clients[j]);
        }
        printf("\n");

        total_orders += catalog[i].total_requests;
        total_success += catalog[i].total_sold;
        total_failure += catalog[i].unsatisfied_count;
        total_revenue += catalog[i].total_sold * catalog[i].price;
    }

    printf("\nΣυνολικές Παραγγελίες: %d\n", total_orders);
    printf("Επιτυχείς Παραγγελίες: %d\n", total_success);
    printf("Αποτυχημένες Παραγγελίες: %d\n", total_failure);
    printf("Συνολικός Τζίρος: %.2f\n", total_revenue);
}

void process_order(int product_id, char *client_name, int client_pipe[2]) {
    char response[100];

    catalog[product_id].total_requests++;

    if (catalog[product_id].item_count > 0) {
        catalog[product_id].item_count--;
        catalog[product_id].total_sold++;
        snprintf(response, sizeof(response), "Παραγγελία επιτυχής: %s (%.2f)", catalog[product_id].description, catalog[product_id].price);
    } else {
        strcpy(response, "Αποτυχία: Δεν υπάρχει διαθεσιμότητα.");
        strcpy(catalog[product_id].unsatisfied_clients[catalog[product_id].unsatisfied_count++], client_name);
    }

    write(client_pipe[1], response, strlen(response) + 1);
    sleep(1); // Χρόνος διεκπεραίωσης
}

int main() {
    int parent_to_client[CLIENT_COUNT][2], client_to_parent[CLIENT_COUNT][2];
    pid_t pids[CLIENT_COUNT];

    srand(time(NULL));
    initialize_catalog();

    for (int i = 0; i < CLIENT_COUNT; i++) {
        pipe(parent_to_client[i]);
        pipe(client_to_parent[i]);

        if ((pids[i] = fork()) == 0) {
            close(parent_to_client[i][1]);
            close(client_to_parent[i][0]);

            for (int j = 0; j < ORDERS_PER_CLIENT; j++) {
                int product_id = rand() % PRODUCT_COUNT;
                write(client_to_parent[i][1], &product_id, sizeof(int));

                char response[100];
                read(parent_to_client[i][0], response, sizeof(response));
                printf("Πελάτης %d: %s\n", i + 1, response);

                sleep(1);
            }

            close(parent_to_client[i][0]);
            close(client_to_parent[i][1]);
            exit(0);
        } else {
            close(parent_to_client[i][0]);
            close(client_to_parent[i][1]);
        }
    }

    for (int i = 0; i < CLIENT_COUNT * ORDERS_PER_CLIENT; i++) {
        int product_id;
        read(client_to_parent[i % CLIENT_COUNT][0], &product_id, sizeof(int));
        process_order(product_id, "Client", parent_to_client[i % CLIENT_COUNT]);
    }

    for (int i = 0; i < CLIENT_COUNT; i++) {
        wait(NULL);
    }

    print_report();

    return 0;
}




