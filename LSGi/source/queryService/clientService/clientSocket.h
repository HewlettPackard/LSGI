/**
“© Copyright 2017  Hewlett Packard Enterprise Development LP

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
*/

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define ITERATOR_INDEX    int64_t
#define VERTEX_VALUE_TYPE char
#define FLOAT_AGG_VALUE_TYPE float
#ifndef CLIENTSOCKET_H_
#define CLIENTSOCKET_H_

/*structure to manage main request type*/
struct queryRequest {
	int type;
	int value;
};

/*structure to manage vertex state results*/
struct vertexState{
	ITERATOR_INDEX       vid;
	FLOAT_AGG_VALUE_TYPE p_0;
	VERTEX_VALUE_TYPE    state;
	int		     iteration;
};

/*struct to manage vertex statistics for each workinger*/
struct vertexStats{
        ITERATOR_INDEX   nonconverge;
        ITERATOR_INDEX   iteration;
        ITERATOR_INDEX   time;
	ITERATOR_INDEX   totalVertices;
	ITERATOR_INDEX   state0;
};




/*Main query methods*/
void doQuery(int socketFd, int queryType);
void doQueryByVid(int socketFd);
void doQueryByThreshold(int socketFd);
void doQueryAll(int socketFd);
void doQueryByThresholdStats(int socketFd);
void error(const char *msg);
/*
 * Method to use TCP socket to connect to
 * the query service
 *
 */
int connectToServer(int portNo,std::string hostname, queryRequest &request){
	 int sockfd;
	 int portno;
	 int n;
	 struct sockaddr_in serv_addr;
	 struct hostent *server;


	  portno = portNo;
	  server = gethostbyname(hostname.c_str());

	 // std::cout <<"\n Connection to server started... on port" <<portNo;

	  sockfd = socket(AF_INET, SOCK_STREAM, 0);
	  if (sockfd < 0){
	    error("ERROR opening socket");
	  }


	  if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host\n");
	    exit(0);
	  }

	  bzero((char *) &serv_addr, sizeof(serv_addr));
	  serv_addr.sin_family = AF_INET;

	  bcopy((char *)server->h_addr,
	  (char *)&serv_addr.sin_addr.s_addr,
	  server->h_length);
	  serv_addr.sin_port = htons(portno);

	  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	    error("ERROR connecting");

	  //std::cout <<"\nRequest Value to search for: "<<request.value <<" \n";

	 switch(request.type) {
		case 1:
		 	std::cout <<"\n Query Type: [ Search by Vertex Id ]";
		        std::cout <<"\n vertex#:    [ " <<request.value<< " ]";
			break;
		case 3: 
		       std::cout <<"\n Query Type:  [ Search by Probability Threshold ]";;
		       std::cout <<"\n Probability: [ " <<request.value<< " ]";		
		       break;
                case 5:
                       std::cout <<"\n Query Type:  [ Search STATS  by Probability Threshold ]";;
                       std::cout <<"\n Probability: [ " <<request.value<< " ]";
                       break;


		default: std::cout << "Invalid  Id"; return 1;
	}
         n = write(sockfd,&request,sizeof(request));

	  if (n < 0)
	    error("ERROR writing to socket");

	  doQuery(sockfd,request.type);
	  close(sockfd);
	  return 0;
}

/*Main query driver
 * depending on the query type
 * */
void doQuery(int socketFd, int queryType){

	switch(queryType) {

	case 1: /*vid type*/
		doQueryByVid(socketFd);
		break;
	case 2: /**/
		break;
	case 3:
		doQueryByThreshold(socketFd);
		break;

	case 4:
		break;
	case 5: 
		doQueryByThresholdStats(socketFd);
		break;
	default:
		std::cout <<"\n Invalid Query Type\n";
		return;
	}

}

/*
 * search specific vertex id
 * */
