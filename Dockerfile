FROM debian:bookworm-slim

RUN apt-get update && apt-get upgrade -y

ADD https://github.com/wiringpi/wiringpi/releases/download/3.10/wiringpi_3.10_arm64.deb /tmp

# APT install (base) packages
RUN apt-get install -y \
        build-essential \
        cmake \
        git \
        /tmp/wiringpi_3.10_arm64.deb


#Dependencies for SFML 
RUN apt install -y \
        libxrandr-dev \
        libxcursor-dev \
        libudev-dev \
        libopenal-dev \
        libflac-dev \
        libvorbis-dev \
        libgl1-mesa-dev \
        libegl1-mesa-dev \
        libdrm-dev \
        libgbm-dev \
        x11-xserver-utils

