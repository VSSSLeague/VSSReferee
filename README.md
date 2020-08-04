# VSS_Referee

## Introduction
The VSS-Referee is an module created to the **IEEE Very Small Size Soccer League**, possibiliting the automatic judge of the field, creating the possibility to the teams catch the data coming from it and replace strategically their robots, avoiding a little bit the human presence/necessity at the field.

## Requirements
 * g++
 * Qt
 * Google protocol buffers (protoc)
 
## Compilation
First of all, check the ports and the addresses in the `main.cpp` file and change if needed.  
Run the `protobuf.sh` file at the main folder, using the command `sudo sh protobuf.sh` in the terminal.  
Create an folder named `build`, open it and run the command `qmake ..`  
So, after this, run the command `make` and if everything goes ok, the binary will be at the folder `bin` (at the main folder).  

## Usage
After compilation, simply run the binary at the `bin` folder using the `./VSS-Referee` command at the terminal.  

## Modules explanation
Currently, the VSS-Referee have 3 modules inside it:  

### VSS-Vision Client
The VSS-Vision Client is responsable to catch vision data (robots and ball positons, field geometry, etc.) from the network, possibilyting to use these data later to process some fouls occurred in the field.

### VSS-Referee
The VSS-Referee module is responsable to use the VSS-Vision Client data and check the fouls occured in the field. **Currently, this module is (by default) sending an `FREE_KICK` command for the `BLUE` team for tests.** For the future, this module will be able to send consistent data about the fouls occured in the field.  
It uses the `vssref_command.proto` protobuf file to send the commands. This protobuf supports the 4 types of fouls of VSS category, that are: `FREE_KICK`, `PENALTY_KICK`, `FREE_BALL` and `GOAL_KICK`, also sending the team color (`BLUE` or `YELLOW`), showing the team that need to do that foul.  
The `example/main.cpp` file contains an example of how the teams can catch data from VSS-Referee.  

### VSS-Replacer
The VSS-Replacer module is responsable to catch the data sent by the teams containing their desired position of each robot in the field, posteriorly positioning the robots in the `FIRASim` simulator.  
The teams need to use the `vssref_placement.proto` protobuf file to send these commands to the VSS-Replacer. This protobuf have an var type named `Frame` on it, that contains the team `Color` and an vector of `Robot` type. The definition of these vars can be seen in the `vssref_common.proto` protobuf file.  
The `example/main.cpp` file contains an example of how the teams can send placement data to VSS-Replacer.  

## Example of usage
An example of usage of the software is shown at the [example](https://github.com/zsmn/vss_referee/blob/master/example/main.cpp) file. It shows how an team can receive the data coming from the VSS-Referee and how to send placement data to it.  
To use this example to check the funcionality of the VSS-Referee, you can literally use the same step-by-step compiling instructions showed above (but remember to be at the example folder).  
