

#define PRODUCT_COUNT 20
#define CLIENT_COUNT 5
#define ORDERS_PER_CLIENT 10

// struct of product
typedef struct {
    char description[50];
    float price;
    int item_count;
    int total_requests;
    int total_sold;
    char unsatisfied_clients[100][50];
    int unsatisfied_count;
} Product;



void initialize_catalog(Product catalog[PRODUCT_COUNT]);

void print_report(Product catalog[PRODUCT_COUNT]);

void process_order(int product_id, char *client_name, int client_pipe[2], Product catalog[PRODUCT_COUNT]);
