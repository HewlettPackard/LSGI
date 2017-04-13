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
//============================================================================
// Name        : binaryConvertor.cpp
// Author      : tere
// Version     :
// Copyright   : Your copyright notice
// Description : Graph generator in binary format to speed up the reading of graps
// in LSGI engine.
//============================================================================


#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <stdint.h>
#include <sstream>

#define ITERATOR_INDEX int64_t

/*
 * Overview
 *
 *
 * This program includes functions to covert binary files from ALCHEMY WITH FACTORS or EDGE LIST WITH Factors format
 *
 *
 *
 * The binary file is written as follows
 *
 *
 * <number_Vertices><number_vars><number_of_unaryfactor><number_of_edgefactors>
 * <v1_from><v2_from> <unaryFactor_of_v1> <unaryFactor_of_v1>  <edFactor_1> <edFactor_2>  <edFactor_3>  <edFactor_R>
 * ...
 * <vn_from>..
 * */

/*
 * Write binary file of edge list with factors,
 * custom version for the wiki graph which is not in alchemy format
 *
 * */
bool writeBinaryGraphFromEdgeList(std::string &fileName){

	std::ifstream inGraphFile(fileName.c_str());
	std::ofstream outGraphFile;
	std::string line;
	ITERATOR_INDEX nVars  	 	= 2;
	ITERATOR_INDEX nVertices	= 0;
	ITERATOR_INDEX nEdges 		= 0;
	ITERATOR_INDEX variable1	= 0;
	ITERATOR_INDEX variable2	= 0;
	ITERATOR_INDEX nUFactors	= 2;
	ITERATOR_INDEX nEdgesFactors= 4;
	float unaryFactos[2] ={-0.69314,-0.69314};
	float edgeFactors[4] ={-0.67334,-0.71334,-1.38629,-0.28768};
	char *p=NULL;
	ITERATOR_INDEX iEdges = 0;
	std::string parName;
	int factorIdx=0;
	iEdges=0;



	if (inGraphFile.good()){

				std::stringstream parName;
				parName << fileName <<".bin";
				std::cout <<"\n---------------";
				std::cout <<"\nStarting converting the EDGE LIST file::" <<parName.str();
				outGraphFile.open(parName.str().c_str(), std::ios::out| std::ios::binary);

				if (outGraphFile.good()){

				/*Writing Headers of the file:*/
				/*Writing number of vertices:*/
				outGraphFile.write((char*)&nVertices,sizeof(ITERATOR_INDEX));
				/*Writing number of vars:*/
				outGraphFile.write((char*)&nVars,sizeof(ITERATOR_INDEX));

				/*Writing unary factor size*/
				outGraphFile.write((char*)&nUFactors,sizeof(ITERATOR_INDEX));

				/*write edge factor size*/
				outGraphFile.write((char*)&nEdgesFactors,sizeof(ITERATOR_INDEX));

				iEdges = 0;
				do{

					if( line.size()<3)
						continue; //empty line, then continue;
					//std::cout <<"\nline:"<<line;
					p =strtok(&line[0],"/");
					if (p != NULL){
						variable1 = atol (p);
						//std::cout <<"\t variable1:"<<variable1;
						// parse"/"
						p = strtok (NULL, "/");
						variable2 = atol (p);
						//std::cout <<"\t variable2:"<<variable2;

						//getting factors value
						factorIdx=0;
						p = strtok (NULL, " ");
						while ((p = strtok (NULL, " "))!=NULL){
							edgeFactors[factorIdx] =atof(p);
							//std::cout <<"\nfactor:"<<factorIdx<<"= "<<edgeFactors[factorIdx] <<" p="<<p;
							factorIdx++;
						}

						nEdges++;
						iEdges++;
					}

					/*write vertex pair*/
					outGraphFile.write((char*)&variable1,sizeof(ITERATOR_INDEX));
					outGraphFile.write((char*)&variable2,sizeof(ITERATOR_INDEX));

					/*write factors*/
					//TODO: unary factors should be redesign
					outGraphFile.write((char*)unaryFactos,nUFactors*sizeof(float));
					outGraphFile.write((char*)edgeFactors,nEdgesFactors*sizeof(float));


				}while(getline(inGraphFile, line));

				outGraphFile.close();
				std::cout <<"\nTotal edges written:"<<iEdges;
			}else{
				std::cout<<"\nError creating file:"<<fileName;
				return false;
			}
		inGraphFile.close();



	}else{
		std::cout<<"\nError reading file:"<<fileName;
		return false;
	}

	return true;
}

