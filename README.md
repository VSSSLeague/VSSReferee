[![Build Status](https://travis-ci.com/VSSSLeague/VSSReferee.svg)](https://travis-ci.com/VSSSLeague/VSSReferee)

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
Create an folder named `build`, open it and run the command `qmake ..`  
So, after this, run the command `make` and if everything goes ok, the binary will be at the folder `bin` (at the main folder).  

## Usage
After compilation, simply run the binary at the `bin` folder using the `./VSS-Referee` command at the terminal. 
**Remember to change the Json file to adjust the parameters for your need!**

## Modules explanation

