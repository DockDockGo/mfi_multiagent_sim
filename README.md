# SubT Multi UAV

This repo is for simulating multiple subt uavs in separate docker containers. It uses a `172.20.20.0/24` network for communication between containers. If there is a conflict with this on your system check the `Changing the Network` section for instructions on how to use a different network. This should be done before running the `docker_build.sh` script.

### Getting and Building the Code

To get the code and build the docker image run the following. It will ask for a sudo password so that it can delete files that might be left over if the image was built previously.

```
git clone git@bitbucket.org:castacks/subt_multi_uav.git
cd subt_multi_uav
./docker_build.sh
```

### Running

To run the simulation for uav1 do:

```
./docker_run.sh 1
```

When inside the docker container do:

```
roscore &
mon launch core_central sim_main.launch
```

When you can see velodyne points in rviz, the sim is fully loaded and you can press the `Autonomously Explore` button in the RQT GUI to launch the drone.

The same steps can be repeated to launch other drones by passing a different number, 1-4, to the `./docker_runs.sh` script.

To start the sim with a different world you can use the `world` argument to the launch file, for example:

```
mon launch core_central sim_main.launch world:=hawkins_qualification.world
```

To list the available worlds and the directory where they are stored you can run the command:

```
worlds
```

You can add new words to the directory shown in this command.

The files and directories `ws/`, `.bashrc`, and `.bash_history` are loaded as volumes for the docker container so changes you make within one container will be applied to all other containers and to those files and directories outside of docker.

### Changing the Network

If the `172.20.20.0/24` network conflicts with a network on your computer, there are two files you need to change to reconfigure it.

In `docker_build.sh`, the docker network is created in the first line of the script. Change the `172.20.20/24` to whatever you want. If you have already run `docker_build.sh` previously, you will also need to do `docker network rm subt_multi_uav_network` before running `docker_build.sh` again. If you have already run `docker_build.sh` and just want to change the network you should be able to run just the `docker network create...` line and not the rest of the script, in addition to making the following change in the communication manager package.

For the communication manager to find the drones running in other docker containers, their IPs must be added in `ws/src/communication_manager/config/USER_QOS_PROFILES.xml` in the same way that the `172.20.20.*` IPs are. In the `172.20.20.0/24` netowrk, usually the host computer will get assigned `172.20.20.1` and the four robots will get assigned `172.20.20.2-5` so listing all the IPs is probably not necessary. The only IPs that need to be there are the IPs assigned to each docker container.