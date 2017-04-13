
# LSGi: Inference Engine

  This code implements a iterative graph processing engine that
  take advantange of The Machine architecture - Fabric Attached Memory-large number of cores
  to scale inference processing on large  graphs.

## Overview:
  
  Graphical Inference is a remarkable algorithm for graph analytics
  that abstracts knowledge combining probabilities and graph representations
  to capture useful insights for solving problems like malware detection,
  genomics, or online advertisement. These are analytics applications
  that demands fast response. For example, in the security domain, a fast-time
  response is required for malware detection (3-4 minutes), but state-of-art
  solutions take close to an hour. Therefore, we need to exploit parallelism
  to speed-up the runtime, but the main challenge is the system scalability.
  As the size of graphs increase, the number of cores on a single commodity
  server (8-32 cores) is not enough to achieve runtime targets consequently
  more nodes are required.
  We have build the graph inference engine which computes on graph partitions
  mapped to multiple computing nodes. Each node performs iterative local processing
  that needs to read remote outcomes which derives high communication volume over
  the network and slows down runtimes on traditional clusters. To address this problem,
  we have built a scalable large scale inference engine taking advantage of
  The Machine architecture. Our memory-centric approach makes efficient use of the FAM
  bandwidth and reduces synchronization overhead among computing nodes while exploiting
  massive parallelism via the large number of cores on The Machine.
 
## Reference Paper:
  https://www.labs.hpe.com/publications/HPE-2016-101
## Contributors 
Fei,Tere, Krishna, Nandish, Hideaki
Last update: April 10, 2017
 
## Requirements
 Before compiling, intall library Dependencies: libpmem,pthread
 
  To compile use Makefile
  ./make clean
  ./make

  Main Driver: EngineMain.cpp

## Test and Exectue
  Go to ../../demo/

## Modules:

-InMemoryGraph: Implements the raph Structure in Memory with an optimize Index.
-InferenceConfig: Implements main engine configurations.
-BinaryReader: Implements the graph loading from file to Memory.
-GlobalStates: Implements inference states in DRAM and synchronize to FAM.
-ProcessSync: Implements synchronization barriers accros running computing nodes.
