// Name: Anne Marie Heidebreicht
// NetID: alh1310
// Program 1: Create a UDP socket program that transfers a file from the client to the server
// Notes: Skeleton of this code was provided by Dr. Charan Gudla (Author of Skeleton Code: Maxwell Young)


// Server.cpp


// Headers
#include<iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <time.h>        // used for random number generation
#include <string.h> // using this to convert random port integer to string
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include <cstdlib>
#include <sstream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    cout << "Usage: ./server <n_port>" << endl;
  }


  /* Negotiation Stage (Handshake) */
  // Initialize variables
  struct sockaddr_in server;
  struct sockaddr_in client;
  int mysocket = 0;
  socklen_t clen = sizeof(client);
  char payload[512];
  int n_port = atoi(argv[1]); // <n_port> aka the Handshake Port

  // Create socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
  {
    cout << "Error in socket creation.\n";
  }
  
  // Socket Structure
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the host address
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    cout << "Error in binding.\n";
  }
  
  // Receive message from Client
  cout << "I'm waiting for handshake now.\n";
  if (recvfrom(mysocket, payload, 512, 0, (struct sockaddr *)&client, &clen) == -1)
  {
    cout << "Failed to receive.\n";     
  }
  cout << "Received Handshake: " << payload << endl;
  
  cout << "n_port: " << n_port << endl;

  // Random port generator reference: https://www.geeksforgeeks.org/rand-and-srand-in-ccpp/
  srand(time(0));
  int r_port = 1024 + (rand() % 65535); // Generate <r_port>

  char ack[512]; // char ack

  // string to char array reference: https://www.tutorialspoint.com/convert-string-to-char-array-in-cplusplus
  string r_portString = to_string(r_port); // Convert char to string 
  r_portString.copy(ack, r_portString.size() + 1);

  // Send message back to Client
  if (sendto(mysocket, ack, 64, 0, (struct sockaddr *)&client, clen) == -1)
  {
    cout << "Error in sendto function.\n";
  }
  cout << "Random Port: " << r_port << endl;
  close(mysocket); // Close the socket



  /* Transaction Stage (File Transfer) */
  // Create file to write and open it
  ofstream newFile;
  newFile.open("upload.txt"); // Create new file
  if (!newFile.is_open())
  {
    cout << "Error creating file.\n";
    newFile.close();
    return 0;
  }

  mysocket = 0; // reset socket variable

  // Create a second socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
  {
    cout << "Error in socket creation.\n";
  }
  
  // Second socket structure
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(r_port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the host address to the second socket
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    cout << "Error in binding.\n";
  }
  
  bool endFile = false;
  while (!endFile) // Until the EOF
  {
    // Acks per payload (4 payloads for the 4 bytes + 1 for EOF)
    ack[0] = '\0';
    ack[1] = '\0';
    ack[2] = '\0';
    ack[3] = '\0';
    ack[4] = '\0';
  
    // Receive message from Client
    if (recvfrom(mysocket, payload, 5, 0, (struct sockaddr *)&client, &clen) == -1)
    {
      cout << "Failed to receive.\n";     
    }

    // EOF check
    if (payload[4] == 26)
    {
      endFile = true;
    }

    for (int i = 0; i < 4; i++) // 4 chars
    {
      if (payload[i] == 26) // check EOF
      {
        endFile = true;
      }
      ack[i] = toupper(payload[i]); // Change from lowercase to uppercase and put in ack char array
      if (payload[i] != '\0') // To make sure not to get garbage chars
      {
        newFile << payload[i]; // Write shared lowercase text to upload.txt
      }
    }

    // Send message back to Client
    if (sendto(mysocket, ack, 5, 0, (struct sockaddr *)&client, clen) == -1)
    {
      cout << "Error in sendto function.\n";
    }
  }

    newFile.close(); // Close the file
    close(mysocket); // Close the socket

    return 0;
}
