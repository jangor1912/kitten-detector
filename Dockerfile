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

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]