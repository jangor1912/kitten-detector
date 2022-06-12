# Pipeline CLI entrypoint

## Compilation
```bash
gcc -Wall -L/usr/local/lib -Wl,-rpath=/usr/local/lib -o pipeline_cli main.c -lkitten-detector 
```

## Running
```bash
./pipeline_cli 1024 1024 1 file:///data/20200820_191031.mp4
```