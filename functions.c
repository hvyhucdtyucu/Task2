#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "header.h"

void initialize_catalog(Product catalog[PRODUCT_COUNT]) {
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        snprintf(catalog[i].description, sizeof(catalog[i].description), "Product %d", i + 1);
        catalog[i].price = (float)((rand() % 1000) / 10.0); // value 0.0~99.9
        catalog[i].item_count = 2; // starting stock
        catalog[i].total_requests = 0;
        catalog[i].total_sold = 0;
        catalog[i].unsatisfied_count = 0;
    }
}


void print_report(Product catalog[PRODUCT_COUNT]) {
    int total_orders = 0, total_success = 0, total_failure = 0;
    float total_revenue = 0;

    printf("\n--- Summary ---\n");
    for (int i = 0; i < PRODUCT_COUNT; i++) {
        printf("%s:\n", catalog[i].description);
        printf("  Requests: %d\n", catalog[i].total_requests);
        printf("  Sales: %d\n", catalog[i].total_sold);
        printf("  Not received: ");
        for (int j = 0; j < catalog[i].unsatisfied_count; j++) {
            printf("%s ", catalog[i].unsatisfied_clients[j]);
        }
        printf("\n");

        total_orders += catalog[i].total_requests;
        total_success += catalog[i].total_sold;
        total_failure += catalog[i].unsatisfied_count;
        total_revenue += catalog[i].total_sold * catalog[i].price;
    }

    printf("\nTotal Orders: %d\n", total_orders);
    printf("Successful Orders: %d\n", total_success);
    printf("Failed Orders: %d\n", total_failure);
    printf("Total Revenue: %.2f\n", total_revenue);
}

void process_order(int product_id, char *client_name, int client_pipe[2], Product catalog[PRODUCT_COUNT]) {
    char response[100];

    catalog[product_id].total_requests++;

    if (catalog[product_id].item_count > 0) {
        catalog[product_id].item_count--;
        catalog[product_id].total_sold++;
        snprintf(response, sizeof(response), "Purchace successful: %s (%.2f)", catalog[product_id].description, catalog[product_id].price);
    } else {
        strcpy(response, "Failed: Item is not in stock");
        strcpy(catalog[product_id].unsatisfied_clients[catalog[product_id].unsatisfied_count++], client_name);
    }

    write(client_pipe[1], response, strlen(response) + 1);
    sleep(1); // 1 sec delay
}
