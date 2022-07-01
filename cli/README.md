# Pipeline CLI entrypoint

## Compilation
```bash
gcc \
  -Wall -L/usr/local/lib \
  -Wl,-rpath=/usr/local/lib \
  -o pipeline_cli main.c -lkitten-detector 
```

## Running
```bash
./pipeline_cli \
  1920 1080 1 \
  rtsp://192.168.0.115:8080/h264_pcm.sdp
```

or

```bash
./pipeline_cli \
  1920 1080 1 \
  file:///opt/nvidia/deepstream/deepstream/samples/streams/sample_1080p_h264.mp4
```