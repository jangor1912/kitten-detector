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
  -dit --rm \
  -v /home/jan/Projects/kitten-detector/output/:/project/output \
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
rm -rf build
mkdir build
cd build
cmake ..
make
make install
cd ..
```

## Checking if rtsp source works

### With common rtsp-source
```bash
gst-launch-1.0 -e \
  rtspsrc location="rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4" \
  ! queue ! rtph264depay ! nvv4l2decoder ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' \
  ! queue ! nvv4l2h264enc ! h264parse \
  ! splitmuxsink location="/output/video-%02d.mp4" max-size-time=10000000000
```

### With smartphone rtsp-source
```bash
gst-launch-1.0 -e \
  rtspsrc location="rtsp://192.168.0.115:1935/h264_pcm.sdp" \
  ! queue ! rtph264depay ! nvv4l2decoder ! nvvideoconvert ! 'video/x-raw(memory:NVMM),format=NV12' \
  ! queue ! nvv4l2h264enc ! h264parse \
  ! splitmuxsink location="/output/video-%02d.mp4" max-size-time=10000000000
```

### Simple rtsp capture
```bash
gst-launch-1.0 -e \
  rtspsrc location="rtsp://admin:adimn@192.168.0.115:1935" \
  ! queue ! rtph264depay ! h264parse \
  ! splitmuxsink location="/output/video-%02d.mp4" max-size-time=10000000000
```

### With uridecodebin
```bash
gst-launch-1.0 -e \
  uridecodebin uri="rtsp://192.168.0.115:8080/h264_pcm.sdp" \
  ! queue ! 'video/x-raw(memory:NVMM),format=NV12' \
  ! queue ! nvv4l2h264enc ! h264parse \
  ! splitmuxsink location="/output/video-%02d.mp4" max-size-time=10000000000
```