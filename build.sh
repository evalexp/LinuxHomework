PWD=$(pwd)
SCRIPT_LOC=$(cd "$(dirname "$0")";pwd)

cd $SCRIPT_LOC/build

cmake -DCMAKE_BUILD_TYPE=Debug .. -Wno-dev && make

cd $PWD