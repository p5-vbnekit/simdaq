FROM debian:buster
ENV DEBIAN_FRONTEND noninteractive
RUN apt update && apt full-upgrade -y
RUN apt update && apt install -y apt-utils
RUN apt update && apt install -y g++ git
RUN apt update && apt install -y bash procps psmisc
RUN apt clean
RUN git -C /root clone https://github.com/p5-vbnekit/simdaq.git simdaq
RUN g++ -o /usr/local/bin/pcxxpd /root/simdaq/*.cpp -std=c++14 -lpthread -lstdc++fs
COPY pcxxpd.docker.conf /etc/pcxxpd.conf
RUN mkdir /root/source-directory /root/destination-directory
RUN echo 1 > /root/source-directory/first.txt && echo 2 > /root/source-directory/second.txt && echo 3 > /root/source-directory/third.txt && echo x > /root/destination-directory/third.txt
WORKDIR /root
ENTRYPOINT ["/usr/bin/bash"]
