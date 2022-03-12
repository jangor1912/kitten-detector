FROM nvcr.io/nvidia/deepstream:6.0.1-devel

# dependecies of Clion docker integration
RUN apt-get update \
  && apt-get install -y ssh \
    build-essential \
    gcc \
    g++ \
    gdb \
    clang \
    cmake \
    rsync \
    tar \
    python \
  && apt-get clean

RUN useradd -m user && yes password | passwd user



COPY . /project

WORKDIR /project
