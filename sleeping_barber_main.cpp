//
//  main.cpp
//  barber
//
//  Created by Yaroslav Zarechnyy on 10/6/19.
//  Copyright © 2019 Yaroslav Zarechnyy. All rights reserved.
//
//
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<errno.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <dispatch/dispatch.h>
#include <iostream>

#define N 5

using namespace std;

time_t end_time;/*end time*/
sem_t customers,barbers; /*Three semaphors*/
int counter = 0;/*The number of customers waiting for haircuts*/
mutex mtx;

dispatch_semaphore_t customer_sem, barber_sem;

void *barber(void *arg);
void *customer(void *arg);

int main(int argc,char *argv[])
{
    pthread_t barber_thread, customer_thread;

    end_time=time(NULL)+20;/*Barber Shop Hours is 20s*/
    /*Semaphore initialization*/
    customer_sem = dispatch_semaphore_create(0);
    barber_sem = dispatch_semaphore_create(1);

    mtx.unlock();
    
    /*Barber_thread initialization*/
    pthread_create(&barber_thread, nullptr, barber, &counter);
    /*Customer_thread initialization*/
    pthread_create(&customer_thread, nullptr, customer, &counter);
    
    /*Customer_thread first blocked*/
    pthread_join(customer_thread, nullptr);
    pthread_join(barber_thread, nullptr);
    
    exit(0);
}

void *barber(void *arg)/*Barber Process*/
{
    while(time(NULL)<end_time || counter > 0)
    {
//        if (counter > 0) {
            dispatch_semaphore_wait(customer_sem, DISPATCH_TIME_FOREVER);
            mtx.lock();
            counter--;
            printf("Barber:cut hair,count is:%d.\n",counter);
            if (counter == 0) {
                printf("Barber fall asleep\n");
            }
            mtx.unlock();
            dispatch_semaphore_signal(barber_sem);
            sleep(3);
        
//        }
    }
    return nullptr;
}

void *customer(void *arg)/*Customers Process*/
{
    while(time(NULL)<end_time)
    {
        mtx.lock();
        if(counter < N )
        {
            counter++;
            printf("Customer:add count,count is:%d\n",counter);
            mtx.unlock();
            dispatch_semaphore_signal(customer_sem);
            dispatch_semaphore_wait(barber_sem, DISPATCH_TIME_NOW);
            
        }
        else
        /*If the number is full of customers,just put the mutex lock let go*/
            printf("Customer is leaving\n");
            mtx.unlock();
        sleep(1);
    }
    return nullptr;
}
