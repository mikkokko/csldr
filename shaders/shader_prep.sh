#!/bin/bash

echo -e "#version 110\n" > studio_cpu.vert
cat studio.vert >> studio_cpu.vert

echo -e "#version 120\n\n#define GPU_SKINNING\n" > studio_gpu.vert
cat studio.vert >> studio_gpu.vert

xxd -i studio.frag studio_frag.h
xxd -i studio_cpu.vert studio_cpu_vert.h
xxd -i studio_gpu.vert studio_gpu_vert.h

#rm studio_cpu.vert
#rm studio_gpu.vert
