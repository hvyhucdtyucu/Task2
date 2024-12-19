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
 
 printf("Initializing...\n");
 for (int i=0; i<productCOUNT; i++) {
  strcpy(catalog[i].desc, "something something");
  srand(time(NULL));
  catalog[i].price = (float)((rand() % 1000) / 10.0); // 0.0-99.9
  catalog[i].item_count = 2;
  catalog[i].total_requests = 0;
  catalog[i].total_sold = 0;
  catalog[i].not_received = 0;
  }
 }
 
 void print_report() {
  int f_orders = 0;
  int f_sold=0;
  int f_failure = 0;
  float f_income = 0;
  
  for (int i = 0; i< productCOUNT; i++) {
   printf("Product: %d\n", i);
   
   printf("%s: ", catalog[i].desc);
   printf("Requests: %d ", catalog[i].total_requests);
   printf("Sales: %d ", catalog[i].total_sold);
   printf("Failed Requests: %d \n", catalog[i].not_received);
   
    f_orders += catalog[i].total_requests;
    f_sold += catalog[i].total_sold;
    f_failure += catalog[i].not_received;
    f_income += catalog[i].total_sold * catalog[i].price;
    }
    
    printf("Total orders: %d\n", f_orders);
    printf("Total successful orders: %d\n", f_sold);
    printf("Total failed orders: %d\n", f_failure);
    printf("Total income: %f\n", f_income);
    
  exit(0);
   }
   
   int main(int argc, char* argv[]) {
   
   int p1[clientCOUNT][2]; //child => father
   int p2[clientCOUNT][2]; // father => child
   
   pid_t pid;
   
   
   
   initialize_catalog();
   
   
 
    
   for (int i = 0; i < clientCOUNT; i++) {
    //open all pipes
     pipe(p1[i]);
     pipe(p2[i]);
    
    
    pid = fork();
    
    if (pid == 0) {
     //child
    close(p1[i][0]);
    close(p2[i][1]);
    
    
    
     printf("client %d inside\n", i+1);
     for (int j =0; j < ordersPerCLIENT; j++) {
     
      srand(time(NULL));    
      int product_id = rand() % productCOUNT;
      write(p1[i][1], &product_id, sizeof(int));
      
      
      char response[100];
      read(p2[i][0], response , sizeof(response));
      printf("Customer %d: %s\n", i+1, response);
      
      
      sleep(1);
      
     }
       printf("client %d done\n", i+1);
     
    } else {
    //father
     close(p1[i][1]);
     close(p2[i][0]);
        
    
    
    while (i< i*ordersPerCLIENT) {
    int id;
    
    read(p1[i][0], &id, sizeof(int));
    
    catalog[id].total_requests++;
    
    printf("total requests: %d\n", catalog[id].total_requests);
    
    
    printf("Request received for Product %d from Customer %d and %d remain\n", id, i+1, catalog[id].item_count);
    char response[100];
    
    if (catalog[id].item_count > 0) {
     catalog[id].item_count--;
     catalog[id].total_sold++;
     strcpy(response, "Successful purchase\n");
     
    } else {
     strcpy(response, "Out of stock\n");
     catalog[id].not_received++;
     
    }
    
    write(p2[i][1], response, strlen(response) + 1);
 
    
    //sleep(1); // wait 1 sec
     }
     close(p1[i][0]);
     close(p2[i][1]);
    
   }
      
     
     
   
  }
    for (int k = 0; k < clientCOUNT; k++) {
     wait(NULL);
     
     }
     
     print_report();
     
   return 0;
    }
   
   
   
 
