#include "definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

char *endMsg = ":end";


void initInputBuffer(char ***buf) {
  *buf = calloc(INPUT_BUFFER_CELL_COUNT, sizeof(char *));
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    (*buf)[i] = calloc(INPUT_BUFFER_CELL_SIZE, sizeof(char));
    (*buf)[i][INPUT_BUFFER_CELL_SIZE - 1] = '\0';
  }
}

void destroyInputBuffer(char ***buf) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    free((*buf)[i]);
  }
  free(*buf);
  *buf = NULL;
}

void zeroInputBuffer(char **buf) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    bzero(buf[i], INPUT_BUFFER_CELL_SIZE);
    buf[i][INPUT_BUFFER_CELL_SIZE - 1] = '\0';
  }
}

bool compareInputBuffer(char **src, char **dst) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    if (strcmp(src[i], dst[i]) != 0) {
      return false;
    }
  }
  return true;
}

void inputReaderData_init(IR_DATA *data, unsigned int activeThreads) {
  data->activeThreads = activeThreads;
  initInputBuffer(&data->buf);
  pthread_mutex_init(&data->mutex, NULL);
  pthread_cond_init(&data->condition, NULL);
}

void inputReaderData_destroy(IR_DATA *data) {
  destroyInputBuffer(&data->buf);
  pthread_mutex_destroy(&data->mutex);
  pthread_cond_destroy(&data->condition);
}

void copyInputBuffer(char **src, char **dest) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    memcpy(dest[i], src[i], INPUT_BUFFER_CELL_SIZE);
  }
}

void data_init(DATA *data, IR_DATA * inputReaderData, const char* userName, const int socket) {
  data->inputReaderData = inputReaderData;
	data->socket = socket;
	data->stop = 0;
	data->userName[USER_LENGTH] = '\0';
  bzero(data->peerUserName, USER_LENGTH);
  data->peerUserName[USER_LENGTH] = '\0';
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
