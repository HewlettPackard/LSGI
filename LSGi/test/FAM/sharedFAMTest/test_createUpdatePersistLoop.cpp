/*
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
// Name        : test_updatePersistLoop.cpp
// Author      : tere, fei, krishna
// Version     :
// Copyright   : Your copyright notice
// Description : Producer code: Update file content using mem_perisist method for several iterations
// if the file does not exist, it will create the file in the path
// Dec 2, 2015
//============================================================================
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>
#include <libpmem.h>
#include <iostream>
#include <string.h>

#define FILESIZE 10

/* create file
 * to test update push to the librarian
 * @Params:filename, filesize
 * @return: pointer to the file;
 * **/
int createFile(char *fileName, int fileSize){
	int fdout;
	char *vals= (char*)malloc(fileSize);

	if ((fdout = open (fileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR)) < 0){
	        printf ("can't create %s for writing\n", fileName);
	      }

	      /* truncate output file size to the size of the input file */
	      if (ftruncate(fdout, fileSize)) {
	        printf ("Error, ftruncate error, file may already exist\n");
	        close(fdout);
	        -1;
	      }
	      memset(vals,'0', fileSize);
	      write (fdout, vals,fileSize);


	      if(vals != NULL){
	    	  free(vals);
	    	  vals=NULL;
	      }
	      return fdout;
}

/* main driver
 * code to test push update and sync to a librarian file
 *
 * */
int main (int argc, char *argv[])
{
  int fdin;
  char *src;
  struct stat statbuf;
  int enterVal=0;

  if (argc != 4) {
    printf ("\nusage: a.out <fileToUpdate> <changeToValue> <N iterations>\n");
    exit(1);
  }

  char val= argv[2][0];
  int nIterations= atoi(argv[3]);
  /* open the input file */
  if ((fdin = open (argv[1], O_RDWR)) < 0){
    printf ("\n file does not exist %s for reading\n", argv[1]);
    printf ("\n File will be created");

    /* create the file when it does not exist*/
    fdin =createFile(argv[1],FILESIZE);
    if (fdin==-1){
    	return 0;
    }
  }


  /* find size of input file */
  if (fstat (fdin,&statbuf) < 0){
    printf ("\nfstat error\n");
    return 1;
  }

  std::cout <<"\nfile size:"<<statbuf.st_size;

  /* mmap the input file */
  if ((src = (char *)pmem_map (fdin)) == (caddr_t) -1){
   printf ("\nmmap error for input\n");
   return 1;
  }


  for (int n=0; n<nIterations; n++){
  	  printf("\nNew file Content:");
  	  for(int i=0; i<5; i++){
        src[i]=val;
        printf("\n[%d] = %c",i,src[i]);
      }
  	  val++;
  	  pmem_persist(src,5);
  	  std::cout <<"\nSync Ok, ITER=............"<<n;
  	  std::cout <<"\nEnter to continue to next iteration";
  	  std::cin >> enterVal;
  }
  printf("\nDone:\n");
  close(fdin);
} /* main */
