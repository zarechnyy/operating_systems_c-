//
//  main.cpp
//  readers
//
//  Created by Yaroslav Zarechnyy on 10/8/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>

#define READERS 8
#define WRITERS 2

using namespace std;

pthread_cond_t reader_cond, writer_cond;

pthread_mutex_t mtx_output, doors, mp, mtx_writer_order, mtx_reader_order;

int writer_counter = 0, reader_counter = 0;

int writters_list[WRITERS];
int readers_list[READERS];

void *writers(void *params) {
    
    int id = *(int *) params;
    
    
    for (;;) {
        pthread_cond_wait(&writer_cond, &mp);
        
        pthread_mutex_lock(&mtx_writer_order);
        writer_counter++;
        pthread_mutex_unlock(&mtx_writer_order);
        
        pthread_mutex_lock(&doors);
        
        pthread_mutex_lock(&mtx_output);
        cout<< "writer "<<id<<" writes"<<endl;
        pthread_mutex_unlock(&mtx_output);
        
        sleep(rand() %2);
        
        pthread_mutex_lock(&mtx_output);
        cout<< "writer "<<id<<" finished writing"<<endl;
        pthread_mutex_unlock(&mtx_output);
        
        pthread_mutex_unlock(&doors);
        
        pthread_mutex_lock(&mtx_writer_order);
        writer_counter--;
        
        if (writer_counter == 0) {
            pthread_cond_broadcast(&reader_cond);
        }
        pthread_mutex_unlock(&mtx_writer_order);
        
        sleep(rand() % 3 + 5);
    }
    return nullptr;
}

void *readers(void *params) {
    int id = *(int *) params;
    for (;;) {
        
        
        pthread_cond_wait(&reader_cond, &mp);
        
        pthread_mutex_lock(&mtx_reader_order);
        reader_counter++;
        pthread_mutex_unlock(&mtx_reader_order);
        
        pthread_mutex_lock(&mtx_output);
        cout<< "reader "<<id<<" reads"<<endl;
        pthread_mutex_unlock(&mtx_output);
        
        sleep(rand() % 2);
        
        pthread_mutex_lock(&mtx_output);
        cout<< "reader "<<id<<" finished reading"<<endl;
        pthread_mutex_unlock(&mtx_output);
        
        
        pthread_mutex_lock(&mtx_reader_order);
        reader_counter--;
        if (reader_counter == 0) {
            pthread_cond_broadcast(&writer_cond);
        }
        pthread_mutex_unlock(&mtx_reader_order);
        
        sleep(rand() % 3 + 2);
    }
    return nullptr;
}

int main() {
    
    pthread_mutex_init(&mtx_output, NULL);
    pthread_mutex_init(&doors, NULL);
    pthread_mutex_init(&mp, NULL);
    
    pthread_cond_init(&reader_cond, NULL);
    pthread_cond_init(&writer_cond, NULL);
    
    pthread_mutex_init(&mtx_writer_order, NULL);
    pthread_mutex_init(&mtx_reader_order, NULL);
    
    pthread_t readers_threads[READERS];
    pthread_t writers_threads[WRITERS];
    
    for (int i = 0; i < WRITERS; i++) {
        writters_list[i] = i;
        pthread_create(&writers_threads[i], nullptr, writers, &writters_list[i]);
    }
    
    sleep(2);
    pthread_cond_broadcast(&writer_cond);
    
    for (int i = 0; i < READERS; i++) {
        readers_list[i] = i;
        pthread_create(&readers_threads[i], nullptr, readers, &readers_list[i]);
    }
    
    for (int i = 0; i < READERS; i++) {
        pthread_join(readers_threads[i], nullptr);
    }
    
    for (int i = 0; i < WRITERS; i++) {
        pthread_join(writers_threads[i], nullptr);
    }
    
    return 0;
}
