#include "definitions.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char *argv[]) {
  if (argc < 4) {
    printError("Client has to be started with these parameters: address port username.");
  }

  //ziskanie adresy a portu servera <netdb.h>
  struct hostent *server = gethostbyname(argv[1]);
  if (server == NULL) {
    printError("Server does not exist.");
  }
  int port = atoi(argv[2]);
	if (port <= 0) {
		printError("Port has to be number larger than 0.");
	}
  char *userName = argv[3];

  //vytvorenie socketu <sys/socket.h>
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    printError("Error - socket.");
  }

  //definovanie adresy servera <arpa/inet.h>
  struct sockaddr_in serverAddress;
  bzero((char *)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
  serverAddress.sin_port = htons(port);

  if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    printError("Error - connect.");
  }

  // create data for thread that reads data and signals them to other threads
  IR_DATA *inputReaderData = calloc(1, sizeof(IR_DATA));
  inputReaderData_init(inputReaderData, 2);

  // create input reader thread
  pthread_t inputReaderThread;
  pthread_create(&inputReaderThread, NULL, signalUserInput,
                 (void *)inputReaderData);

  // inicializacia dat zdielanych medzi vlaknami
  DATA data;
	data_init(&data, inputReaderData, userName, sock);

	//vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
  pthread_t thread;
  pthread_create(&thread, NULL, sendData, (void *)&data);

	//v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
	receiveData((void *)&data);

	//pockame na skoncenie zapisovacieho vlakna <pthread.h>
	pthread_join(thread, NULL);

  //wait for signaling thread to end
  pthread_join(inputReaderThread, NULL);

	data_destroy(&data);
  inputReaderData_destroy(inputReaderData);

  //uzavretie socketu <unistd.h>
  close(sock);

  return (EXIT_SUCCESS);
}
