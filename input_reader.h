#ifndef INPUT_READER_H
#define INPUT_READER_H

#include <pthread.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INPUT_BUFFER_CELL_SIZE 256
#define INPUT_BUFFER_CELL_COUNT 5

typedef struct inputReaderData {
    unsigned int activeThreads;
    char **buf;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} IR_DATA;

void inputReaderData_init(IR_DATA *data, unsigned int activeThreads);

void inputReaderData_destroy(IR_DATA *data);

void inputReaderData_decrementActiveThreads(IR_DATA *data);

unsigned int inputReaderData_getActiveThreads(IR_DATA *data);

void initInputBuffer(char ***buf);

void destroyInputBuffer(char ***buf);

void copyInputBuffer(char **src, char **dest);

void zeroInputBuffer(char **buf);

bool compareInputBuffer(char **src, char **dst);


void *signalUserInput(void *data);

void readInput(char **buf);

#ifdef __cplusplus
}
#endif
#endif
