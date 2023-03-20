FROM mcr.microsoft.com/devcontainers/base:ubuntu-22.04

# Update package lists
RUN apt update -qq \
    # Install build tools
    && apt install -y \
        clang-14 \
        clang-format \
        cmake \
        curl \
        doxygen \
        git \
        libgl1-mesa-dev \
        libx11-dev \
        libx11-xcb-dev \
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
        python3-venv \
        wget \
        xkb-data \
        xorg-dev \
    && pip3 install wheel setuptools \
    && pip3 install conan mako
