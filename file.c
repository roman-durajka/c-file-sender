#include "file.h"
#include "definitions.h"
#include "input_reader.h"
#include "huffman.h"
#include "lzw.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

bool loadFileData(char *fileName, char **fileData, int *fileSize) {
  FILE *f = fopen(fileName, "rb");
  if (!f) {
    return false;
  }

  //get file size
  fseek(f, 0, SEEK_END);
  *fileSize = ftell(f);
  rewind(f);

  *fileData = calloc(*fileSize, sizeof(char));
  if (!fileData) {
    printf("[system] Error allocating memory on file %s.\n", fileName);
    fclose(f);
    return false;
  }

  if (fread(*fileData, 1, *fileSize, f) != *fileSize) {
    fclose(f);
    free(*fileData);
    return false;
  }

  fclose(f);

  return true;
}

bool writeFileData(char *fileName, const char *fileData, int fileSize) {
  FILE *f = fopen(fileName, "wb");
  if (!f) {
    return false;
  }

  if (fwrite(fileData, 1, fileSize, f) != fileSize) {
    fclose(f);
    return false;
  }

  fclose(f);

  return true;
}

void data_writeData(int socket, void *data, size_t dataSize) {
  size_t sentBytesTotal = 0;

  while (sentBytesTotal < dataSize) {
    size_t sentBytes = write(socket, (unsigned char *) data + sentBytesTotal, dataSize - sentBytesTotal);
    sentBytesTotal += sentBytes;
  }
}

void *sendData(void *data) {
  DATA *pdata = (DATA *) data;
  //send username
  data_writeData(pdata->socket, pdata->userName, USER_LENGTH);

  //initialize input buffer where input from user will be saved
  char **inputBuffer = NULL;
  initInputBuffer(&inputBuffer);

  //save peer username into variable
  char peerUserName[USER_LENGTH + 1];
  data_getPeerUserName(pdata, peerUserName);

  while (!data_isStopped(pdata)) {
    //use mutex and signal to read input from user and copy it to local variable
    pthread_mutex_lock(&pdata->inputReaderData->mutex);
    while (compareInputBuffer(pdata->inputReaderData->buf, inputBuffer)) {
      pthread_cond_wait(&pdata->inputReaderData->condition, &pdata->inputReaderData->mutex);
    }
    copyInputBuffer(pdata->inputReaderData->buf, inputBuffer);
    pthread_mutex_unlock(&pdata->inputReaderData->mutex);

    // check again if connection has ended while waiting for user input
    if (data_isStopped(pdata)) {
      continue;
    }

    if (strcmp(inputBuffer[0], "quit") == 0) {
      // send quit to peer
      printf("[send][%s] Ending connection... Notifying peer.\n", peerUserName);
      data_writeData(pdata->socket, inputBuffer[0], INPUT_BUFFER_CELL_SIZE);
      data_stop(pdata);
      continue;
    }

    //check for wrong first argument
    if (strcmp(inputBuffer[0], "") == 0) {
      continue;
    }
    if (strcmp(inputBuffer[0], "huffman") != 0 &&
        strcmp(inputBuffer[0], "lzw") != 0 &&
        strcmp(inputBuffer[0], "plain") != 0) {
      printf("[send][%s] Wrong input. Try again with correct mode - "
             "plain/huffman/lzw.\n",
             peerUserName);
      continue;
    }

    //send only to specified recepients
    if (strcmp(inputBuffer[1], "all") != 0 && strcmp(inputBuffer[1], peerUserName) != 0) {
      continue;
    }

    // for every specified file
    for (int i = 2; i < INPUT_BUFFER_CELL_COUNT; i++) {
      if (strcmp(inputBuffer[i], "") == 0) {
        break;
      }

      // load file into variable
      char *fileData;
      int fileSize;
      bool result = loadFileData(inputBuffer[i], &fileData, &fileSize);
      if (!result) {
        printf("[send][%s] Could not read the file \"%s\". Skpping...\n",
               peerUserName, inputBuffer[i]);
        continue;
      } else {
        printf("[send][%s] Successfuly read file.\n", peerUserName);
      }

      // send compression method
      data_writeData(pdata->socket, inputBuffer[0], INPUT_BUFFER_CELL_SIZE);

      // if huffman then compress using huffman before sending
      if (strcmp(inputBuffer[0], "huffman") == 0) {
        // one bit is saved as byte, so size * 8
        // + 1 so \0 can be added
        char *huffmanEncoded = calloc(fileSize * 8 + 1, sizeof(char));
        if (huffmanEncoded == NULL) {
          printf("[send][%s] Failed to allocate (probably too big) chunk of memory. Skipping...\n", peerUserName);
          free(fileData);
          continue;
        }
        int encodedSize;
        unsigned int *frequencies = calloc(TREE_NODE_COUNT, sizeof(unsigned int));
        huffmanEncode(fileData, fileSize, huffmanEncoded, &encodedSize, frequencies);

        //replace variables with new data
        fileSize = encodedSize;
        free(fileData);
        fileData = huffmanEncoded;

        //send and free frequencies
        data_writeData(pdata->socket, frequencies, TREE_NODE_COUNT * sizeof(unsigned int));
        free(frequencies);
      }

      // if lzw then compress using lzw before sending
      if (strcmp(inputBuffer[0], "lzw") == 0) {
        // alloc atleast size of original file, and output will be in int
        int *lzwEncoded = calloc(fileSize, sizeof(int));
        if (lzwEncoded == NULL) {
          printf("[send][%s] Failed to allocate (probably too big) chunk of "
                 "memory. Skipping...\n",
                 peerUserName);
          free(fileData);
          continue;
        }
        int encodedSize;
        lzwEncode(fileData, fileSize, lzwEncoded, &encodedSize);


        //replace original variables with new
        fileSize = encodedSize;
        free(fileData);
        fileData = (char *) lzwEncoded;
      }

      // send file size
      data_writeData(pdata->socket, &fileSize, sizeof(int));
      // send file name
      data_writeData(pdata->socket, inputBuffer[i], INPUT_BUFFER_CELL_SIZE);
      // send file over socket
      printf("[send][%s] Sending file %s of size %d.\n", peerUserName,
             inputBuffer[i], fileSize);
      data_writeData(pdata->socket, fileData, fileSize);
      printf("[send][%s] Successfully sent file %s to peer.\n", peerUserName,
             inputBuffer[i]);

      free(fileData);
    }
  }

  if (inputBuffer != NULL) {
    destroyInputBuffer(&inputBuffer);
  }

  inputReaderData_decrementActiveThreads(pdata->inputReaderData);

  return NULL;
}

