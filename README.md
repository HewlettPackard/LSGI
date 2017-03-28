# LSGi
Large Scale Graph inference

We construct an iterative graph processing engine exploiting The Machine’s Fabric-attached-memory as a communication medium, and
demonstrate near-linear scalability and a 162 speed-up over today’s state of the art graph processing system on a Dragonhawk
and a projected 85 speed up on the Machine Fabric Testbed (MFT).

The engine takes a graph and associated metadata as input and performs the following computation. Each vertex is
associated with a state variable. The state variable is updated iteratively based on the states of the neighboring
vertices as well as the metadata associated with the graph until convergence is achieved.

Contributors: Fei, Tere, Krishna, Hernan, Janneth

Acknowledgements
We thank our colleagues Brad Morrey, Terence Kelly, Hideaki Kumura and Rob Schneider for their support understanding memory mappings, libpmem library interfaces, and CPU optimization techniques.  Thanks to Qiong Cai who developed the model to estimate performance on The Machine based on performance on Dragonhawk statistics. 

