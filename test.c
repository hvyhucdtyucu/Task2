#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PRODUCT_COUNT 20
#define CLIENT_COUNT 5
#define ORDERS_PER_CLIENT 10

typedef struct {
    char description[50];
    float price;
    int item_count;
    int total_requests;
    int total_sold;
} Product;

// Κατάλογος προϊόντων
Product catalog[PRODUCT_COUNT];

// Αρχικοποίηση καταλόγου
void initialize_catalog() {
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        snprintf(catalog[i].description, 50, "Product %d", i);
        catalog[i].price = (float)(rand() % 100 + 1); // Τιμές από 1-100
        catalog[i].item_count = 2; // 2 διαθέσιμα τεμάχια
        catalog[i].total_requests = 0;
        catalog[i].total_sold = 0;
    }
}

// Εξυπηρέτηση παραγγελίας
void process_order(int product_id, int client_id, int *write_pipe) {
    char message[100];
    catalog[product_id].total_requests++;
    if (catalog[product_id].item_count > 0) {
        catalog[product_id].item_count--;
        catalog[product_id].total_sold++;
        snprintf(message, 100, "Order successful for Client %d. Total cost: %.2f\n", client_id, catalog[product_id].price);
    } else {
        snprintf(message, 100, "Order failed for Client %d. Product %d out of stock.\n", client_id, product_id);
    }
    write(write_pipe[1], message, strlen(message) + 1);
}

int main() {
    int client_to_server[CLIENT_COUNT][2];
    int server_to_client[CLIENT_COUNT][2];
    pid_t pid;

    srand(time(NULL));
    initialize_catalog();

    // Δημιουργία pipes και πελατών
    for (int i = 0; i < CLIENT_COUNT; i++) {
        pipe(client_to_server[i]);
        pipe(server_to_client[i]);

        pid = fork();
        if (pid == 0) { // Πελάτης
            close(client_to_server[i][0]); // Κλείνουμε read για τον πελάτη
            close(server_to_client[i][1]); // Κλείνουμε write για τον πελάτη

            for (int j = 0; j < ORDERS_PER_CLIENT; j++) {
                int product_id = rand() % PRODUCT_COUNT;
                write(client_to_server[i][1], &product_id, sizeof(int));
                char response[100];
                read(server_to_client[i][0], response, sizeof(response));
                printf("Client %d: %s", i, response);
                sleep(1);
            }
            exit(0);
        } else if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Πατρική διεργασία
    for (int i = 0; i < CLIENT_COUNT; i++) {
        close(client_to_server[i][1]); // Κλείνουμε write για τον εξυπηρετητή
        close(server_to_client[i][0]); // Κλείνουμε read για τον εξυπηρετητή
    }

    int completed_clients = 0;
    while (completed_clients < CLIENT_COUNT) {
        for (int i = 0; i < CLIENT_COUNT; i++) {
            int product_id;
            if (read(client_to_server[i][0], &product_id, sizeof(int)) > 0) {
                process_order(product_id, i, server_to_client[i]);
                sleep(1); // Χρόνος διεκπεραίωσης παραγγελίας
            } else {
                completed_clients++;
            }
        }
    }

    // Εκτύπωση αναφοράς
    printf("\nFinal Report:\n");
    int total_orders = 0, total_successful = 0, total_failed = 0;
    float total_revenue = 0;

    for (int i = 0; i < PRODUCT_COUNT; i++) {
        printf("Product: %s\n", catalog[i].description);
        printf("  Requests: %d\n", catalog[i].total_requests);
        printf("  Sold: %d\n", catalog[i].total_sold);
        total_orders += catalog[i].total_requests;
        total_successful += catalog[i].total_sold;
        total_revenue += catalog[i].total_sold * catalog[i].price;
    }

    total_failed = total_orders - total_successful;
    printf("\nSummary:\n");
    printf("  Total Orders: %d\n", total_orders);
    printf("  Successful Orders: %d\n", total_successful);
    printf("  Failed Orders: %d\n", total_failed);
    printf("  Total Revenue: %.2f\n", total_revenue);

    return 0;
}
