[![Build Status](https://travis-ci.com/MaracatronicsRobotics/VSSReferee.svg)](https://travis-ci.com/MaracatronicsRobotics/VSSReferee)

# VSS_Referee

## Introduction
The VSS-Referee is the automatic referee module created to **IEEE Very Small Size Soccer League**, creating the possibility to teams catch the data coming from it and replace strategically their robots, decreasing the human necessity at the field.

## Requirements
 * g++
 * Qt
 * Qt OpenGL
 * Google protocol buffers (protoc)
 
## Compilation
First of all, check the `constants/constants.json` file and update for your parameters (addresses, ports, team names, team colors, etc.)
Run the `protobuf.sh` file at the main folder, using the command `sudo sh protobuf.sh` in the terminal.  
Create an folder named `build`, open it and run the command `qmake ..`  
So, after this, run the command `make` and if everything goes ok, the binary will be at the folder `bin` (at the main folder).  

## Usage
After compilation, simply run the binary at the `bin` folder using the `./VSS-Referee` command at the terminal. 
**Remember to change the Json file to adjust the parameters for your need!**

## Modules explanation
Currently, the VSS-Referee have 3 modules inside it:  

### VSS-Vision Client
The VSS-Vision Client is responsable to catch vision data (robots and ball positons, field geometry, etc.) from the network, possibilyting to use these data later to process some fouls occurred in the field.

### VSS-Referee
The VSS-Referee module is responsable to use the VSS-Vision Client data and check the fouls occured in the field. 
It uses the `vssref_command.proto` protobuf file to send the commands. The following fouls are supported by this protobuf:
- [x] `KICKOFF`: This command is sent when the game start (when you open the referee), after valid goals and after half passed.
- [x] `FREE_BALL`: This command is sent when the ball is stucked in any part of the field for `ballStuckTime` seconds. It passes the quadrant where the foul occured, that can be `QUADRANT_1`, `QUADRANT_2`, `QUADRANT_3` and `QUADRANT_4`. These quadrants begin from the TOP-RIGHT quadrant and the others follow an anti-hour orientation, so the `QUADRANT_2` is the TOP_LEFT and so on.
- [ ] `FREE_KICK`: Currently this command isn't supported by the referee due to it's complexity to be analysed.
- [x] `GOAL_KICK`: This command is sent when two attackers disputate the ball with the enemy goalkeeper.
- [x] `PENALTY_KICK`: This command is sent when two defenders are inside of the goal area (at least 50%) with the ball in the area, or when the goalkeeper didn't takeout the ball from the goal area in at least `GKTakeoutTime` seconds.
- [x] `STOP`: This is an new command, that occurs when the teams placed succesfuly. This allows the teams to make tiny modifications at their robot orientations to adapt for the other teams placement.
- [x] `GAME_ON`: This command is sent when the stop ended , so all the players can play normally.

The `example/main.cpp` file contains an example of how the teams can catch data from VSS-Referee.  

### VSS-Replacer
The VSS-Replacer module is responsable to catch the data sent by the teams containing their desired position of each robot in the field, posteriorly positioning the robots in the `FIRASim` simulator.  
The teams need to use the `vssref_placement.proto` protobuf file to send these commands to the VSS-Replacer. This protobuf have an var type named `Frame` on it, that contains the team `Color` and a vector of `Robot` type. The definition of these vars can be seen in the `vssref_common.proto` protobuf file.  
The current version, when your team don't place the robots, automatically place your team based at the **IEEE VSS Rules**, but it **only can recognize your goalkeeper**, cause it store the time that each robot spent at the goal area, so the other players will be literally placed following the rules. You can check that rules placement at [Rules](http://200.145.27.208/cbr/wp-content/uploads/2020/07/vssRegras_Portugues.pdf).
The `example/main.cpp` file contains an example of how the teams can send placement data to VSS-Replacer.  

#### Examples of placement
Here you can see examples of how the referee place automatically the robots in the field (if the team don't send the placement packet).
##### Penalty

![Penalty Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/Penalty.jpeg)
##### Kickoff

![Kickoff Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/Kickoff.jpeg)
##### Free Ball
- Quadrant 1 (TOP_RIGHT)

![FreeBall 1 Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/FreeBall_TOPRIGHT.jpeg)
- Quadrant 2 (TOP_LEFT)

![FreeBall 2 Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/FreeBall_TOPLEFT.jpeg)
- Quadrant 3 (BOT_LEFT)

![FreeBall 3 Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/FreeBall_BOTLEFT.jpeg)
- Quadrant 4 (BOT_RIGHT)

![FreeBall 4 Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/FreeBall_BOTRIGHT.jpeg)
##### Goal Kick

![GoalKick Placement](https://github.com/MaracatronicsRobotics/VSSReferee/blob/master/resources/GoalKick.jpeg)
## Example of usage
An example of usage of the software is shown at the [example](https://github.com/zsmn/vss_referee/blob/master/example/main.cpp) file. It shows how an team can receive the data coming from the VSS-Referee and how to send placement data to it.  
To use this example to check the funcionality of the VSS-Referee, you can literally use the same step-by-step compiling instructions showed above (but remember to be at the example main folder).  
