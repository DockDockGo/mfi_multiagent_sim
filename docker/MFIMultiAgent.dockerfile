FROM osrf/ros:humble-simulation-jammy

RUN useradd -ms /bin/bash admin

# USER admin
WORKDIR /home/admin

USER root

COPY extras/robot-setup-tool ./robot-setup-tool

RUN cd ./robot-setup-tool/package-setup && ./setup-simulation-custom.sh

RUN cd ./ && mkdir worlds

