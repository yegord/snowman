FROM ubuntu:18.04

RUN useradd -m snowman
RUN apt-get update -y && \
	apt-get upgrade -y && \
	apt-get install -y build-essential cmake libboost-dev qt5-default git sudo

USER snowman
RUN cd /tmp && \
	git clone https://github.com/yegord/snowman.git && \
	cd snowman && mkdir build && cd build && \
	cmake ../src && make -j$(nproc)

USER root
RUN cd /tmp/snowman/build && \
	make install && \
	cd / && \
	rm -rf /tmp/snowman

RUN echo "snowman:snowman" | chpasswd && adduser snowman sudo

USER snowman
CMD /bin/bash