/*
 * Read the alchemy file and
 * write a binary format of the alchemy as edge list with factors
 *
 * The edge list is partitioned over multiple partitions
 * */
bool writeBinaryGraphFromAlchemyByParitions(std::string &fileName, int numberOfPartitions, ITERATOR_INDEX E){

	std::ifstream inGraphFile(fileName.c_str());
	std::ofstream outGraphFile;
	std::string line;
	ITERATOR_INDEX nVars  	 	= 2;
	ITERATOR_INDEX nVertices		= 0;
	ITERATOR_INDEX nEdges 		= 0;
	ITERATOR_INDEX variable1	= 0;
	ITERATOR_INDEX variable2	= 0;
	ITERATOR_INDEX nUFactors	= 2;
	ITERATOR_INDEX nEdgesFactors= 4;
	float unaryFactos[2] ={-0.69314,-0.69314};
	float edgeFactors[4] ={-0.67334,-0.71334,-1.38629,-0.28768};
	char *p=NULL;
	int iPartition        = 0;
	ITERATOR_INDEX iEdges = 0;
	std::string parName;
	int factorIdx=0;
	ITERATOR_INDEX partitionSize= E/numberOfPartitions;
	std::cout <<"Partition size="<<partitionSize;
	iEdges=0;



	if (inGraphFile.good()){

			//read variables
			while(getline(inGraphFile, line) && line.find("factors")==std::string::npos) {
				nVertices++;
				//std::cout <<"\nline:"<<line;
			}
			nVertices--;
			while(getline(inGraphFile, line) && (line.find(" / ")==std::string::npos)) {
			//	std::cout <<"\nline:"<<line;
			}


			for (iPartition =0 ; iPartition<numberOfPartitions; iPartition++){

				std::stringstream parName;
				parName << fileName <<".bin."<<numberOfPartitions<<"."<< iPartition;
				std::cout <<"\n---------------";
				std::cout <<"\nStarting converting the file::" <<parName.str();
				outGraphFile.open(parName.str().c_str(), std::ios::out| std::ios::binary);

				if (outGraphFile.good()){

				/*Writing Headers of the file:*/
				/*Writing number of vertices:*/
				outGraphFile.write((char*)&nVertices,sizeof(ITERATOR_INDEX));
				/*Writing number of vars:*/
				outGraphFile.write((char*)&nVars,sizeof(ITERATOR_INDEX));

				/*Writing unary factor size*/
				outGraphFile.write((char*)&nUFactors,sizeof(ITERATOR_INDEX));

				/*write edge factor size*/
				outGraphFile.write((char*)&nEdgesFactors,sizeof(ITERATOR_INDEX));

				iEdges = 0;
				do{

					if( line.size()<3)
						continue; //empty line, then continue;
					//std::cout <<"\nline:"<<line;
					p =strtok(&line[0],"/");
					if (p != NULL){
						variable1 = atol (p);
						//std::cout <<"\t variable1:"<<variable1;
						// parse"/"
						p = strtok (NULL, "/");
						variable2 = atol (p);
						//std::cout <<"\t variable2:"<<variable2;

						//getting factors value
						factorIdx=0;
						p = strtok (NULL, " ");
						while ((p = strtok (NULL, " "))!=NULL){
							edgeFactors[factorIdx] =atof(p);
							//std::cout <<"\nfactor:"<<factorIdx<<"= "<<edgeFactors[factorIdx] <<" p="<<p;
							factorIdx++;
						}

						nEdges++;
						iEdges++;
					}

					/*write vertex pair*/
					outGraphFile.write((char*)&variable1,sizeof(ITERATOR_INDEX));
					outGraphFile.write((char*)&variable2,sizeof(ITERATOR_INDEX));

					/*write factors*/
					//TODO: unary factors should be redesign
					outGraphFile.write((char*)unaryFactos,nUFactors*sizeof(float));
					outGraphFile.write((char*)edgeFactors,nEdgesFactors*sizeof(float));


				}while(getline(inGraphFile, line) && iEdges <partitionSize);

				outGraphFile.close();
				std::cout <<"Total edges written:"<<iEdges<<" from "<<nEdges;
			}else{
				std::cout<<"\nError creating file:"<<fileName;
				return false;
			}
		}
		inGraphFile.close();



	}else{
		std::cout<<"\nError reading file:"<<fileName;
		return false;
	}

	return true;
}


