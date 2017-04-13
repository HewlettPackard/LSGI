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
// Description : Consumer: Pulling updates from file using mem_invalidate and printing file content
// for each iterations
// Assumptions:
// 1. the file should be already created in the file system, if the file does not exist use first run the producer program.
// 2. there is producer process (test_updatePersisLoop.cpp) updating file content
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

int main (int argc, char *argv[])
{
  int fdin;
  char *src;
  struct stat statbuf;
  int enterVal=0;

  if (argc != 3) {
    printf ("\nusage: a.out <fileToUpdate> <N iterations>\n");
    exit(1);
  }

  int nIterations= atoi(argv[2]);
  /* open the input file */
  if ((fdin = open (argv[1], O_RDWR)) < 0){
    printf ("can't open %s for reading\n", argv[1]);
    return 1;
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
	  /*invalidate caches to get latest updates*/
  	  pmem_invalidate(src,statbuf.st_size);
  	  printf("\nNew file Content after invalidate:");
  	  for(int i=0; i<5; i++){
        printf("\n[%d] = %c",i,src[i]);
      }


  	  std::cout <<"\nPull Ok, ITER=............"<<n;
  	  std::cout <<"\nEnter to continue to next iteration";
  	  std::cin >> enterVal;
  }
  printf("\nDone:\n");
  close(fdin);
} /* main */
