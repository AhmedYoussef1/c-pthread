#include <pthread.h>

struct station {
	int waiters;
	int seats;
	int on_board;
	pthread_mutex_t lock;
	pthread_cond_t train_arrived, train_loaded;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);