void data_readData(int socket, void *data, size_t dataSize) {
  size_t receivedBytesTotal = 0;

  while (receivedBytesTotal < dataSize) {
    size_t receivedBytes = read(socket, (unsigned char *) data + receivedBytesTotal, dataSize - receivedBytesTotal);
    receivedBytesTotal += receivedBytes;
  }
}

void *receiveData(void *data) {
  DATA *pdata = (DATA *) data;

  //read and save username
  char userName[USER_LENGTH + 1];
  userName[USER_LENGTH] = '\0';
  bzero(userName, USER_LENGTH);
  data_readData(pdata->socket, userName, USER_LENGTH);
  data_setPeerUserName(pdata, userName);
  printf("[receive][%s] Peer joined.\n", userName);

  while (!data_isStopped(pdata)) {
    // read compression method / or end of messaging
    char compressionMethod[INPUT_BUFFER_CELL_SIZE];
    data_readData(pdata->socket, compressionMethod, INPUT_BUFFER_CELL_SIZE);

    // if user sends quit, then stop
    if (strcmp(compressionMethod, "quit") == 0) {
      //if quit was initiated from this application and is just receiving response to quit
      if (data_isStopped(pdata)) {
        continue;
      }

      //also send quit to peer LISTENING thread
      char *quitMessage = "quit";
      data_writeData(pdata->socket, quitMessage, INPUT_BUFFER_CELL_SIZE);
      //print statement that connection has ended
      printf("[receive][%s] Peer ended connection... Enter any key or word to "
             "continue.\n", userName);
      data_stop(pdata);
      continue;
    }

    //do stuff if comression method is huffman
    unsigned int *frequencies;
    if (strcmp(compressionMethod, "huffman") == 0) {
      frequencies = calloc(TREE_NODE_COUNT, sizeof(unsigned int));
      data_readData(pdata->socket, frequencies, TREE_NODE_COUNT * sizeof(unsigned int));
    }

    // read file size
    int fileSize;
    data_readData(pdata->socket, &fileSize, sizeof(int));
    // read file name
    char fileName[INPUT_BUFFER_CELL_SIZE];
    data_readData(pdata->socket, fileName, INPUT_BUFFER_CELL_SIZE);
    // read file
    printf("[receive][%s] Incoming file %s with size %d.\n", userName,
           fileName, fileSize);
    char *fileData = calloc(fileSize, sizeof(char));
    data_readData(pdata->socket, fileData, fileSize);

    // decompress if compression was used
    // huffman
    if (strcmp(compressionMethod, "huffman") == 0) {
      char *huffmanDecoded;
      int decodedSize;
      huffmanDecode(fileData, fileSize, &huffmanDecoded, &decodedSize, frequencies);

      //replace variables with new data and free
      free(fileData);
      fileData = huffmanDecoded;
      fileSize = decodedSize;
      free(frequencies);
    }

    //lzw
    if (strcmp(compressionMethod, "lzw") == 0) {
      char *lzwDecoded;
      int decodedSize;
      lzwDecode((int *) fileData, fileSize, &lzwDecoded, &decodedSize);

      // replace variables with new data and free
      free(fileData);
      fileData = lzwDecoded;
      fileSize = decodedSize;
    }

    // save file
    bool result = writeFileData(fileName, fileData, fileSize);
    if (!result) {
      printf("[receive][%s] Could not write the file \"%s\". Skpping...\n",
             userName, fileName);
    } else {
      printf("[receive][%s] Successfuly written file \"%s\".\n", userName, fileName);
    }

    //free allocated memory
    free(fileData);
  }

  inputReaderData_decrementActiveThreads(pdata->inputReaderData);

  return NULL;
}