/*
 * Read the alchemy file and
 * write a binary format of the alchemy as edge list with factors
 * */
bool writeBinaryGraphFromAlchemy(std::string &fileName){

	std::ifstream inGraphFile(fileName.c_str());
	std::ofstream outGraphFile;
	std::string line;
	ITERATOR_INDEX nVars  	 	= 2;
	ITERATOR_INDEX nVertices		= 0;
	ITERATOR_INDEX nEdges 		= 0;
	ITERATOR_INDEX variable1	= 0;
	ITERATOR_INDEX variable2	= 0;
	ITERATOR_INDEX nUFactors	= 2;
	ITERATOR_INDEX nEdgesFactors= 4;
	float unaryFactos[2] ={-0.69314,-0.69314};
	float edgeFactors[4] ={-0.67334,-0.71334,-1.38629,-0.28768};
	char *p=NULL;
	ITERATOR_INDEX iEdges = 0;
	std::string parName;
	int factorIdx=0;

	iEdges=0;

	if (inGraphFile.good()){
			//read variables
			while(getline(inGraphFile, line) && line.find("factors")==std::string::npos) {
				nVertices++;
				//std::cout <<"\nline:"<<line;
			}
			nVertices--;
			while(getline(inGraphFile, line) && (line.find(" / ")==std::string::npos)) {
			//	std::cout <<"\nline:"<<line;
			}


				std::stringstream parName;
				parName << fileName <<".bin";
				std::cout <<"\n---------------";
				std::cout <<"\nStarting converting the ALCHEMY file::" <<parName.str();
				outGraphFile.open(parName.str().c_str(), std::ios::out| std::ios::binary);

				if (outGraphFile.good()){

				/*Writing Headers of the file:*/
				/*Writing number of vertices:*/
				outGraphFile.write((char*)&nVertices,sizeof(ITERATOR_INDEX));
				/*Writing number of vars:*/
				outGraphFile.write((char*)&nVars,sizeof(ITERATOR_INDEX));

				/*Writing unary factor size*/
				outGraphFile.write((char*)&nUFactors,sizeof(ITERATOR_INDEX));

				/*write edge factor size*/
				outGraphFile.write((char*)&nEdgesFactors,sizeof(ITERATOR_INDEX));

				iEdges = 0;
				do{

					if( line.size()<3)
						continue; //empty line, then continue;
					//std::cout <<"\nline:"<<line;
					p =strtok(&line[0],"/");
					if (p != NULL){
						variable1 = atol (p);
						//std::cout <<"\t variable1:"<<variable1;
						// parse"/"
						p = strtok (NULL, "/");
						variable2 = atol (p);
						//std::cout <<"\t variable2:"<<variable2;

						//getting factors value
						factorIdx=0;
						p = strtok (NULL, " ");
						while ((p = strtok (NULL, " "))!=NULL){
							edgeFactors[factorIdx] =atof(p);
							//std::cout <<"\nfactor:"<<factorIdx<<"= "<<edgeFactors[factorIdx] <<" p="<<p;
							factorIdx++;
						}

						nEdges++;
						iEdges++;
					}

					/*write vertex pair*/
					outGraphFile.write((char*)&variable1,sizeof(ITERATOR_INDEX));
					outGraphFile.write((char*)&variable2,sizeof(ITERATOR_INDEX));

					/*write factors*/
					//TODO: unary factors should be redesign
					outGraphFile.write((char*)unaryFactos,nUFactors*sizeof(float));
					outGraphFile.write((char*)edgeFactors,nEdgesFactors*sizeof(float));


				}while(getline(inGraphFile, line));

				outGraphFile.close();
				std::cout <<"\nTotal edges written:"<<iEdges;
				std::cout <<"\nTotal vertices read:"<<nVertices;
			}else{
				std::cout<<"\nError creating file:"<<fileName;
				return false;
			}
		inGraphFile.close();

	}else{
		std::cout<<"\nError reading file:"<<fileName;
		return false;
	}

	return true;
}

