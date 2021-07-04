# How to compile

## Mac

All you need to compile the projects an a mac is to download [xcode](https://apps.apple.com/de/app/xcode/id497799835?mt=12). You can either use make to build the projects from the command line or open the xcode-projects when available.

## Windows

## Linux

You need the following packages in order to compile most of the projects on a Debian based Distribution.

```bash
sudo apt install -y libglu1-mesa-dev freeglut3-dev mesa-common-dev xorg-dev mesa-utils libglfw3-dev libglew-dev libomp-dev
```

You need furthermore those packages

```bash
sudo apt install ocl-icd-opencl-dev ocl-icd-libopencl1 ocl-icd-dev opencl-headers clinfo
```

and some hardware specific packages to get OpenCL up and running on your system (use `clinfo -l` to see if your hardware gets recognized).

Here are some sources on where to start to look in order to get the necessary packages/drivers.

| Hardware | |
| :--- | :--- |
| Intel | [intel/compute-runtime](https://github.com/intel/compute-runtime) |
| NVIDIA | ... |
| AMD | ... |

To compile the projects move to one of the `<num>_<name>` directories and use the command `make release`.
To compile all at at once move to `lnc/cpp/OpenGL` and use `make release -j` to build all projects in parallel.
