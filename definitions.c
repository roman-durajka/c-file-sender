#include "definitions.h"
#include "input_reader.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

char *endMsg = ":end";

void data_init(DATA *data, IR_DATA *inputReaderData, const char *userName, const int socket) {
  data->inputReaderData = inputReaderData;
  data->socket = socket;
  data->stop = 0;
  data->userName[USER_LENGTH] = '\0';
  bzero(data->peerUserName, USER_LENGTH);
  data->peerUserName[USER_LENGTH] = '\0';
  strncpy(data->userName, userName, USER_LENGTH);
  pthread_mutex_init(&data->mutex, NULL);
  pthread_cond_init(&data->condition, NULL);
}

void data_destroy(DATA *data) {
  pthread_mutex_destroy(&data->mutex);
  pthread_cond_destroy(&data->condition);
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

void data_setPeerUserName(DATA *data, char *userName) {
  pthread_mutex_lock(&data->mutex);
  memcpy(data->peerUserName, userName, USER_LENGTH);
  pthread_cond_signal(&data->condition);
  pthread_mutex_unlock(&data->mutex);
}

void data_getPeerUserName(DATA *data, char *userName) {
  pthread_mutex_lock(&data->mutex);
  while (strcmp(data->peerUserName, "") == 0) {
    pthread_cond_wait(&data->condition,
                      &data->mutex);
  }
  memcpy(userName, data->peerUserName, USER_LENGTH);
  pthread_mutex_unlock(&data->mutex);
}

void printError(char *str) {
  if (errno != 0) {
    perror(str);
  } else {
    fprintf(stderr, "%s\n", str);
  }
  exit(EXIT_FAILURE);
}
