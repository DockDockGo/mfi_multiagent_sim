#!/bin/bash

docker network create --driver=bridge --subnet=172.20.20.0/24 --ip-range=172.20.20.0/24 mfi_multi_agent_network

#git submodule update --init
#git clone git@bitbucket.org:castacks/firmwaresubt.git
#cd firmwaresubt
#git checkout 0871bea
#cd ../
mv ws/src temporary_location_for_src/
sudo rm -rf ws/*
mv temporary_location_for_src/ ws/src
docker build --no-cache --network=host -t mfi_multi_agent -f docker/MFIMultiAgent.dockerfile .

#exit
xhost +

XAUTH=/tmp/.docker.xauth
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

echo -e "roscore &\nmon launch core_central sim_main.launch\n" > extras/.bash_history

echo "Preparing Xauthority data..."
xauth_list=$(xauth nlist :0 | tail -n 1 | sed -e 's/^..../ffff/')
if [ ! -f $XAUTH ]; then
    if [ ! -z "$xauth_list" ]; then
        echo $xauth_list | xauth -f $XAUTH nmerge -
    else
        touch $XAUTH
    fi
    chmod a+r $XAUTH
fi

echo "Done."
echo ""
echo "Verifying file contents:"
file $XAUTH
echo "--> It should say \"X11 Xauthority data\"."
echo ""
echo "Permissions:"
ls -FAlh $XAUTH
echo ""
echo "Running docker..."

docker run -it \
    --env="DISPLAY=$DISPLAY" \
    --env="QT_X11_NO_MITSHM=1" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --env="XAUTHORITY=$XAUTH" \
    --volume="$XAUTH:$XAUTH" \
    --hostname=mfi \
    --volume $SCRIPT_DIR/extras/.bashrc:/root/.bashrc \
    --volume $SCRIPT_DIR/extras/init.el:/root/.emacs.id/init.el \
    --volume $SCRIPT_DIR/extras/inputrc:/etc/inputrc \
    --volume $SCRIPT_DIR/ws:/ws \
    --privileged \
    --runtime=nvidia --env=NVIDIA_VISIBLE_DEVICES=all --env=NVIDIA_DRIVER_CAPABILITIES=all --env=DISPLAY --env=QT_X11_NO_MITSHM=1 --gpus 1 \
    mfi_multi_agent:latest \
    /bin/bash -c "cd /ws; colcon build --symlink-install"

echo "Done."
