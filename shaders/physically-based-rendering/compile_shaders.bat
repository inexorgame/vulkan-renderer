glslangValidator.exe -V filtercube.vert -o filtercube.vert.spv

glslangValidator.exe -V genbrdflut.vert -o genbrdflut.vert.spv

glslangValidator.exe -V pbr.vert -o pbr.vert.spv

glslangValidator.exe -V genbrdflut.frag -o genbrdflut.frag.spv

glslangValidator.exe -V irradiancecube.frag -o irradiancecube.frag.spv

glslangValidator.exe -V pbr_khr.frag -o pbr_khr.frag.spv

glslangValidator.exe -V prefilterenvmap.frag -o prefilterenvmap.frag.spv

pause
