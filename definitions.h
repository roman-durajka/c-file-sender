#ifndef K_DEFINITIONS_H
#define	K_DEFINITIONS_H

#include <pthread.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define USER_LENGTH 10
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
  void initInputBuffer(char ***buf);
  void destroyInputBuffer(char ***buf);
  void copyInputBuffer(char ** src, char ** dest);
  void zeroInputBuffer(char ** buf);
  bool compareInputBuffer(char ** src, char ** dst);

  typedef struct data {
    IR_DATA *inputReaderData;
    char userName[USER_LENGTH + 1];
    char peerUserName[USER_LENGTH + 1];
    pthread_mutex_t mutex;
    int socket;
    int stop;
  } DATA;

  void data_init(DATA *data, IR_DATA * inputReaderData, const char* userName, const int socket);
  void data_destroy(DATA *data);
  void data_stop(DATA *data);
  int data_isStopped(DATA *data);

  void printError(char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* K_DEFINITIONS_H */

