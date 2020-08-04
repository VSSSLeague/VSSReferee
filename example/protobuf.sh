# !/bin/bash

# Generates protobuf skeletons
cd include/protobuf/firasim
sh protobuf.sh
cd ..
cd vssreferee
sh protobuf.sh
cd ../../../

