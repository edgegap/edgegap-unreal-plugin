FROM ubuntu

RUN apt-get update
RUN apt-get install openssh-server sudo jq -y && \
    apt-get clean && \
    rm -rf /var/lib/{apt,dpkg,cache,log}/

COPY . /app
WORKDIR /app

RUN useradd -rm -d /home/ubuntu -s /bin/bash -g root -G sudo -u 1000 m -o
RUN sudo chown -R m:sudo *

USER m

CMD ./StartServer.sh
