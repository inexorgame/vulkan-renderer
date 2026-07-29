FROM mcr.microsoft.com/devcontainers/cpp:ubuntu

ARG VULKAN_VERSION=1.4.357.0
ARG VULKAN_SDK_CHECKSUM="0f09bf6a0625e346bf004be70b92907e934a4c76606b323441b2baf3a5a0e66d"
ARG LLVM_VERSION=22
ARG GCC_VERSION=15

ENV VULKAN_SDK_PATH=/inexor-vulkan-sdk

# Update dependencies
RUN apt update -qq && \
    # Install build tools
    apt-get install -y \
        gcc-${GCC_VERSION} \
        g++-${GCC_VERSION} \
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
        xkb-data \
        xorg-dev \
        # doxygen for documentation
        doxygen \
        # clang installation tools
        lsb-release \
        wget \
        software-properties-common \
        gnupg && \
    pip3 install --break-system-packages wheel setuptools && \
    # install clang
    wget -O llvm.sh https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    bash llvm.sh ${LLVM_VERSION} all && \
    rm llvm.sh

# Install Vulkan SDK
RUN mkdir -p ${VULKAN_SDK_PATH} && \
    wget -O vulkansdk.tar.xz https://sdk.lunarg.com/sdk/download/${VULKAN_VERSION}/linux/vulkan-sdk.tar.xz && \
    echo "${VULKAN_SDK_CHECKSUM} vulkansdk.tar.xz" | sha256sum --check && \
    tar -xJf vulkansdk.tar.xz -C ${VULKAN_SDK_PATH} && \
    rm vulkansdk.tar.xz && \
    # runtime dependencies
    apt-get -y install qtbase5-dev libxcb-xinput0 libxcb-xinerama0

# Vulkan environment variables
ENV VULKAN_SDK=${VULKAN_SDK_PATH}/${VULKAN_VERSION}/x86_64
ENV LD_LIBRARY_PATH=$VULKAN_SDK/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
ENV VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
ENV PATH="${VULKAN_SDK}/bin:/usr/lib/llvm-${LLVM_VERSION}/bin:${PATH}"
