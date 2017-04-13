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

/**
 * \brief Read file header:
 * It fills the information of the graph that
 * is going to be read in  GraphFileMemoryMap var.
 * The binary file has the following 4 variables as a header:
 * #number of Vertices (long)
 * #number of variables or states per vertice (long)
 * #number of unary factors in bytes (long)
 * #number of edge factors in bytes (long
 * Based on this information,the method computes remaining information
 * like number of edges in the file, total size of the factors per edge.
 * */

#include "customMalloc.h"
#include "inMemoryGraph.h"
#include "inferenceConfig.h"
#include "binaryGraphReader.h"

/**var initialization*/
GraphFileMemoryMap *graphFileMap=NULL;


/*****************************************************************
 * BinaryGraph Reader Methods
 *
 *****************************************************************/

void readGraphHeaders(GraphFileMemoryMap &graphFilePtrs){

	std::ifstream inFileGraph(graphFilePtrs.fileName,std::ios::in| std::ios::binary| std::ios::ate);

	if(inFileGraph.good()){


		std::streampos fileSize = inFileGraph.tellg();

		inFileGraph.seekg(0,std::ios::beg);
		std::cout <<"\n file size is: "<<fileSize <<"  name:"<<graphFilePtrs.fileName;
		inFileGraph.read((char*)&graphFilePtrs.num_of_nodes,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal vertices:"<<graphFilePtrs.num_of_nodes;

		inFileGraph.read((char*)&graphFilePtrs.numVars,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal vars per edge:"<<graphFilePtrs.numVars;

		inFileGraph.read((char*)&graphFilePtrs.uFactorSize,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal ufactors:"<<graphFilePtrs.uFactorSize;

		inFileGraph.read((char*)&graphFilePtrs.eFactorSize,sizeof(ITERATOR_INDEX));
		std::cout <<"\nTotal eFactorSize:"<<graphFilePtrs.eFactorSize;

		graphFilePtrs.factorBlockSize =(graphFilePtrs.uFactorSize +graphFilePtrs.eFactorSize)*sizeof(float);
		std::cout <<"\nfactorBlockSize="<<graphFilePtrs.factorBlockSize;
		graphFilePtrs.headerSize = HEADERELEMENTS*sizeof(ITERATOR_INDEX);
		graphFilePtrs.num_of_edges =(fileSize -graphFilePtrs.headerSize) / ((EDGESIZE*sizeof(ITERATOR_INDEX)+graphFilePtrs.factorBlockSize));
		std::cout << "\nNumber of edges:"<<graphFilePtrs.num_of_edges;

		inFileGraph.close();
	}else{
				std::cout <<"\nError reading the file "<<graphFilePtrs.fileName;
	}

}


/**
 *\brief get number of vertices
 *Return the number of vertices in the graph file
 * This method was used to read the number of vertices, but now it is not necessary,
 * it only retuns the value return in the structure.
 *
 */
int getNumberOfVertices(GraphFileMemoryMap &graphFilePtrs){


	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);

	//readGraphHeaders(graphFilePtrs);

	#ifdef DEBUG_BREAKDOWN_IME_ON
	struct timeval tv;
	double loadTime = 0;
	gettimeofday(&tv, NULL);
	loadTime = (tv.tv_sec - start_tv.tv_sec) +
	 			(tv.tv_usec - start_tv.tv_usec) / 1000000.0;
	std::cout <<"\n **TIME numberOfVertexCount: "<<loadTime;
	#endif
	return graphFilePtrs.num_of_nodes;
}


/**
 * \brief get number of vertices
 * Read number of vertices from a file.
 * This is done in order to skip the variables from the alchemy
 * and use only graph edges + factors.
 * The method assumes a ".var" file
 * which will contain the number of vertices
 * this method is obsolete
 * */
ITERATOR_INDEX getVerticesFromFile(GraphFileMemoryMap &graphFilePtrs, std::string &fileName){

	ITERATOR_INDEX numVertices=0;
	std::string line;
	std::ifstream inFile((fileName+".var").c_str());


	if (inFile.good()){
		std::cout <<"\n Getting vertices count from a file.";
		getline(inFile, line);
		numVertices = atol(line.c_str());
		graphFilePtrs.num_of_nodes    = numVertices;
	}else{
		std::cout <<"\n getting vertices count alchemy file.";
		numVertices =getNumberOfVertices(graphFilePtrs);
	}
	return numVertices;
}

/**\brief count number of edges
 * Read the binary file and count neighbors for the each vertex
 * accumulate the count by vertex id in the array factor_count_arr
 * The counter keeps the number in terms of elements
 * by pair of connected vertices.
 * it assumes that for each edge, there is a defined number of elements
 * store on the topologySizeByVertex.
 * In our case the number of elements by vertex is 5: four factors+vertexid
 * Return : 1 or 0 success
 */
int getEdgeNeigbornsCounter(GraphFileMemoryMap &graphFilePtrs, int* factor_count_arr, int num_nodes){

	std::ifstream inFileGraph(graphFilePtrs.fileName,std::ios::in| std::ios::binary);

	ITERATOR_INDEX variable    = 0;
	ITERATOR_INDEX variable2   = 0;
	ITERATOR_INDEX iEdgeCounter = 0;
	float factors[6]={0.0,0.0,0.0,0.0,0.0,0.0};


	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);



	std::cout <<"\n Parsing neighbors counters... n="<<num_nodes<<" - "<<factor_count_arr[0]<<"\t  graph vars:";
	std::cout <<graphFilePtrs.numVars;

	if (inFileGraph.good()){

		std::cout <<"File is open";

		inFileGraph.seekg(graphFilePtrs.headerSize,std::ios::beg);

		while (iEdgeCounter < graphFilePtrs.num_of_edges ){
			inFileGraph.read((char*)&variable,sizeof(ITERATOR_INDEX));
			//std::cout<<"\nvar1:"<<variable;
			inFileGraph.read((char*)&variable2,sizeof(ITERATOR_INDEX));
			//std::cout<<"\tvar2:"<<variable2;

			inFileGraph.read((char*)factors,(graphFilePtrs.factorBlockSize));
			factor_count_arr[variable]  += topologySizeByVertex;  //double for the unary factor + edge factor
			factor_count_arr[variable2] += topologySizeByVertex;  //double for the unary factor + edge factor

			iEdgeCounter++;
			}
	}else{
		std::cout <<"Error open the file:XXX$$$"<<graphFilePtrs.fileName<<"--\n";
		return false;
	}

		std::cout <<"\n+Total Edges read:"<<iEdgeCounter;

	#ifdef DEBUG_BREAKDOWN_IME_ON
		struct timeval tv;
		double loadTime = 0;
		gettimeofday(&tv, NULL);
		loadTime = (tv.tv_sec - start_tv.tv_sec) +
				(tv.tv_usec - start_tv.tv_usec) / 1000000.0;
				std::cout <<"\n **TIME numberOfEdgeNeigbornFactorCounter: "<<loadTime;
	#endif
	return 1;

}

/**
 * \brief Copy default factors
 * Copy factor values used for DNS example
 * the factors are in log format
 * This code is not used because factor values are
 * loaded from the binary file,
 * but it is a reference of
 */
void copyFactors(TOPOLOGY_VALUE_TYPE *temp_toplogy, TOPOLOGY_VALUE_TYPE &vertexId1, TOPOLOGY_VALUE_TYPE &vertexId2,ITERATOR_INDEX offset ){

	std::cout <<"\n v1" <<vertexId1 <<" , v2"<<vertexId2;
	if (vertexId1<vertexId2){
		std::cout <<"\n**it is a host\n";
		temp_toplogy[offset++] = inflate_factor_value(-0.67334);// (0, 0)
		temp_toplogy[offset++] = inflate_factor_value(-0.71334);// (0, 1)
		temp_toplogy[offset++] = inflate_factor_value(-1.38629);// (1, 0)
		temp_toplogy[offset]   = inflate_factor_value(-0.28768);// (1, 1)

	}else{
		std::cout <<"\n**it is a domain\n";
		temp_toplogy[offset++] = inflate_factor_value(-0.67334);// (0, 0)
		temp_toplogy[offset++] = inflate_factor_value(-1.38629);// (0, 1)
		temp_toplogy[offset++] = inflate_factor_value(-0.71334);// (1, 0)
		temp_toplogy[offset] = inflate_factor_value(-0.28768);// (1, 1)
	}
}

/**\brief Copy file factors
 * Copy factor values to the topology structure using
 * values loaded from the file.*/
void copyFactors2(TOPOLOGY_VALUE_TYPE *temp_toplogy, TOPOLOGY_VALUE_TYPE &vertexId1, TOPOLOGY_VALUE_TYPE &vertexId2,ITERATOR_INDEX offset, float *edgeFactors  ){
  //	std::cout <<"\n v1" <<vertexId1 <<" , v2"<<vertexId2;

	int i=0;
	int j=0;
	int pos1 =0;
	int pos2 =0;

	if (vertexId1<vertexId2){
			for (i=0; i< numVars; i++){
				pos1= i*2;

				for (j=0; j< numVars; j++){
					pos2= pos1 + j;
					temp_toplogy[offset++] =inflate_factor_value(edgeFactors[pos2]);
				}
			}

		}else{

			for (i=0; i< numVars; i++){
				for (j=0; j< numVars; j++){
					pos2= i + j*2;
					temp_toplogy[offset++] =inflate_factor_value(edgeFactors[pos2]);
				}
		}
	}
}

/**
 * \brief Read Edges or neigborn of the vertices
 * Read neighbors values from the file in order to construct topology index array
 * The topology index is sorted a array by vertex id, where hold all unary and edge factors within.
 *
 * The method builds:
 *
 * From vertex0 (connected to): unaryfactor1 unaryfactor2 edgefator1 edgefactor2 edgefactor3 edgefactor4 toVertexId1 edgefactor1..edgefactor4 toVertexId2
 *
 * @PARAM: pointers to the file in memory, temp_topology to map edge list, total number of nodes
 * @RETURN: 1: sucess
 */
int getNeigborns(GraphFileMemoryMap &graphFilePtrs , TOPOLOGY_VALUE_TYPE *temp_topology,
		vertex_info* vertex_array, size_t num_nodes, int numNodeId){


	int neighbornIndex    = 0;
	std::ifstream inFileGraph(graphFilePtrs.fileName,std::ios::in| std::ios::binary );

	TOPOLOGY_VALUE_TYPE  neighbors[10];
	TOPOLOGY_VALUE_TYPE  vertex		  = 0;
	TOPOLOGY_VALUE_TYPE* start_indexes= NULL;


	ITERATOR_INDEX i;
	ITERATOR_INDEX j;
	ITERATOR_INDEX offset;
	ITERATOR_INDEX si;
	ITERATOR_INDEX iEdgeCounter = 0;

	float *edgeFactors=(float*)malloc(graphFilePtrs.factorBlockSize );
	start_indexes = (TOPOLOGY_VALUE_TYPE*)malloc(sizeof(TOPOLOGY_VALUE_TYPE)*num_nodes);
	std::memset( start_indexes,0,sizeof(TOPOLOGY_VALUE_TYPE)*num_nodes);


	struct timeval start_tv;
	gettimeofday(&start_tv, NULL);

	//move to the end of until "f" of factor
	neighbornIndex=2;
	if (inFileGraph.good()){

		/*skip header*/
		inFileGraph.seekg(graphFilePtrs.headerSize,std::ios::beg);

				while (iEdgeCounter < graphFilePtrs.num_of_edges ){

					inFileGraph.read((char*)neighbors,2*sizeof(size_t));
					inFileGraph.read((char*)edgeFactors,graphFilePtrs.factorBlockSize );


					for  (i =0; i<neighbornIndex; i++){
						vertex = neighbors[i];
						//offset = topologyOffset[vertex]; //+2 for the inital unary

						/*if the vertex is not in the partition size, then allocation is ignored*/
						if (vertex <startThreadIndex[numNodeId] || vertex>= endThreadIndex[numNodeId] ) {
							/*ignore*/
							continue;
						}
						offset = vertex_array[vertex].first_factor_offset;
						si     = start_indexes[vertex];


						if(si==0){
							/*update unary factors, to check how to remove from here later*/
							temp_topology[offset]   =inflate_factor_value(edgeFactors[0]);
							temp_topology[offset+1] =inflate_factor_value(edgeFactors[1]);
						}

						offset = offset+ numVars+(si*topologySizeByVertex);
						//offset = offset+ (si*topologySizeByVertex);

						for(j=0; j<neighbornIndex; j++) {
							// For all the neighbors in this factor, put in the vertex IDs of the neighbors.
							if(i != j){
								si++;
								//std::cout <<"\noffset:"<<offset<< "\tv=" <<vertex <<"\tnumEdgeFactors"<<numEdgeFactors;
								copyFactors2(temp_topology, vertex, neighbors[j],offset,(edgeFactors+2) );
								offset+= numEdgeFactors;
								temp_topology[offset]  =  neighbors[j];
							}

						}
					// the next set of neighbors will be put in from this index (we don't have to traverse through the
					// temp_topology[vertex] array to figure out where to add the information from.
					start_indexes[vertex] = si;
					}

				iEdgeCounter++;
			}
	}else{
		std::cout <<"\nError reading the file"<<graphFilePtrs.fileName;
	}


	if (start_indexes!=NULL){
		free(start_indexes);
		start_indexes=NULL;
	}

	if (edgeFactors!=NULL){
		free(edgeFactors);
		edgeFactors=NULL;

	}
	#ifdef DEBUG_BREAKDOWN_IME_ON
		struct timeval tv;
		double loadTime = 0;
		gettimeofday(&tv, NULL);
		loadTime = (tv.tv_sec - start_tv.tv_sec) +
		(tv.tv_usec - start_tv.tv_usec) / 1000000.0;
		std::cout <<"\n **TIME getNeighborEdgeList:"<<loadTime;
	#endif
	return 1;
}

/**\brief Read graph partition for each node
 * Read Partition Size for each node.
 * assumes that  there is an specific partition file that
 * enumerates the list of partitions that will run the inference
 * and how may vertices will be process in each node
 * like graph1.par.8 which implies to read 8 node partitions
 * */
int readPartitionSizes(std::string fileName, int num_sockets_to_use, ITERATOR_INDEX *sizeGlobalArrayDistribution){


	std::stringstream parName;
	parName << fileName <<"."<< num_sockets_to_use;

	std::cout <<"\n Reading partitions file:"<< parName.str();
	std::ifstream fin(parName.str().c_str());
	std::string line;
        int line_number=-1;

	while(fin.good() && getline(fin, line, line_number++)){
		line = trim(line);
		assert(line_number<num_sockets_to_use);
		sizeGlobalArrayDistribution[line_number]=(ITERATOR_INDEX)atoi(line.c_str());

	}

	fin.close();
    printf("\nnumber of lines %d\n",line_number);
	assert(line_number==num_sockets_to_use);
	return line_number;
}

/**
 * Methods for debugging purposes
 *
 *
 * */
/**\brief check neighbors
 * print some neighbors of the topology arra
 * Check topology struct*/
int checkNE(const TOPOLOGY_VALUE_TYPE  *topologyOffset, size_t num_nodes){

	int64_t offset;
	for (size_t i=0; i <num_nodes; i++){

			offset = topologyOffset[i];
			if (offset <0 || offset>4999999975){
					std::cout <<"\n vetex ="<<i;
					std::cout <<"\n id ="<<i;
					std::cout <<"\n Offset ="<<offset;
					std::cout <<"\n topologyOffset[id] ="<<topologyOffset[i];
				}
		}
	return 0;
}

/**\brief displa arrays
 * Print n-size elements the topology arrays
 *
*/
int displayArrays(const TOPOLOGY_VALUE_TYPE  *topology, size_t size ){

	std::cout <<"\n START array dispaly:  ";
	for (size_t i =0; i < size; i++){
		std::cout <<" "<< topology[i];
	}

	return 0;
}

/**
 *
 * Disply topology struct*/
int checkSumArrays(VERTEX_VALUE_TYPE  *arr, size_t size,int key ){

	VERTEX_VALUE_TYPE sum=0;
	//std::cout <<"\n START array checksum:  ";
	for (size_t i =0; i < size; i++){
		sum += arr[i];
	}
	std::cout <<"\n ID:"<<key<<" checksum:"<<sum;

	return 0;
}
void checkSumTopology(const TOPOLOGY_VALUE_TYPE  *topology, size_t size){

	long sumValue = 0;

	for (size_t i =0; i < size; i++){
		sumValue =  sumValue + topology[i];
	}
	std::cout <<"\n END topology checksum: "<<sumValue;
}



