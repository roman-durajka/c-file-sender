#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

char *endMsg = ":end";

void data_init(DATA *data, const char* userName, const int socket) {
	data->socket = socket;
	data->stop = 0;
	data->userName[USER_LENGTH] = '\0';
	strncpy(data->userName, userName, USER_LENGTH);
	pthread_mutex_init(&data->mutex, NULL);
}

void data_destroy(DATA *data) {
	pthread_mutex_destroy(&data->mutex);
}

void data_stop(DATA *data) {
  pthread_mutex_lock(&data->mutex);
  data->stop = 1;
  pthread_mutex_unlock(&data->mutex);
}

int data_isStopped(DATA *data) {
  int stop;
  pthread_mutex_lock(&data->mutex);
  stop = data->stop;
  pthread_mutex_unlock(&data->mutex);
  return stop;
}

void printError(char *str) {
  if (errno != 0) {
		perror(str);
	} else {
		fprintf(stderr, "%s\n", str);
	}
  exit(EXIT_FAILURE);
}
