FROM ubuntu:rolling

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc-14 \
    g++-14 \
    clang-18 \
    cmake \
    curl \
    git \
    libgl1-mesa-dev \
    libwayland-dev \
    libx11-dev \
    libx11-xcb-dev \
    libxkbcommon-dev \
    libxcb-dri3-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-render0-dev \
    libxcb-shape0-dev \
    libxcb-sync-dev \
    libxcb-util-dev \
    libxcb-xfixes0-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    lld \
    ninja-build \
    pkg-config \
    python3 \
    python3-pip \
    qtbase5-dev \
    libxcb-xinput0 \
    libxcb-xinerama0 \
    xkb-data \
    xorg-dev && \
    pip3 install --break-system-packages wheel setuptools && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /project
