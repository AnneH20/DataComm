// Name: Anne Marie Heidebreicht
// NetID: alh1310

// Name: John Tweedy
// NetID: jbt346
// Program 2: Implement the stop-and-wait protocol (We will call ARQ)

// Server.cpp
// Used the server.cpp code from the Emulator example as a skeleton (Author: Maxwell Young)

// Sources:
// https://stackoverflow.com/questions/13547721/udp-socket-set-timeout
// https://linux.die.net/man/3/setsockopt
// https://beej.us/guide/bgnet/
// https://cplusplus.com/reference/fstream/ofstream/open/

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
#include <time.h>
#include <stdio.h>

using namespace std;

int main(int argc, char *argv[]) {

	// sets up datagram socket for receiving from emulator
	int ESSocket = socket(AF_INET, SOCK_DGRAM, 0);  
	if(ESSocket < 0){
		cout << "Error: failed to open datagram socket.\n";
	}

	// set up the sockaddr_in structure for receiving
	struct sockaddr_in ES; 
	socklen_t ES_length = sizeof(ES);
	bzero(&ES, sizeof(ES)); 
	ES.sin_family = AF_INET;	
	ES.sin_addr.s_addr = htonl(INADDR_ANY);	
	char * end;
	int sr_rec_port = strtol(argv[2], &end, 10);  // server's receiving port and convert to int
	ES.sin_port = htons(sr_rec_port);             // set to emulator's receiving port
		
	// do the binding
	if (bind(ESSocket, (struct sockaddr *)&ES, ES_length) == -1)
		cout << "Error in binding.\n";		
		
	// ******************************************************************
	// ******************************************************************
	
	// declare and setup server
	struct hostent *em_host;            // pointer to a structure of type hostent
	em_host = gethostbyname(argv[1]);   // host name for emulator
	
	if(em_host == NULL){                // failed to obtain server's name
		cout << "Failed to obtain server.\n";
		exit(EXIT_FAILURE);
	}

	int SESocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(SESocket < 0){
		cout << "Error in trying to open datagram socket.\n";
		exit(EXIT_FAILURE);
	}
		
	// setup sockaddr_in struct  
	struct sockaddr_in SE;	
    socklen_t SE_length = sizeof(SE);
	memset((char *) &SE, 0, sizeof(SE));
	SE.sin_family = AF_INET;
	bcopy((char *)em_host->h_addr, (char*)&SE.sin_addr.s_addr, em_host->h_length);
	int em_rec_port = strtol(argv[3], &end, 10);
	SE.sin_port = htons(em_rec_port);

  // ******************************************************************
  // ****************************************************************** 

    ofstream outputText;
    outputText.open(argv[4], ofstream::trunc); // using trunc to overwrite (source: https://cplusplus.com/reference/fstream/ofstream/open/)
    ofstream arlog;
    arlog.open("arrival.log", ofstream::trunc);

    if (!outputText.is_open()) {
        cout << "Error opening output file.\n";
        return -1;
    }

    char serialized[40]; // recieving packet
    bool eof = false; // EOF test
    int packetNum = 0;
    int expe_seqnum = 0;
    int prev_seqnum = 0;

    // Receiving data packets from client, sending back ACK packets, and recording to arrival.log and output.txt
    while (true) {
        if (packetNum % 2 == 0){
            expe_seqnum = 0;
        }
        else {
            expe_seqnum = 1;
        }
        if ((packetNum - 1) % 2 == 0){
            prev_seqnum = 0;
        }
        else {
            prev_seqnum = 1;
        }

        memset((char *) &serialized, 0, sizeof(serialized)); // clear data packet

        // deserialize received packet
        char payload[40];
        memset((char *) &payload, 0, sizeof(payload));
        char *instantiation = payload; // pointer to payload in memory
        packet rcvdPacket(0, 0, 0, instantiation);

        if (recvfrom(ESSocket, serialized, 40, MSG_CONFIRM, (struct sockaddr*)&ES, &ES_length) == -1){
            cout << "Failed to receive.\n";
        }

        cout << "Received packet and deserialized to obtain the following: " << endl;
        rcvdPacket.deserialize(serialized);
        rcvdPacket.printContents();

        // create ACK packets
        int type = 0;
        int seqnum = rcvdPacket.getSeqNum();
        int length = 0;
        char *data = NULL;
        char spacket[40];
        memset((char *) &spacket, 0, sizeof(spacket));

        arlog << seqnum << "\n";

        // if the expected seqnum matches, send ACK packet
        if (rcvdPacket.getSeqNum() == expe_seqnum && rcvdPacket.getType() == 1){

            packet myAckPacket(type, seqnum, length, data);
            myAckPacket.serialize(spacket);

            if (sendto(SESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*)&SE, SE_length) < 0){
                cout << "Failed to send payload.\n";
                return 0;
            }

            packetNum++;

            int data_length = rcvdPacket.getLength();
            char *data = rcvdPacket.getData(); 

            for (int i = 0; i < data_length; i++){
                outputText << data[i];
            }
        }
        // if we get EOT packet from client, send own EOT packet to client
        else if (rcvdPacket.getSeqNum() == expe_seqnum && rcvdPacket.getType() == 3){

            eof = true;
            type = 2;

            packet EOT(type, seqnum, length, data);
            EOT.serialize(spacket);

            if (sendto(SESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*)&SE, SE_length) < 0){
                cout << "Failed to send payload.\n";
                return 0;
            }
        }
        // if the expected seqnum isn't the seqnum, get previous seqnum and send ACK packet        
        else if (rcvdPacket.getSeqNum() != expe_seqnum){
            
            seqnum = prev_seqnum;

            packet myAckPacket2(type, seqnum, length, data);
            myAckPacket2.serialize(spacket);

            if (sendto(SESocket, spacket, 40, MSG_CONFIRM, (struct sockaddr*)&SE, SE_length) < 0){
                cout << "Failed to send payload.\n";
                return 0;
            }

        }

        if (eof){
            break; // move on
        }
    }

    // close all the things
    outputText.close();
    arlog.close();
    close(ESSocket);
    close(SESocket);
    return 0;
}
