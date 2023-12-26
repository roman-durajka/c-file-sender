#include "definitions.h"
#include "file.h"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>


int main(int argc, char* argv[]) {
  if (argc < 4) {
    printError("Sever has to be started with these paremeters: port username number_of_clients.");
  }
  int port = atoi(argv[1]);
	if (port <= 0) {
		printError("Port has to be number larger than 0.");
	}

  int numberOfClients = atoi(argv[3]);
  if (numberOfClients < 1) {
    printf("There has to be at least 1 client.");
  }

  char *userName = argv[2];

  //vytvorenie TCP socketu <sys/socket.h>
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    printError("Error - socket.");
  }

  //definovanie adresy servera <arpa/inet.h>
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;         //internetove sockety
  serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
  serverAddress.sin_port = htons(port);       //nastavenie portu

  //prepojenie adresy servera so socketom <sys/socket.h>
  if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    printError("Error - bind.");
  }

  //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
  listen(serverSocket, numberOfClients);

  //allocate memory for threads
  pthread_t threads[numberOfClients * 2];

  //allocate memory to store sockets
  int clientSockets[numberOfClients];

  //create array to store all data
  DATA * savedData[numberOfClients];

  //create data for thread that reads data and signals them to other threads
  IR_DATA inputReaderData;
  inputReaderData_init(&inputReaderData, numberOfClients * 2);

  //create input reader thread
  pthread_t inputReaderThread;
  pthread_create(&inputReaderThread, NULL, signalUserInput, (void *)&inputReaderData);

  printf("Waiting for connections...\n");
  for (int i = 0; i < numberOfClients * 2; i += 2) {
    // server caka na pripojenie klienta <sys/socket.h>
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress,
                              &clientAddressLength);

    if (clientSocket < 0) {
      printf("Error accepting socket. Skpping...\n");
      continue;
    }

    clientSockets[i/2] = clientSocket;

    // inicializacia dat zdielanych medzi vlaknami
    DATA * data = calloc(1, sizeof(DATA));
    data_init(data, &inputReaderData, userName, clientSocket);
    savedData[i/2] = data;

    // vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_create(&threads[i], NULL, sendData, (void *)data);
    // vytvorenie vlakna pre citanie dat zo socketu <pthread.h>
    pthread_create(&threads[i+1], NULL, receiveData, (void *)data);
  }

  // uzavretie pasivneho socketu <unistd.h>
  close(serverSocket);


	//pockame na skoncenie vlakien
  pthread_join(inputReaderThread, NULL);
  for (int i = 0; i < numberOfClients * 2; i++) {
    pthread_join(threads[i], NULL);
  }

  //uzavretie socketu klienta <unistd.h>
  for (int i = 0; i < numberOfClients; i++) {
    close(clientSockets[i]);
  }

  //destroy input reader data
  inputReaderData_destroy(&inputReaderData);

  //destroy all initialized data
  for (int i = 0; i < numberOfClients; i++) {
    data_destroy(savedData[i]);
    free(savedData[i]);
  }

  return (EXIT_SUCCESS);
}
