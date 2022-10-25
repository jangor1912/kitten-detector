FROM nvcr.io/nvidia/deepstream:6.0.1-devel

# dependecies of Clion docker integration
# Stolen from https://github.com/JetBrains/clion-remote/blob/master/Dockerfile.remote-cpp-env
# CLion remote docker environment (How to build docker container, run and stop it)
#
# Build and run:
#   docker build -t deepstream-clion/remote-c-env:0.5 -f Dockerfile .
#   docker run -d --cap-add sys_ptrace -p127.0.0.1:2222:22 --name deepstream_clion_remote_env deepstream-clion/remote-c-env:0.5
#   ssh-keygen -f "$HOME/.ssh/known_hosts" -R "[localhost]:2222"
#
# stop:
#   docker stop deepstream_clion_remote_env
#
# ssh credentials (test user):
#   user@password

# Key rotation - https://forums.developer.nvidia.com/t/gpg-error-http-developer-download-nvidia-com-compute-cuda-repos-ubuntu1804-x86-64/212904/3
RUN apt-key del 7fa2af80
RUN apt-key adv --fetch-keys http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1604/x86_64/3bf863cc.pub

RUN DEBIAN_FRONTEND="noninteractive" apt-get update && apt-get -y install tzdata

RUN apt-get update \
  && apt-get install -y ssh \
      build-essential \
      gcc \
      g++ \
      gdb \
      clang \
      make \
      ninja-build \
      cmake \
      autoconf \
      automake \
      locales-all \
      dos2unix \
      rsync \
      tar \
      python \
  && apt-get clean

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/lib/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion \
  && mkdir /run/sshd

RUN useradd -m user \
  && yes password | passwd user

RUN usermod -s /bin/bash user

COPY . /project

WORKDIR /project

RUN mkdir -p /models/configs/infer-engine && \
    mkdir -p /models/configs/tracker && \
    cp /project/configs/infer-engine/pgie_config.txt /models/configs/infer-engine && \
    cp /project/configs/tracker/config_tracker_NvDCF_accuracy.yml /models/configs/tracker

RUN mkdir -p /project/build && \
    cd /project/build && \
    cmake .. && \
    make && \
    make install

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]