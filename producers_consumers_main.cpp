//
//  main.cpp
//  products
//
//  Created by Yaroslav Zarechnyy on 10/6/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//

#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <semaphore.h>
#include <queue>
#include <zconf.h>
#include <dispatch/dispatch.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>

#include <iterator>

#define PRODS 2
#define CONSUMER 4

using namespace std;

dispatch_semaphore_t prod_sem, cust_sem;
pthread_mutex_t prod_mtx, cust_mtx, mtx;

pthread_mutex_t mtx_4, mtx_5;

int max_size = 5;

void *functionCostumer(void *);

void *functionProducer(void *);

typedef struct node_list{
    int value;
    struct node_list * next;
}node;
typedef struct List{
    struct node_list *head;
    struct node_list * tail;
    int size;
}List;
List* createList() {
    List * hlist = (List *)malloc(sizeof(List));
    hlist->head = NULL;
    hlist->tail = hlist->head;
    hlist->size = 0;
    return hlist;
}
void printList(List & hlist){
    node * cur = hlist.head;
    while(cur!= NULL ){
        printf("%d ",cur->value);
        cur = cur->next;
    }
}
void push(List & hlist) {
    srand(time(NULL));
    if(hlist.size == 0) {
        hlist.head = (node *)malloc(sizeof(node));
        hlist.head->value = rand()%100 ;
        hlist.head->next = NULL;
        hlist.tail = hlist.head;
        pthread_mutex_lock(&mtx_4);
        hlist.size++;
        printf("Item is pushed, size %d: ", hlist.size);
        printList(hlist);
        printf("\n");
        pthread_mutex_unlock(&mtx_4);
    } else {
        node * current = hlist.tail;
        hlist.tail->next = (node *)malloc(sizeof(node));
        hlist.tail->next->value = rand()%100;
        hlist.tail = hlist.tail->next;
        hlist.tail->next = NULL;
        pthread_mutex_lock(&mtx_4);
        hlist.size++;
        printf("Item is pushed, size %d: ", hlist.size);
        printList(hlist);
        printf("\n");
        pthread_mutex_unlock(&mtx_4);
    }
    
}
int pop(List & hlist) {
    if(hlist.size == 0) {
        return -1;
    }
    else{
        node * cur = hlist.head->next;
        int value = hlist.head->value;
        free(hlist.head);
        hlist.head=cur;
        pthread_mutex_lock(&mtx_4);
        hlist.size--;
        printf("Item is poped, size %d: ", hlist.size);
        printList(hlist);
        printf("\n");
        pthread_mutex_unlock(&mtx_4);
        return value;
    }
}

int main() {
    
    List * queue = createList();

    pthread_mutex_init(&mtx, NULL);
    pthread_mutex_init(&prod_mtx, NULL);
    pthread_mutex_init(&cust_mtx, NULL);
    
    pthread_mutex_init(&mtx_4, NULL);
    
    prod_sem = dispatch_semaphore_create(max_size);
    cust_sem = dispatch_semaphore_create(0);
    
    pthread_t cust_threads[CONSUMER];
    pthread_t prod_threads[PRODS];
    
    for(int i = 0; i < CONSUMER; i++) {
        pthread_create(&cust_threads[i], nullptr, functionCostumer, (void *)queue);
    }
    
    for (int i = 0; i < PRODS; i++) {
        pthread_create(&prod_threads[i], nullptr, functionProducer, (void *)queue);
    }
    
    for (int i = 0; i < CONSUMER; i++) {
        pthread_join(cust_threads[i], nullptr);
    }
    
    for (int i = 0; i < PRODS; i++) {
        pthread_join(prod_threads[i], nullptr);
    }

    dispatch_release(prod_sem);
    dispatch_release(cust_sem);
    return 0;
}

void *functionCostumer(void *arg) {
    for (;;) {
        List *items = (List *) arg;
        dispatch_semaphore_wait(cust_sem, DISPATCH_TIME_FOREVER);
        pthread_mutex_lock(&cust_mtx);
        pthread_mutex_lock(&mtx);
        if(items->size==1){
            pop(*items);
            pthread_mutex_unlock(&mtx);
        }else {
            pthread_mutex_unlock(&mtx);
            pop(*items);
        }
        sleep(3);
        pthread_mutex_unlock(&cust_mtx);
        dispatch_semaphore_signal(prod_sem);
    }
    return nullptr;
}

void *functionProducer(void *arg) {
    for (;;) {
        List *items = static_cast<List *>(arg);

        dispatch_semaphore_wait(prod_sem, DISPATCH_TIME_FOREVER);
        pthread_mutex_lock(&prod_mtx);
        pthread_mutex_lock(&mtx);
        
        if (items->size <= 1) {
            push(*items);
            pthread_mutex_unlock(&mtx);
//            dispatch_semaphore_signal(prod_sem);
//        }else if (items->size > 5) {
//            pthread_mutex_unlock(&mtx);
        }else {
            pthread_mutex_unlock(&mtx);
            push(*items);
//            dispatch_semaphore_signal(prod_sem);
        }
        sleep(1);
        pthread_mutex_unlock(&prod_mtx);
        dispatch_semaphore_signal(cust_sem);
    }
    return nullptr;
    
}
