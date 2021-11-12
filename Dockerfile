FROM ubuntu:20.04

WORKDIR /app

RUN apt-get update && \
    apt-get install -y libpcap-dev gcc g++ make wget && \
    cd /opt && \
    wget https://github.com/seladb/PcapPlusPlus/releases/download/v21.11/pcapplusplus-21.11-ubuntu-20.04-gcc-9.tar.gz && \
    tar -xzf pcapplusplus-21.11-ubuntu-20.04-gcc-9.tar.gz

RUN cd /opt/pcapplusplus-21.11-ubuntu-20.04-gcc-9 && ./install.sh

COPY main.cpp Makefile ./

RUN make

CMD ["./application"]


