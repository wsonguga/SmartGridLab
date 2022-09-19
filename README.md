# SmartGridLab

SmartgridLab (also called ScorePlus) is an integrated and scalable experimental environment whose design framework and major components can be extended to build other cyber-physical testing systems in general. ScorePlus includes both software emulator and hardware testbed. The software emulator emulates the communication and energy behavior of Smart Grid entities. The hardware testbed hosts physical devices, such as micro renewable energy generators (solar panel, wind turbine), energy demanders, storages, and topology switches, which are all connected by power network and communication network in a physical environment. Both the software emulator and hardware testbed follow the same architecture and interface, and the same Smart Grid application program can be tested on either of them without any modification.


@article{tan2016scoreplus,
title = {ScorePlus: A Software-Hardware Hybrid and Federated Experiment Environment for Smart Grid},
author = {Song Tan and WenZhan Song and Steve Yothment and Junjie Yang and Lang Tong},
url = {http://sensorweb.engr.uga.edu/wp-content/uploads/2016/08/DropboxChooserAPI_TSYY-TECS2016.pdf},
doi = {10.1145/2964200},
year = {2016},
date = {2016-04-30},
journal = {ACM transaction on Embedded Computing Systems, Special issue on Emerging Embedded Software and Systems (ESS)},
keywords = {Energy},
pubstate = {published},
tppubtype = {article}
}


*This folder contains the source code for scoreplus software emulator.
*This project contains the software emulator which could integrate with hardware testbed..
*It should work with core 4.5 
*The project is fully tested in Ubuntu 12.04 with CORE4.5 

=====Install CORE 4.5======

Detailed steps can be found:

 http://pf.itd.nrl.navy.mil/core/core-html/install.html#installing-from-source


=====Install boost library ==

Detailed steps can be found:
 
 http://www.boost.org/doc/libs/1_56_0/more/getting_started/unix-variants.html#header-only-libraries
 

 
All the previous steps can be achieved by:

  ./precompile.sh


!!!!!! Note!!!!!! Export Environment permanently by adding the following two lines in ~/.bashrc :

  LD_LIBRARY_PATH=/usr/local/lib
  export LD_LIBRARY_PATH
   

**********************************************************************
For each of the directories, specifically,


==== coordinator directory ====

This is the source for distributed computation and integration.

==== powersimulation directory ====

The main program for power simulator.

==== powerslave ====

The part for distributed computation.

==== scoreplusgui directory ====

This is the gui part.

====================== Netbeans projects ==============================

The following are the independent projects that are created in Netbeans. 

To compile the following project: 

1) Install netbeans full version IDE with c/c++ plugin from:

  https://netbeans.org/downloads/

2)  Configure GNU make tools for netbeans:

  http://dot.netbeans.org/community/releases/72/cpp-setup-instructions.html

3) Simply open the project through netbeans, then compile and run.

OR in command line:

1) change your current directory to the folder.

2) make

3) Two subdirectory called build and dist will be generated after make. 
   The generated executable file is located dist/Debug/GNU-Linux-x86/


==================To install the above all===================


  make
  sudo make install

==================To run=======================

 1) sudo /etc/init.d/scoreplus start

Under install directory:

 2) ./scoreplus
