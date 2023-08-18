# This is an auto generated Dockerfile for ros:ros-core
# generated from docker_images/create_ros_core_image.Dockerfile.em
FROM nvidia/cuda:10.2-cudnn7-devel-ubuntu18.04

RUN rm /etc/apt/sources.list.d/cuda.list
RUN rm /etc/apt/sources.list.d/nvidia-ml.list

#COPY ws/src /home/ws/src

#COPY extras/.bashrc /.bashrc

#COPY extras/inputrc /etc/inputrc

#COPY extras/init.el /.emacs.d/init.el

# setup timezone
RUN echo 'Etc/UTC' > /etc/timezone && \
    ln -s /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    apt-get update && \
    apt-get install -q -y --no-install-recommends tzdata lsb-release curl python-pip python-setuptools clinfo && \
    rm -rf /var/lib/apt/lists/*

# install packages
#RUN apt-get update && apt-get install -q -y --no-install-recommends \
#    dirmngr \
#    gnupg2 \
#    && rm -rf /var/lib/apt/lists/*

# setup keys
#RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

# setup sources.list
#RUN echo "deb http://packages.ros.org/ros/ubuntu bionic main" > /etc/apt/sources.list.d/ros1-latest.list

RUN pip install numpy toml future serial

RUN echo "deb http://packages.ros.org/ros/ubuntu $(lsb_release -sc) main" > /etc/apt/sources.list.d/ros-latest.list

RUN apt-get install -q -y curl

RUN curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | apt-key add -

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-melodic-desktop-full  emacs ros-melodic-joy python-wstool python-jinja2 ros-melodic-geographic-msgs libgeographic-dev geographiclib-tools python-pip python-catkin-tools libusb-dev libsuitesparse-dev ros-melodic-serial ros-melodic-teraranger-array ros-melodic-teraranger ros-melodic-mav-msgs libglfw3-dev libblosc-dev libopenexr-dev liblog4cplus-dev libpcap-dev opencl-headers ros-melodic-rosmon* ros-melodic-jsk-rviz-plugins ros-melodic-ros-type-introspection net-tools \
    && rm -rf /var/lib/apt/lists/*

# setup environment
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

ENV ROS_DISTRO melodic

# install ros packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-melodic-ros-core=1.4.1-0* ros-melodic-cv-bridge \
    && rm -rf /var/lib/apt/lists/*

# setup entrypoint
COPY ./docker/ros_entrypoint.sh /
RUN chmod +x ros_entrypoint.sh
ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]

RUN apt-get update \
 && apt-get install -y vim wget tmux less htop python-pip python-tk\
 libsm6 libxext6 libxrender-dev \
 && apt-get clean

RUN mkdir -p /etc/OpenCL/vendors && \
    echo "libnvidia-opencl.so.1" > /etc/OpenCL/vendors/nvidia.icd

#COPY firmwaresubt /px4/firmwaresubt
#COPY ws/src/mavros/mavros/scripts/install_geographiclib_datasets.sh px4/install_geographiclib_datasets.sh
#COPY firmwaresubt /px4/ws/src/firmwaresubt
#RUN /bin/bash -c 'cd /px4; ./install_geographiclib_datasets.sh; cd /px4/firmwaresubt/; ./build.sh;'
#RUN /bin/bash -c '. /opt/ros/melodic/setup.bash; cd /px4/ws; catkin build'

COPY ws/src/mavros/mavros/scripts/install_geographiclib_datasets.sh px4/install_geographiclib_datasets.sh
RUN /bin/bash -c 'cd /px4; ./install_geographiclib_datasets.sh;'
RUN /bin/bash -c 'git config --global --add safe.directory /ws/src/firmwaresubt;'

#RUN /bin/bash -c 'rm -rf /home/ws/src/firmwaresubt/;'
#COPY firmwaresubt /home/ws/src/firmwaresubt
#RUN /bin/bash -c 'cd /home/ws/src/mavros/mavros/scripts; ./install_geographiclib_datasets.sh; cd ../../../firmwaresubt/; ./build.sh;'

#RUN /bin/bash -c '. /opt/ros/melodic/setup.bash; cd /home/ws; catkin build'
