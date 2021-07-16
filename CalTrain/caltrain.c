#include <pthread.h>
#include "caltrain.h"
#include <stdio.h>

void
station_init(struct station *station)
{
	station->on_board = 0;
	station->waiters = 0;
	pthread_mutex_init(&(station->lock), NULL);
	pthread_cond_init(&(station->train_arrived), NULL); 
	pthread_cond_init(&(station->train_loaded), NULL);
}

void
station_load_train(struct station *station, int count)
{
	station->seats = count;

	pthread_mutex_lock(&(station->lock));
		while(station->seats > 0 && station->waiters > 0){
			pthread_cond_broadcast(&(station->train_arrived));
			pthread_cond_wait(&(station->train_loaded), &(station->lock));
		}
	pthread_mutex_unlock(&(station->lock));

	station->seats = 0;
}

void
station_wait_for_train(struct station *station)
{
	pthread_mutex_lock(&(station->lock));
		station->waiters ++;
		while(station->on_board >= station->seats)
			pthread_cond_wait(&(station->train_arrived), &(station->lock));
		station->on_board ++;
	pthread_mutex_unlock(&(station->lock));
}

void
station_on_board(struct station *station)
{
	pthread_mutex_lock(&(station->lock));
    		station->on_board --;
		station->waiters --;
		station->seats --;
	pthread_mutex_unlock(&(station->lock));
	
	if((station->seats <= 0 || station->waiters <= 0))
		pthread_cond_signal(&(station->train_loaded));
}
