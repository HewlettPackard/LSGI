# LSGi System

![arch](https://github.com/HewlettPackard/LSGI/blob/master/docs/LSGiSystem.jpg)

# LSGi Package
The LSGI Package has the following modules:
/config: Scripts to configure remote nodes: compile and ssh passwordless.
/data: It contains input graphs as binary files, configuration files and will host local outputs of the inferences like local states and statistics.
/demo: It contains scripts to demo the existing functionality for the project that comprises:
  -run a single node inference 
  -run multimode inference 
  -run query client to request inference states
/deploy. Scripts to deploy the local checkout to remote nodes.
/docs: It contains multiple documents that describe the inference engine.
/source: It contains the main source code for the modules: inference, query service and
  query client. To compile and generate the executables it should be executed “make clean”
  and then “make” commands.
/test. It contains several basic test cases to test librarian access, pmem persistence and
  sync that are fundamental to verify that the shared states are working in the environment.
hosts: main file to configure the computing nodes.
Makefile: main configuration and deployment goals
