#include "input_reader.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

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

void inputReaderData_decrementActiveThreads(IR_DATA *data) {
  pthread_mutex_lock(&data->mutex);
  data->activeThreads--;
  pthread_mutex_unlock(&data->mutex);
}

unsigned int inputReaderData_getActiveThreads(IR_DATA *data) {
  unsigned int activeThreads;
  pthread_mutex_lock(&data->mutex);
  activeThreads = data->activeThreads;
  pthread_mutex_unlock(&data->mutex);

  return activeThreads;
}

void copyInputBuffer(char **src, char **dest) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    memcpy(dest[i], src[i], INPUT_BUFFER_CELL_SIZE);
  }
}

void *signalUserInput(void *data) {
  IR_DATA *pdata = (IR_DATA *) data;

  while (inputReaderData_getActiveThreads(pdata) > 0) {
    pthread_mutex_lock(&pdata->mutex);

    zeroInputBuffer(pdata->buf);
    readInput(pdata->buf);
    pthread_cond_broadcast(&pdata->condition);

    pthread_mutex_unlock(&pdata->mutex);
    usleep(100000);
  }

  return NULL;
}

void readInput(char **buf) {
  int tempSize = INPUT_BUFFER_CELL_COUNT * INPUT_BUFFER_CELL_SIZE + 1;
  char temp[tempSize + 1];
  int bufIndex = 0;
  fgets(temp, tempSize, stdin);

  char *strtokPtr;
  char delimeter = ' ';
  char *splitStr = strtok_r(temp, &delimeter, &strtokPtr);
  while (splitStr != NULL && bufIndex < INPUT_BUFFER_CELL_COUNT) {
    memcpy(*buf, splitStr, strlen(splitStr) * sizeof(char));
    if (*(*buf + strlen(splitStr) - 1) == '\n') {
      *(*buf + strlen(splitStr) - 1) = '\0';
    }
    splitStr = strtok_r(NULL, &delimeter, &strtokPtr);
    buf++;
    bufIndex++;
  }
}
