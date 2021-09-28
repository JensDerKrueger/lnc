herein are some templates meant to be copied to the VS141/props-folder\
the props-folder itself is mentioned in .gitignore so its contents\
can be customized to the local environment without being affected by git merges or something.\
\
vs_defaults.props :\
sets the bin and tmp-folder per solution.\
sets the location of the vs-lib and vs-include folder.\
every binary will be placed into the bin-folder,\
this also includes static libs. so the bin-folder got included as librarypath as well.\
Another defaultsetting is the c++-version you intend to use, and the conformance-mode.\
this depends on the visualstudio-version (or msbuild-environment) installed on your system.\
result of a build will have tmp and bin. to clean up, tmp can be deleted safely.\
![image](BuildFolders.JPG)
\
vs_cuda.props :\
contains opencl include and lib-directory.\
allows the "#include <cl/opencl.h>"\
current default assumes the nvidia cuda sdk which sets %CUDA_PATH%-environmentvariable.\
Alternative would be lib and include-folder of the khronos opencl-sdk found on github. would be another propsfile\
(you'll have to run cmake and build the libs yourself when using khronos)