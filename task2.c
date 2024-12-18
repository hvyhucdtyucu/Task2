#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define productCOUNT 20
#define clientCOUNT 5
#define ordersPerCLIENT 10


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
 
 for (int i=0; i<productCOUNT; i++) {
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
  int total_sold=0;
  int total_failure = 0;
  float total_income = 0;
  
  for (int i = 0; i< productCOUNT; i++) {
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
   
   void process_order(int product_id, int client_pipe[2]) {
    char response[100];
    
    catalog[product_id].total_requests++;
    
    if (catalog[product_id].item_count > 0) {
     catalog[product_id].item_count--;
     catalog[product_id].total_sold++;
     strcpy(response, "Successful purchace");
    } else {
     strcpy(response, "Out of stock");
     catalog[product_id].not_received++;
    }
    
    write(client_pipe[1], response, strlen(response) + 1);
    sleep(1); // wait 1 sec
    
   }
   
   
  int main() {
   int father_to_child[clientCOUNT][2], child_to_father[clientCOUNT][2];
   pid_t pids[clientCOUNT];
   
   srand(time(NULL));
   initialize_catalog();
   
   for (int i = 0; i < clientCOUNT; i++) {
    pipe(father_to_child[i]);
    pipe(child_to_father[i]);
    
    if ((pids[i] = fork()) == 0) {
     close(father_to_child[i][1]);
     close(child_to_father[i][0]);
     
     for (int j =0; j < ordersPerCLIENT; j++) {
     
      int product_id = rand() % productCOUNT;
      write(child_to_father[i][1], &product_id, sizeof(int));
      char response[100];
      read(father_to_child[i][0], response , sizeof(response));
      printf("Customer %d: %s\n", i+1, response);
      
      sleep(1);
      
     }
     
     close(father_to_child[i][0]);
     close(child_to_father[i][1]);
     exit(0);
    } else {
     close(father_to_child[i][0]);
     close(child_to_father[i][1]);
    }
    
   }
   
   for (int i = 0; i < clientCOUNT * ordersPerCLIENT; i++) {
    int product_id;
    read(child_to_father[i % clientCOUNT][0], &product_id, sizeof(int));
    process_order(product_id, father_to_child[i % clientCOUNT]);
   }
   
   for (int i =0; i<clientCOUNT; i++) {
    wait(NULL);
   }
   
   print_report();
   
   return 0;
   
  }
