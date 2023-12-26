#ifndef K_DEFINITIONS_H
#define	K_DEFINITIONS_H
#include "input_reader.h"

#include <pthread.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define USER_LENGTH 10

  typedef struct data {
    IR_DATA *inputReaderData;
    char userName[USER_LENGTH + 1];
    char peerUserName[USER_LENGTH + 1];
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int socket;
    int stop;
  } DATA;

  void data_init(DATA *data, IR_DATA *inputReaderData, const char *userName,
                 const int socket);
  void data_destroy(DATA *data);
  void data_stop(DATA *data);
  int data_isStopped(DATA *data);
  void data_setPeerUserName(DATA *data, char *userName);
  void data_getPeerUserName(DATA * data, char * userName);

  void printError(char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* K_DEFINITIONS_H */

