# Pipeline CLI entrypoint

## Compilation
```bash
gcc -Wall -L/usr/local/lib -Wl,-rpath=/usr/local/lib -o pipeline_cli main.c -lkitten-detector 
```