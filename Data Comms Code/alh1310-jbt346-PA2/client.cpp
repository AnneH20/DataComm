// Name: Anne Marie Heidebreicht
// NetID: alh1310

// Name: John Tweedy
// NetID: jbt346
// Program 2: Implement the stop-and-wait protocol (We will call ARQ)

// Client.cpp
// Used the client.cpp code from the Emulator example as a skeleton (Author: Maxwell Young)

// Sources:
// https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
// https://linux.die.net/man/3/setsockopt
// https://beej.us/guide/bgnet/

#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h> 
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include <string.h>
#include <unistd.h>
#include "packet.h" // include packet class
#include <math.h>
#include "client.h" // chose not to use for this implementation but still included
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[]) {

	// declare and setup server
	struct hostent *em_host;            // pointer to a structure of type hostent
	em_host = gethostbyname(argv[1]);   // host name for emulator
	if(em_host == NULL){                // failed to obtain server's name
		cout << "Failed to obtain server.\n";
		exit(EXIT_FAILURE);
	}
 
  // ******************************************************************
  // ******************************************************************

  // client sets up datagram socket for sending
  int CESocket = socket(AF_INET, SOCK_DGRAM, 0);  
  if(CESocket < 0){
  	cout << "Error: failed to open datagram socket.\n";
  }

  // set up the sockaddr_in structure for sending
  struct sockaddr_in CE; 
  socklen_t CE_length = sizeof(CE);
  bzero(&CE, sizeof(CE)); 
  CE.sin_family = AF_INET;	
  bcopy((char *)em_host->h_addr, (char*)&CE.sin_addr.s_addr, em_host->h_length);  // both using localhost so this is fine
  char * end;
  int em_rec_port = strtol(argv[2], &end, 10);  // get emulator's receiving port and convert to int
  CE.sin_port = htons(em_rec_port);             // set to emulator's receiving port

  // ******************************************************************
  // ******************************************************************

  // client sets up datagram socket for receiving
  int ECSocket = socket(AF_INET, SOCK_DGRAM, 0);  
  if(ECSocket < 0){
  	cout << "Error: failed to open datagram socket.\n";
  }

  // set up the sockaddr_in structure for receiving
  struct sockaddr_in EC; 
  socklen_t EC_length = sizeof(EC);
  bzero(&EC, sizeof(EC)); 
  EC.sin_family = AF_INET;	
  EC.sin_addr.s_addr = htonl(INADDR_ANY);	
  char * end2;
  int cl_rec_port = strtol(argv[3], &end2, 10);  // client's receiving port and convert to int
  EC.sin_port = htons(cl_rec_port);             // set to emulator's receiving port

  // do the binding
  if (bind(ECSocket, (struct sockaddr *)&EC, EC_length) == -1){
  		cout << "Error in binding.\n";
  } 

    // 2 second timer for dropped packets
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(ECSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror(0);
        close(ECSocket);
        exit(1);
    }

  // ******************************************************************
  // ****************************************************************** 

    // open text file
    ifstream textFile;
    textFile.open(argv[4]);

    // define log files
    ofstream slog;
    ofstream alog;
    slog.open("clientseqnum.log", ofstream::trunc);
    alog.open("clientack.log", ofstream::trunc);

    if (!textFile.is_open()) {
        cout << "Error opening file.\nClient Terminated." << endl;
        close(ECSocket);
        return -1;
    }

    char byte = 0; // storage to hold each character from file
    int index = 0; // number assigned to characters
    char payload[30]; // payload with max length of 30
    int packetNumber = 0; // seqnum for each packet
    int seqnum;
    char serialized[40]; // ACK packet from server
    memset((char *) &payload, 0, sizeof(payload));
    memset((char *) &serialized, 0, sizeof(serialized));
    
    while (textFile.get(byte)) {
        payload[index] = byte;
        index += 1; 

        // full packet has length of 30
        if (index == 30){
            
            int type = 1; // set type = 1
            if (packetNumber % 2 == 0){
                seqnum = 0;
             } 
            else {
                seqnum = 1;
            }
            int length = 30; // set length = 30
            char *data = payload; // set data = whatever is read

            // create packet
            packet pack(type, seqnum, length, data);
            char spacket[40];

            // serialize the packet to be sent
            memset((char *) &spacket, 0, sizeof(spacket));
            pack.serialize(spacket);

            // send data packet to server and receive ACK packet from server
            if (sendto(CESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length) < 0){
                cout << "Failed to send payload.\n";
			    return 0;
            }
            slog << seqnum << "\n";
            pack.printContents();

            // wait for ACK packet, resends if 2 second timer expires
            while (recvfrom(ECSocket, serialized, 40, MSG_CONFIRM, (struct sockaddr*) &EC, &EC_length) < 0){

                cout << "Time exceeded 2 seconds. Resending previous packet.\n" << endl;
                sendto(CESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length);
                slog << seqnum << "\n";

            }

            // get seqnum from server
            pack.deserialize(serialized);
            alog << pack.getSeqNum() << "\n";
            cout << "Received packet and deserialized to obtain the following: " << endl;
            pack.printContents();

            memset((char *) &payload, 0, sizeof(payload)); // clear out data packet
            memset((char *) &serialized, 0, sizeof(serialized)); // clear out ACK packet

            packetNumber++; // increase sequence number
            index = 0; // reset payload length

            // if EOF happens
            if (textFile.peek() == EOF) {
                type = 3; // set type = 3
                if (packetNumber % 2 == 0){
                    seqnum = 0;
                }
                else {
                    seqnum = 1;
                }
                length = 0; // set length = 0
                data = NULL; // set data = NULL
    
                // set up EOT packet
                packet EOT(type, seqnum, length, data);
                char sEOT[40];

                // serialize the packet to be sent
                memset((char *) &sEOT, 0, sizeof(sEOT));
                EOT.serialize(sEOT);

                // send and receive EOT packet
                if (sendto(CESocket, sEOT, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length) < 0){
                    cout << "Failed to send payload.\n";
			        return 0;
                }
                slog << seqnum << "\n";
                EOT.printContents();

                // wait for EOT packet from server, resends if 2 second timer expires
                while (recvfrom(ECSocket, serialized, 40, MSG_CONFIRM, (struct sockaddr*) &EC, &EC_length) < 0){

                    cout << "Time exceeded 2 seconds. Resending previous packet.\n" << endl; // supposed to assume EOT packet will never be dropped so can disregard
                    sendto(CESocket, sEOT, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length);
                    slog << seqnum << "\n";

                }

                // get seqnum from EOT packet
                EOT.deserialize(serialized);
                alog << EOT.getSeqNum() << "\n";
                cout << "Received packet and deserialized to obtain the following: " << endl;
                EOT.printContents();
            }

        } 
        // EOF happens when length is not 30
        else if (textFile.peek() == EOF){

            int type = 1; // set type = 1
            if (packetNumber % 2 == 0){
                seqnum = 0;
            }
            else {
                seqnum = 1;
            }
            int length = index; // set length = what is left
            char *data = payload; // set data to whatever is read

            // create packet
            packet pack(type, seqnum, length, data);
            char spacket[40];

            // serialize the packet to be sent
            memset((char *) &spacket, 0, sizeof(spacket));
            pack.serialize(spacket);
            pack.printContents();

            // send data packet to server and receive ACK packet from server
            if (sendto(CESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length) < 0){
                cout << "Failed to send payload.\n";
			    return 0;
            }
            slog << seqnum << "\n";

            // wait for ACK packet, resends if 2 second timer expires
            while (recvfrom(ECSocket, serialized, 40, MSG_CONFIRM, (struct sockaddr*) &EC, &EC_length) < 0){

                cout << "Time exceeded 2 seconds. Resending previous packet.\n" << endl;
                sendto(CESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length);
                slog << seqnum << "\n";

            }

            // get seqnum from ACK packet
            pack.deserialize(serialized);
            alog << pack.getSeqNum() << "\n";
            cout << "Received packet and deserialized to obtain the following: " << endl;
            pack.printContents();
            
            packetNumber++; // increase sequence number
            memset((char *) &serialized, 0, sizeof(serialized)); // clear ACK packet

            // EOT stuff
            type = 3; // set type = 3
            if (packetNumber % 2 == 0){
                seqnum = 0;
            }
            else {
                seqnum = 1;
            }
            length = 0; // set length = 0
            data = NULL; // set data = NULL

            // set up EOT packet
            packet EOT(type, seqnum, length, data);
            char sEOT[40];

            // serialize the packet to be sent
            memset((char *) &sEOT, 0, sizeof(sEOT));
            EOT.serialize(sEOT);

            // send EOT
            if (sendto(CESocket, sEOT, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length) < 0){
                cout << "Failed to send payload.\n";
			    return 0;
            }
            slog << seqnum << "\n";
            EOT.printContents();

            // wait for EOT packet from server, resends if 2 second timer expires
            while (recvfrom(ECSocket, serialized, 40, MSG_CONFIRM, (struct sockaddr*) &EC, &EC_length) < 0){

                cout << "Time exceeded 2 seconds. Resending previous packet.\n" << endl; // supposed to assume EOT packet will never be dropped so can disregard
                sendto(CESocket, sEOT, 40, MSG_CONFIRM, (struct sockaddr*) &CE, CE_length);
                slog << seqnum << "\n";

            }

            // get seqnum from EOT packet
            EOT.deserialize(serialized);
            alog << EOT.getSeqNum() << "\n";
            cout << "Received packet and deserialized to obtain the following: " << endl;
            EOT.printContents();
        }
    }

    cout << "\n";
    
    // close all the things
    close(ECSocket);
    close(CESocket);
    textFile.close();
    alog.close();
    slog.close();
    return 0;
}