# MFI Multi-Agent Simulation environment

This repo is for simulating two agents case for MFI project.


To clone this repo run:
```
git clone --recursive -j8 -b cbs_planner_updated git@github.com:DockDockGo/mfi_multiagent_sim.git
```

### Requirements

#### Docker

Install from the [website](https://docs.docker.com/engine/install/ubuntu/) and [post-install](https://docs.docker.com/engine/install/linux-postinstall/)

```
curl https://get.docker.com | sh \
  && sudo systemctl --now enable docker
```

#### Nvidia-docker

```
curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add -
distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | sudo tee /etc/apt/sources.list.d/nvidia-docker.list
sudo apt-get update
sudo apt-get install -y nvidia-docker2
sudo pkill -SIGHUP dockerd
```

#### Nvidia-container-runtime

Please follow the [guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) from Nvidia.

```
sudo pkill -SIGHUP dockerd
```

Verify using:

```
docker info | grep -i runtime
```

It should contains nvidia.

### Getting and Building the Code

To get the code and build the docker image run the following. It will ask for a sudo password so that it can delete files that might be left over if the image was built previously.

```
./docker_build.sh
```

### Running

To run the simulation:

```
./docker_run.sh
```
