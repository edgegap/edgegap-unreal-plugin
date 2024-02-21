FROM ubuntu

RUN apt-get update
RUN apt-get install openssh-server sudo -y

COPY . /app
WORKDIR /app

RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 m
RUN sudo chown -R m:sudo *

USER m

CMD ./<PROJECT_NAME>Server.sh -log