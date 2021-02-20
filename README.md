[![Organization](https://img.shields.io/:Very%20Small%20Size%20Soccer-VSSReferee-82B2CD.svg?logo=data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAADjElEQVQ4jSXS2U/bBQDA8R/rapEB6woUaCljSDnKIdfAMRgtlLa0/HofUDqgG8cKCGTcKMcQNgKyRdlGNiUmSsY0Bvc0X6YxmvhgzOKLjz4sMZq4qItxYVx+fdi/8MlHaDDqsVvN2EUbLpcDv99PRUU5ZWUl6PKzKSzKRZefTUF+Dt99+zV7u/9iNtcAf6LLTUPwOG3091xkeKCX0cFB+i70MBNYQCKRsLP8mLLSQvJ1uRQW5KDL05KlzSAjIx2t9jV2nz1BCAbcDPV1MTs1wgw3eEU4gmN8H1m0BOv4X1RUFHGwe51DNsnSZpCenoZSqSQh4QSVZ8oRmn1O+nsukqw8gW/sFyqrbCSnJBATE01mZgZbWxt88+gBwhGBQ+6h0ajZ/uJTFAo5J09qEDrON+P2iBi8GhJTZOzvPcfnd+Jy2mgw6lGrU3n01Zf8/uRnrK5i/P4q0tJUaDRqTmVqEOAQq83EIavss0ioxcL8zDCDb4bp6GjGbDKgUispyMtCFBv47el9MrWJaLMzKC0pQGgNeZFIoujoaKM95GN6vI+F6WEW5kYZu9xDOOTHYKhHpVby2eZtSopyODj4ldJSHXX6MgSv101MTDRJSQm8PTnExx/dZn1tieX5WQ4PduCuCoeop6AwF7U6ldd1WdTWVmIwnMFkrEQINHtQKOQkJSXw/TMnA70RShtbubEyx8zUAEG/SJPZgEFfzd7eC+qN5zCZTFistdTX1SCsvHuNO3fWiYoSiJUdJXL/KaHzzVxdmGCgt51wWwCvy4pTNJGmVnLs2KuIosjVx/9xurwYYX5+DpUqBZlMilQqoSXoA/aZmujHYq2mMxwk1OKmyWZCoZAjlUoYCRUTHx+LtdGIMDE5gsfjIj4+Frk8nlp9Fd2dzdy6OcdPtPLiYIu2Vjdnq07zz99/0Bvp5PnOQ4IBOwGviBC51ElPTxc5OVoc9kbaO4Iv7xfmUnlWhbFJiyxGQld3mKUfDthePUdvd5BIZ5juCwGElqCXtvYW2tpb8HjtWG31KBRycvNOMTs3gaXRiCAIxMUd54Mf4WicwOKVMSZHI4wMdSE4nFY8Xjs+vxOnw0p1zRskJychVxwnJTWRlJSXcDKZlPffW2X9+jU+XFtmdXGKd2aHEZpEM6LdgtlSh9vVhKlBj1204PfYuTx4iem3BlldmmdzY42Hn3/Cg3sbbG/e5ebyHLdWrvA/5UEMH5awhQgAAAAASUVORK5CYII=)](https://github.com/VSSSLeague) ![Build](https://github.com/VSSSLeague/VSSReferee/workflows/Build/badge.svg) [![CodeFactor](https://www.codefactor.io/repository/github/vsssleague/vssreferee/badge)](https://www.codefactor.io/repository/github/vsssleague/vssreferee)

# VSS_Referee

## Introduction
The VSS-Referee is the automatic referee module created to **IEEE Very Small Size Soccer League**, creating the possibility to teams catch the data coming from it and replace strategically their robots, decreasing the human necessity at the field.

## Requirements
 * g++ (used v9.3.0)
 * Qt (used version v5.12.8)
 * Qt OpenGL
 * Google protocol buffers (used protoc v3.6.1)
 
## Compilation
Create an folder named `build`, open it and run the command `qmake ..`  
So, after this, run the command `make` and if everything goes ok, the binary will be at the folder `bin` (at the main folder).  

## Before usage
Remember to change the **src/constants/constants.json** file!

This file will contain some parameters and values ​​useful for the game, such as addresses and ports of the Vision, Referee and Replacer modules.

### Entity
In the Entity field it is possible to modify the frequency of the threads.

### Vision
In the Vision field, it is possible to modify the address and port from which the vision packets will be received, as well as to configure the time (in ms) of filters and enable the use of the Kalman filter.

### Replacer
In the Replacer field, it is possible to modify the address and port from which the positioning packets will be received, as well as configuring the address and port where the packets will be sent (FIRASim related).

### Team
In the Team field, it is possible to modify the name of the teams that will play **(THIS IS NECESSARY BEFORE EACH GAME!)**, in addition to changing the position of the blue team and the amount of players on the field.

### Referee
In the Referee field it is possible to modify the address and port where the Referee commands will be sent, as well as it is possible to change some game constants such as the type of game (Group_Phase, Quarterfinals, Semifinals, Final, etc.), radius of ball, halfs time, etc.

At the field of fouls, it is possible to select whether or not to use the Referee's suggestions and also some constants used to check fouls, such as the time needed for a stucked ball and minimum speed to consider it stucked.

## Usage
After compilation, simply run the binary at the `bin` folder using the `./VSS-Referee` command at the terminal. 

## Modules explanation
Currently, the VSS-Referee have 3 modules inside it:  

### VSS-Vision Client
The VSS-Vision Client is responsable to catch vision data (robots and ball positons, field geometry, etc.) from the network, possibilyting to use these data later to process some fouls occurred in the field.

### Referee workflow

![Referee Workflow](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/workflow.png)

### Concept of PLAY
Referee considers that a `PLAY` is in progress when the ball enters the goal area of ​​either team. There are two ways a `PLAY` can be terminated:

- The ball remained for `ballInAreaMaxTime` seconds in the goal area.
- The ball left the goal area. Note that in this case, both leaving the area back to the field and leaving the area entering the goal are considered.

As soon as a PLAY ends, if the Referee has its suggestion mode active (given by the `useRefereeSuggestions` constants flag) it will interrupt the game with a `HALT` command and provide possible fouls for the human judge to make the decision.
Otherwise, it will make the decision alone, choosing the first `FOUL` that happened (if any).

### Supported fouls
The VSS-Referee module is responsable to use the VSS-Vision Client data and check the fouls occured in the field. 
It uses the `vssref_command.proto` protobuf file to send the commands. The following fouls are supported by this protobuf:
- [x] `KICKOFF`: This command is sent when the game start, after valid goals and after half passed.
- [x] `FREE_BALL`: This command is sent when the ball is stucked in any part of the field for `ballStuckTime` seconds. It passes the quadrant where the foul occured, that can be `QUADRANT_1`, `QUADRANT_2`, `QUADRANT_3` and `QUADRANT_4`. These quadrants begin from the TOP-RIGHT quadrant and the others follow an anti-hour orientation, so the `QUADRANT_2` is the TOP_LEFT and so on.
- [ ] `FREE_KICK`: Currently this command isn't supported by the referee due to it's complexity to be analysed.
- [x] `GOAL_KICK`: This command is sent when two attackers disputate the ball with the enemy goalkeeper.
- [x] `PENALTY_KICK`: This command is sent when two defenders are inside of the goal area (at least 50%) with the ball in the area, or when the goalkeeper didn't takeout the ball from the goal area in at least `GKTakeoutTime` seconds.
- [x] `HALT`: This command is sent when an `PLAY` has ended. In this case, all the players **NEEDS** to be halted. 
- [x] `STOP`: This command occurs when the teams placed succesfuly. This allows the teams to make tiny modifications at their robot orientations to adapt for the other teams placement.
- [x] `GAME_ON`: This command is sent when the stop ended, so all the players can play normally. 

**Note that all players must stop at any `FOUL`, except at `GAME_ON`.**

### Overtime
`OVERTIME` must only be enabled in eliminatory games. Therefore, it will only be enabled when `gameType` constant is different from `Group_Phase`.

It consists of two times of 180 seconds (3 minutes) each. In the case of a tie, the decision will be made through Penalty Shootouts.

### Penalty shootouts
As stated earlier, this stage will only be called if there is a tie during `OVERTIME`.

At this stage, the Referee will allocate infinite time for the game and will be sending alternate `PENALTY_KICK` commands to both teams.

In addition, in this mode none of the common fouls are checked, and the players (in addition of the kicker and goalkeeper) will be removed from the field automatically.

**Note:** The Kicker will be considered as the player positioned closest to the ball and the Goalkeeper will be considered the player who is within the goal area.

Note also that a `PENALTY_KICK` will be given as soon as the `PLAY` ends, that is, **the ball will have to enter the goal area and leave in some way (back to the field) or inside the goal**. That is, **in this case we will not have the time check counting towards the end of an ongoing `PLAY`**.

### Team vs W.O.
The command for this is the `WO Kickoff` and is present only in the Manual Referee and must be executed in the event of a game against a W.O.

This command will remove all players from the opposing team from the field, preventing the human judge from having to remove them manually (as was done before).

Note that the games against W.O. **should only last 3 minutes**, so the human judge must be aware of this time.

### VSS-Replacer
The VSS-Replacer module is responsable to catch the data sent by the teams containing their desired position of each robot in the field, posteriorly positioning the robots in the `FIRASim` simulator.  
The teams need to use the `vssref_placement.proto` protobuf file to send these commands to the VSS-Replacer. This protobuf have an var type named `Frame` on it, that contains the team `Color` and a vector of `Robot` type. The definition of these vars can be seen in the `vssref_common.proto` protobuf file.  
The current version, when your team don't place the robots, automatically place your team based at the **IEEE VSS Rules**, but it **only can recognize your goalkeeper**, cause it store the time that each robot spent at the goal area, so the other players will be literally placed following the rules. You can check that rules placement at [Rules](http://200.145.27.208/cbr/wp-content/uploads/2020/07/vssRegras_Portugues.pdf).
The `example/main.cpp` file contains an example of how the teams can send placement data to VSS-Replacer.  

#### Examples of placement
Here you can see examples of how the referee place automatically the robots in the field (if the team don't send the placement packet).
##### Penalty

![Penalty Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/Penalty.jpeg)
##### Kickoff

![Kickoff Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/Kickoff.jpeg)
##### Free Ball
- Quadrant 1 (TOP_RIGHT)

![FreeBall 1 Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/FreeBall_TOPRIGHT.jpeg)
- Quadrant 2 (TOP_LEFT)

![FreeBall 2 Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/FreeBall_TOPLEFT.jpeg)
- Quadrant 3 (BOT_LEFT)

![FreeBall 3 Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/FreeBall_BOTLEFT.jpeg)
- Quadrant 4 (BOT_RIGHT)

![FreeBall 4 Placement](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/FreeBall_BOTRIGHT.jpeg)
##### Goal Kick

Note that in goal kick the position of the ball is chosen according to the goalkeeper's y-axis. If the y-axis is positive, the ball will be placed above as shown in the image, and if not, it will be placed below.

![GoalKick Placement TOP](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/GoalKick1.jpeg)

![GoalKick Placement BOTTOM](https://github.com/VSSSLeague/VSSReferee/blob/CBR2021/rsc/readme/GoalKick2.jpeg)

## Example of usage
An example of how it is possible to receive Referee packages and send positioning frames to Replacer can be found in this [repository](https://github.com/VSSSLeague/FIRAClient).
