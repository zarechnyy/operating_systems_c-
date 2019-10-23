//
//  main.cpp
//  philosophers
//
//  Created by Yaroslav Zarechnyy on 10/10/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//

#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <semaphore.h>
#include <queue>
#include <zconf.h>
#include <list>
#include <stdio.h>
#include <stdlib.h>

#define FORKS 3
#define PHIL 3

using namespace std;

void *functionPhilosophers(void *arg);
int max = 3;
pthread_mutex_t forks_mtx[FORKS];
pthread_t phil_threads[PHIL];
pthread_mutex_t mtx, mtx_1;

int main() {
    
    pthread_mutex_init(&mtx, NULL);
    
    for (int i = 0; i < FORKS; i++) {
        pthread_mutex_init(&forks_mtx[i], NULL);
    }

    for(int i = 0; i < PHIL; i++) {
        printf("%d\n", i);
        int num = i;
        pthread_create(&phil_threads[i], nullptr, functionPhilosophers, (void* )&num);
        sleep(2);
    }

    for (int i = 0; i < PHIL; i++) {
        pthread_join(phil_threads[i], nullptr);
    }

    return 0;
}

void *functionPhilosophers(void *arg) {
    int phil_num = *(int *) (arg);
    int a = phil_num;
    int b = (phil_num + 1) % PHIL;
    while (1) {
        if (pthread_mutex_trylock(&forks_mtx[a]) == 0) {
            if (pthread_mutex_trylock(&forks_mtx[b]) == 0) {
                pthread_mutex_lock(&mtx);
                printf("Philosopher number %d eats with forks %d %d\n", a, a, b);
                pthread_mutex_unlock(&mtx);
                sleep(1);
                pthread_mutex_unlock(&forks_mtx[b]);

                pthread_mutex_lock(&mtx);
                printf("Philosopher number %d put forks %d %d\n", a, a, b);
                pthread_mutex_unlock(&mtx);
            }
        pthread_mutex_unlock(&forks_mtx[a]);
        sleep(rand() % 10);
        }
    }
}
