#!/bin/bash

HOSTNAME="mfi"

XAUTH=/tmp/.docker.xauth
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

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
    --hostname $HOSTNAME \
    --volume $SCRIPT_DIR/extras/.bashrc:/root/.bashrc \
    --volume $SCRIPT_DIR/extras/init.el:/root/.emacs.id/init.el \
    --volume $SCRIPT_DIR/extras/inputrc:/etc/inputrc \
    --volume $SCRIPT_DIR/extras/ddg_scripts:/root/scripts \
    --volume $SCRIPT_DIR/ws:/ws \
    --privileged \
    --runtime=nvidia --env=NVIDIA_VISIBLE_DEVICES=all --env=NVIDIA_DRIVER_CAPABILITIES=all --env=DISPLAY --env=QT_X11_NO_MITSHM=1 --gpus 1 \
    mfi_multi_agent:latest \
    /bin/bash -c "cd /root/scripts; . multi_robot_launch.sh"

    # --net mfi_multi_agent_network \


echo "Done."
