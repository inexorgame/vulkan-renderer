.. _ENGINE_PARALLELIZATION_STRATEGY:

Engine Parallelization Strategy
===============================

Modern Vulkan engines benefit from parallelism on several levels.

CPU Parallelization
-------------------

The CPU should be kept busy with work that can happen independently of the main render submission path. Typical examples are:

- gameplay logic
- physics
- animation updates
- visibility checks and culling
- asset streaming and loading

This kind of task-based parallelization is usually implemented with a thread pool or job system.

Command Buffer Parallelization
------------------------------

One of Vulkan's biggest advantages is that command buffers can be recorded in parallel.
Different worker threads can prepare work for different views, render passes, or object groups at the same time.

Frame Pipelining
----------------

Multiple frames can be processed in flight at once. While the GPU renders one frame, the CPU can already prepare the next one and update simulation data for a later frame.

Queue Parallelization
---------------------

Vulkan makes it possible to overlap graphics, compute, and transfer work across separate queues when the hardware supports it.
This can be used for asynchronous compute and resource uploads.

Data Parallelism
----------------

Many engine workloads are naturally data parallel, for example particles, instancing, skinning, tiled lighting, or post-processing.
These workloads are well suited for SIMD-friendly CPU code or GPU compute shaders.

In practice, a Vulkan engine usually combines all of these layers:

- CPU task parallelism for simulation and content processing
- parallel command buffer recording for rendering
- frame pipelining for steady throughput
- queue overlap for GPU efficiency
- data parallel workloads for large batches of similar work
