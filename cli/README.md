# Pipeline CLI entrypoint

## Compilation
```bash
gcc \
-Wall \
-o pipeline_cli main.c \
-L/usr/local/lib \
-Wl,-rpath=/usr/local/lib \
-lnvdsgst_meta -lnvds_meta \
-L/opt/nvidia/deepstream/deepstream/lib/ \
-Wl,-rpath=/opt/nvidia/deepstream/deepstream/lib/ \
-lkitten-detector \
-Wl,--as-needed
```

```bash
gcc \
  -Wall \
  -o pipeline_cli main.c \
  -L/opt/nvidia/deepstream/deepstream/lib/ \
  -Wl,-rpath=/opt/nvidia/deepstream/deepstream/lib/ \
  -lnvdsgst_meta -lnvds_meta \
  -L/usr/local/lib \
  -Wl,-rpath=/usr/local/lib \
  -lkitten-detector
```

```bash
gcc \
  -Wall \
  -L/usr/local/lib \
  -Wl,-rpath=/usr/local/lib \
  -L/opt/nvidia/deepstream/deepstream-6.0/lib/ \
  -Wl,-rpath=/opt/nvidia/deepstream/deepstream-6.0/lib/ \
  -o pipeline_cli main.c -lkitten-detector 
```

## Running
```bash
./kitten-detector-cli \
  1920 1080 1 \
  rtsp://192.168.0.116:8080/h264_pcm.sdp
```

or

```bash
./kitten-detector-cli \
  1920 1080 1 \
  file:///opt/nvidia/deepstream/deepstream/samples/streams/sample_1080p_h264.mp4
```