/* Read the binary graph
 * to veriy conrrectness
 * @PARAMS:
 * @RETURN:*/
bool readBinaryGraph(std::string &fileName){

	std::ifstream inFileGraph(fileName.c_str(),std::ios::in| std::ios::binary | std::ios::ate);

	ITERATOR_INDEX variable    = 0;
	ITERATOR_INDEX nVertices   = 0;
	ITERATOR_INDEX variable2   = 0;
	ITERATOR_INDEX iVarCounter = 0;
	ITERATOR_INDEX nEdges 	   = 0;
	ITERATOR_INDEX nVars	   = 0;
	ITERATOR_INDEX uFactorSize = 2;
	ITERATOR_INDEX eFactorSize = 0;

	ITERATOR_INDEX HEADERSIZE  = 4;
	int	   edgeElementsSize  = 4;


	float factors[6]={0.0,0.0,0.0,0.0,0.0,0.0};

	std::streampos fileSize;


	if (inFileGraph.good()){

		std::cout <<"\nStarting to read the file:..."<<fileName;
		/*read header*/

		fileSize = inFileGraph.tellg();
		inFileGraph.seekg(0,std::ios::beg);
		std::cout <<"\n file size is: "<<fileSize;

		inFileGraph.read((char*)&nVertices,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal vertices:"<<nVertices;

		inFileGraph.read((char*)&nVars,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal vars per edge:"<<nVars;

		inFileGraph.read((char*)&uFactorSize,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal ufactors:"<<uFactorSize;

		inFileGraph.read((char*)&eFactorSize,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal eFactorSize:"<<eFactorSize;


		fileSize= (fileSize-(HEADERSIZE*sizeof(ITERATOR_INDEX)));
		edgeElementsSize =(nVars*sizeof(ITERATOR_INDEX)+(eFactorSize + uFactorSize)*sizeof(float));
		nEdges =fileSize/ edgeElementsSize;

		std::cout <<"\nN edges computed:"<<nEdges;
	//	nEdges=2;
		while (iVarCounter < nEdges ){
			inFileGraph.read((char*)&variable,sizeof(ITERATOR_INDEX));
			std::cout<<"\nvar1:"<<variable;
			inFileGraph.read((char*)&variable2,sizeof(ITERATOR_INDEX));
			std::cout<<"\tvar2:"<<variable2;

			inFileGraph.read((char*)factors, (uFactorSize +eFactorSize)*sizeof(float));
			//inFileGraph.read((char*)edgeFactor, eFactorSize*sizeof(ITERATOR_INDEX));

			for (int m =0; m<(uFactorSize+eFactorSize); m++){
				std::cout <<"\tf_"<<m<<"="<<factors[m];
			}

			iVarCounter++;
		}
	}else{
		std::cout <<"Error open the file:"<<fileName;
		return false;
	}

	return true;

}
int main(int argc, char *argv[]){


	if (argc<3){
		std::cout <<"Usage error: <input alchemy file> <0=alchemy, 1=edgelist only>";
		return 1;
	}

	std::string inFileName=argv[1];
	int mode =atoi(argv[2]);

	switch(mode){
		case 0:writeBinaryGraphFromAlchemy(inFileName);
			break;
		case 1:writeBinaryGraphFromEdgeList(inFileName);
			break;
	}

	/*Code to verify if the write was corect*/
	/*
	std::string outFileName=(inFileName+".bin");
	readBinaryGraph(outFileName);
	std::cout <<"\n Done..";
	*/
	return 0;
}
