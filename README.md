# LSGi
Large Scale Graph inference
### Overview

Graphical Inference is a remarkable algorithm for graph analytics that abstracts knowledge combining probabilities and graph representations. It captures useful insights for solving problems like malware detection, genomics analysis, IoT analytics, or online advertisement. Usually, type of applications demands fast response which becomes a challenge when the graphs are large. For example, in the security domain, a fast-time response is required for malware detection (3-4 minutes), but state-of-art solutions may take hours. Therefore, we need to exploit parallelism to speed-up the runtime, but the main challenge is the system scalability. As the size of graphs increase, the number of cores on a single commodity server (8-32 cores) is not enough to achieve runtime targets consequently more nodes are required, but the amount of communication involved also increases.

We construct an iterative graph processing engine exploiting The Machine’s Fabric-attached-memory as a communication medium, and
demonstrate the benefits of a memory driven computing architecture achiving near-linear scalability and two order of magnitud faster than state of the art graph processing system on a SuperdomeX. Our memory-centric approach makes efficient use of the FAM bandwidth and reduces synchronization overhead among computing nodes while exploiting massive parallelism via the large number of cores on The Machine.

## LSGi System
The system includes a) the engine as a service to run on multiple machine environments, b) the query service to retrieve inference results in real time and c) the web console manager to monitor workload performance.

## The Core Engine
LSGi engine takes a graph and associated metadata as input and performs the following computation. Each vertex is
associated with a state variable. The state variable is updated iteratively based on the states of the neighboring
vertices as well as the metadata associated with the graph until convergence is achieved.

### Main Contributors 
Fei Chen, Maria Teresa(Tere) Gonzalez Diaz, Krishna Viswanathan, Hernan Laffite, Quiong Cai, Janneth Rivera

### Acknowledgements
We thank our colleagues Nandish Jayaram, Brad Morrey, Terence Kelly, Hideaki Kumura, Jun Li and Rob Schneider for their support understanding memory mappings, libpmem library interfaces, and CPU optimization techniques.  Thanks to Qiong Cai who developed the model to estimate performance on The Machine based on performance on SuperdomeX statistics. Thanks to Greg Pearson and Alex Jizrawi for testing and evaluating LSGi on The Machine Fabric Test bed(MFT) and The Machine Simulator(TMAS). Thanks to  April Mitchell, Sharad Singhal and Ram Swaminathan for their management support.

### How To..
We have included extensive documentation and demo examples to evaluate LSGi System on dummy datasets. Please refer to document folder to see further details.

### References 
"Billion node graph inference: iterative processing on The Machine",HPE Techical Report, 2016,
URL:https://www.labs.hpe.com/publications/HPE-2016-101"

### More Info
- LSGi System Architeture
- Demo Video

### License
“© Copyright 2017  Hewlett Packard Enterprise Development LP

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”