void doQueryByVid(int socketFd){

	int n=0;
	vertexState state;
	vertexStats statsResponse;
	
	n = read(socketFd,&state,sizeof(state));
	if (n < 0)
	    error("ERROR reading from server socket");

	n = read(socketFd,&statsResponse,sizeof(statsResponse));
	if (n < 0)
            error("ERROR reading from server socket");

	  std::cout <<"\n\n Inference Results";
  	  std::cout <<"\n-----------------------------------------";
 	  std::cout <<"\nvertex#";
          std::cout <<"\tstate ";
          std::cout <<"\tprob_0 ";
          std::cout <<"\titeration#";
	 
	  std::cout <<"\n-----------------------------------------";
	  std::cout <<"\n   "<< state.vid;
	  std::cout <<"\t"<< state.state;
	  std::cout <<"\t"<< state.p_0;
	  std::cout <<"\t"<<state.iteration<<"\n";

 	 std::cout <<"\n-----------------------------------------";
	std::cout  <<"\nElapsed runtime:       \t" <<statsResponse.time;
//	std::cout  <<"\nIteration:"<<statsResponse.iteration;
	std::cout  <<"\nNon-converged vertices:\t"<<statsResponse.nonconverge;
	if(statsResponse.totalVertices>0)
		std::cout  <<"\nConvergence:\t"<<100.0-(statsResponse.nonconverge*100.0/statsResponse.totalVertices);
	std::cout <<"\nstate0="<<statsResponse.state0;
}


/*
 * Search a subset to be greater or equal to
 * a threshold
 * */
void doQueryByThreshold(int socketFd){

	vertexStats statsResponse;
	vertexState *vidResult  = NULL;
	int n=0;

	/*count values*/
	ITERATOR_INDEX counter = 0;
	ITERATOR_INDEX maxVal = 5;


	 n = read(socketFd,&statsResponse,sizeof(statsResponse));
        if (n < 0)
            error("ERROR reading from server socket");


	n = read(socketFd,&counter,sizeof(counter));
	if (n < 0)
		    error("ERROR reading from socket");

	if (counter ==0 ){ /*no elements found*/
		std::cout <<"\nNo elements found...\n";
		return ;
	}

	vidResult = (vertexState*)malloc(counter*sizeof(vertexState));

	n =read(socketFd,vidResult,counter*sizeof(vertexState));
	if (n < 0){
		error("ERROR reading from socket");
	}

	/*print first 5 */	
	  std::cout <<"\n\n Inference Results";
          std::cout <<"\n-----------------------------------------";
          std::cout <<"\nvertex#";
          std::cout <<"\tstate ";
          std::cout <<"\tprob_0 ";
          std::cout <<"\titeration#";
	 std::cout <<"\n-----------------------------------------";

	for (int i =0; i<std::min(maxVal,counter); i++){
		std::cout <<"\n   "<< (vidResult+i)->vid;
		std::cout <<"\t" <<(vidResult+i)->state;
		std::cout <<"\t"<< (vidResult+i)->p_0;
		std::cout <<"\t" <<(vidResult+i)->iteration;
	}

	std::cout  <<"\nElements found >= the threshold: [ "<<counter << " ]";
        std::cout  <<"\nListed First:                    [ 5 ]\n\n";	

	std::cout <<"\n-----------------------------------------";
        std::cout  <<"\nElapsed runtime(sec):            [ " <<statsResponse.time<< " ]";
        std::cout  <<"\nMax Iteration:                   [ " <<statsResponse.iteration << " ]";


	 std::cout  <<"\nNon-converged vertices:\t"<<statsResponse.nonconverge;
        if(statsResponse.totalVertices>0)
                std::cout  <<"\nConvergence:\t"<<100.0-(statsResponse.nonconverge*100.0/statsResponse.totalVertices);
 std::cout <<"\nstate0="<<statsResponse.state0;	


	if (vidResult != NULL ){
		free(vidResult);
	}
}


/*
 * Search a subset to be greater or equal to
 * a threshold
 * */
void doQueryByThresholdStats(int socketFd){

        vertexStats statsResponse;
        int n=0;

         n = read(socketFd,&statsResponse,sizeof(statsResponse));
        if (n < 0){
            error("ERROR reading from server socket");
	   return;
	}


	std::cout  <<"\nElements found >= the threshold: [ "<<statsResponse.state0 << " ]";
        std::cout <<"\n-----------------------------------------";
        std::cout  <<"\nElapsed runtime(sec):            [ " <<statsResponse.time<< " ]";
        std::cout  <<"\nMax Iteration:                   [ " <<statsResponse.iteration << " ]";


         std::cout  <<"\nNon-converged vertices:\t"<<statsResponse.nonconverge;
        if(statsResponse.totalVertices>0)
                std::cout  <<"\nConvergence:\t"<<100.0-(statsResponse.nonconverge*100.0/statsResponse.totalVertices);
 	std::cout <<"\nstate0="<<statsResponse.state0;

}

/*Print error msg*/
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

#endif /*CLIENTSOCKET_H_*/
