// Name: Anne Marie Heidebreicht
// NetID: alh1310
// Program 1: Create a UDP socket program that transfers a file from the client to the server
// Notes: Skeleton of this code was provided by Dr. Charan Gudla (Author of Skeleton Code: Maxwell Young)

// Client.cpp

// Headers
#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include <string.h>
#include <unistd.h>

#include <cstdlib>
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    cout << "Usage: ./client <server_address> <n_port> <filename>" << endl;
    exit(0);
  }
  // Initialize variables
  struct hostent *s; 
  s = gethostbyname(argv[1]); // Server address
  int n_port = atoi(argv[2]); // <n_port> aka the Handshake Port
  ifstream inputFile; // File to send to Server
  inputFile.open(argv[3]);
  if (!inputFile.is_open())
  {
    cout << "Error opening file\n";
    inputFile.close();
    return 0;
  }


  /* Negotiation Stage (Handshake) */
  struct sockaddr_in server;
  int mysocket = 0;
  socklen_t slen = sizeof(server);
  char payload[512]="ABC"; // Handshake
  
  // Creating socket
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    cout << "Error in creating socket.\n";
  }

  // Socket structure
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port); // Set port as n_port
  bcopy((char *)s->h_addr, 
	(char *)&server.sin_addr.s_addr,
	s->h_length);

  // Sending data to Server
  cout << "Initiating handshake with server. Handshake: " << payload << endl;
  if (sendto(mysocket, payload, 8, 0, (struct sockaddr *)&server, slen) == -1)
  {
    cout << "Error in sendto function.\n";
  }

  cout << "n_port: " << n_port << endl;
  
  char  ack[512];
  // Receiving response from Server
  recvfrom(mysocket, ack, 512, 0, (struct sockaddr *)&server, &slen); 
  cout << "Random Port from Server: " << ack << endl;
  int r_port = atoi(ack); // Convert char to int to get random port number from Sever
  close(mysocket); // Close the socket



  /* Transaction Stage (File Transfer) */
  mysocket = 0; // reset socket variable

  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  {
    cout << "Error in creating socket.\n";
  }

  // Socket structure
  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(r_port); // Set port as r_port
  bcopy((char *)s->h_addr, 
	(char *)&server.sin_addr.s_addr,
	s->h_length);
 
  bool endFile = false;  
  char c;
  inputFile.get(c);

  while (!endFile) // Until the EOF
  {
    // Create payloads (4 payloads for the 4 bytes + 1 for EOF)
    payload[0] = '\0';
    payload[1] = '\0';
    payload[2] = '\0';
    payload[3] = '\0';
    payload[4] = '\0';

    for (int i = 0; i < 4 && !endFile; i++)
    {
      // Get the char
      payload[i] = c;
      inputFile.get(c); 

      if (inputFile.eof()) // If it is the end of the file
      {
        payload[i] = c; // Get the char if there's extra
        payload[4] = 26; // ASCII for EOF
        payload[5] = i + 1;
        endFile = true; // Official EOF
        continue;
      }
    }

    // Send payloads to the Server
    if (sendto(mysocket, payload, 5, 0, (struct sockaddr *)&server, slen) == -1)
    {
      cout << "Error in sendto function.\n";
    }

    // Receiving response from Server
    recvfrom(mysocket, ack, 5, 0, (struct sockaddr *)&server, &slen); 
    cout << ack << endl;
  }

    inputFile.close(); // Close the file
    close(mysocket); // Close the socket   
    
    return 0;
}
