#!/bin/bash

docker container rm $(docker ps -a -q  --filter ancestor=subt_multi_uav)
docker image rm subt_multi_uav:latest
