# kitten-detector
The purpose of this repository is to show the capabilities of Deepstream

## Docker container

To build docker container run:
```bash
docker build -t clion/remote-c-env:0.5 -f Dockerfile .
```

To run the container user must invoke following command:

```bash
docker run \
  --runtime nvidia \
  -dit --rm \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v ~/Downloads:/data \
  -e DISPLAY=$DISPLAY \
  -w /opt/nvidia/deepstream/deepstream-6.0 \
  --cap-add sys_ptrace -p127.0.0.1:2222:22 \
  --name kitten-detector \
  clion/remote-c-env:0.5
```

To properly display video on current display user must invoke on host machine:
```bash
xhost +
```


## Installing library
```bash
mkdir build
cd build
cmake ..
make install
```