# simdaq
pure c++ and posix daemon

## build
g++ -o pcxxpd *.cpp -std=c++14 -lpthread -lstdc++fs

## docker
### build
docker build -t pcxxpd .
### examine
docker run -v /dev/log:/dev/log --rm -it pcxxpd
