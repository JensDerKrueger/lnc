#pragma once

#define CL_HPP_MINIMUM_OPENCL_VERSION 110
#define CL_HPP_TARGET_OPENCL_VERSION 110
#define CL_TARGET_OPENCL_VERSION 110

#define CL_VERSION_1_2

#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include "CL\opencl.h"
    #include "CL\cl.hpp"
#endif

#include <exception>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

class OpenClContextException : public std::exception {
public:
  OpenClContextException(const std::string& m);
  OpenClContextException(const std::string& m, cl_int error);
  ~OpenClContextException() throw();
  const char* what() const throw();

private:
  std::string msg;

  static std::string getErrorString(cl_int error);
};

template <typename T> struct DynBuffer {
  T* data{NULL};
  size_t currentSize{0};
  
  DynBuffer(size_t initialSize) :
    currentSize{initialSize}
  {
    data = new T[currentSize];
  }
  
  void ensureSize(size_t minSize) {
    if (minSize < currentSize) return;
    currentSize = minSize;
    delete [] data;
    data = new T[currentSize];
  }
  
  ~DynBuffer() {
    delete [] data;
  }
};

struct DeviceInfo {
  cl_device_id deviceID;
  std::string name{""};
  cl_device_type type{CL_DEVICE_TYPE_DEFAULT};
  std::string deviceVersion{""};
  std::string driverVersion{""};
  std::string openCLCVersion{""};
  cl_uint maxComputeUnits{0};
  
  DeviceInfo(cl_device_id deviceID);
  std::string toString() const;
};

struct PlatformInfo {
  cl_platform_id platformID;
  std::string name{""};
  std::string vendor{""};
  std::string version{""};  
  std::vector<DeviceInfo> devices;
  
  PlatformInfo(cl_platform_id platformID);
  std::string toString() const;
};

template <typename T> class OpenClContext {
public:
  OpenClContext(unsigned int w, unsigned int h)
  : width(w)
  , height(h)
  , bHasInput(true)
  , num_devices(0)
  , context(0)
  , commands(0)
  , program(0)
  , kernel(0)
  , input(0)
  , output(0)
  {
  }
  
  void init(bool useGPU=true, size_t deviceIndex=0);
  void init(cl_device_id deviceID);
  cl_uint getNumDevices() const {return num_devices;}
  void destroy();
  void setProgramCode(const std::string& code,
                      bool bHasInput = true,
                      const std::string& mainMethod="clmain");
  void setProgramFile(const std::string& filename,
                      bool bHasInput = true,
                      const std::string& mainMethod="clmain");
  
  void setInput(T* data);
  void run(size_t ySize = 16);
  void getOutput(T* results);
  void setParam(cl_uint arg_index, size_t arg_size, const void *arg_value);
  
  static std::vector<PlatformInfo> getInfo() {
    std::vector<PlatformInfo> result;
    cl_uint count;
    clGetPlatformIDs(0, NULL, &count);
    DynBuffer<cl_platform_id> platformBuffer{count};
    clGetPlatformIDs(cl_uint(platformBuffer.currentSize), platformBuffer.data, NULL);
    for (cl_uint i = 0;i<count;++i) {
      result.push_back(PlatformInfo(platformBuffer.data[i]));
    }
    return result;
  }

private:
  unsigned int width;
  unsigned int height;
  bool bHasInput;
  
  cl_uint num_devices;
  cl_device_id deviceID;              // computed device id
  cl_context context;                 // compute context
  cl_command_queue commands;          // compute command queue
  cl_program program;                 // compute program
  cl_kernel kernel;                   // compute kernel
  
  cl_mem input;                       // device memory used for the input array
  cl_mem output;                      // device memory used for the output array
  
};

template <typename T>
void OpenClContext<T>::init(bool useGPU, size_t deviceIndex) {
  cl_int err;                            // error code returned from api calls
    
  // Connect to a compute device
  //
  cl_device_id deviceIDs[16];
  err = clGetDeviceIDs(NULL, useGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU,
                       16, deviceIDs, &num_devices);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to create a device group!",
                                         err);
  }
  deviceID = deviceIDs[deviceIndex];

  // Create a compute context
  //
  context = clCreateContext(0, 1, &deviceID, NULL, NULL, &err);
  if (!context) {
    throw OpenClContextException("Failed to create a compute context!",
                                         err);
  }
  
  
  // Create a command commands
  //
  commands = clCreateCommandQueue(context, deviceID, 0, &err);
  if (!commands) {
    throw OpenClContextException("Failed to create a command commands!",
                                         err);
  }
}

template <typename T>
void OpenClContext<T>::init(cl_device_id deviceID) {
  cl_int err;                            // error code returned from api calls
    
  // Create a compute context
  //
  context = clCreateContext(0, 1, &deviceID, NULL, NULL, &err);
  if (!context) {
    throw OpenClContextException("Failed to create a compute context!",
                                         err);
  }
    
  // Create a command commands
  //
  commands = clCreateCommandQueue(context, deviceID, 0, &err);
  if (!commands) {
    throw OpenClContextException("Failed to create a command commands!",
                                         err);
  }
  this->deviceID = deviceID;
}

template <typename T>
void OpenClContext<T>::setProgramFile(const std::string& filename,
                                      bool bHasInput,
                                      const std::string& mainMethod) {
  std::ifstream infile(filename);
  
  if (!infile) {
    throw OpenClContextException("Failed to open program");
  }
  
  std::string line, code;
  
  while (std::getline(infile, line)) {
    code += line + "\n";
  }
  
  setProgramCode(code, bHasInput, mainMethod);
}


template <typename T>
void OpenClContext<T>::setProgramCode(const std::string& code,
                                      bool bHasInput,
                                      const std::string& mainMethod) {
  cl_int err;                            // error code returned from api calls

  this->bHasInput = bHasInput;

  // Create the compute program from the source buffer
  //
  const char* codeArray[1];
  codeArray[0] = code.c_str();
  program = clCreateProgramWithSource(context, 1,
                                      codeArray,
                                      NULL, &err);
  if (!program) {
    throw OpenClContextException("Failed to create compute program!",
                                         err);
  }
  
  // Build the program executable
  //
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];
    clGetProgramBuildInfo(program, deviceID,
                          CL_PROGRAM_BUILD_LOG,
                          sizeof(buffer), buffer, &len);
    
    throw OpenClContextException(std::string("Failed to build "
                                                     "program executable: ")+
                                         std::string(buffer), err);
  }
  
  // Create the compute kernel in the program we wish to run
  //
  kernel = clCreateKernel(program, mainMethod.c_str(), &err);
  if (!kernel || err != CL_SUCCESS) {
    throw OpenClContextException("Failed to create compute kernel!",
                                         err);
  }
  
  // Set the arguments to our compute kernel
  //
  err = 0;
  cl_uint iParam = 0;

  if (bHasInput) {
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,
                           sizeof(float) * width*height, NULL, NULL);
    if (!input)
      throw OpenClContextException("Failed to allocate device memory!",
                                   err);
    err  = clSetKernelArg(kernel, iParam++, sizeof(cl_mem), &input);
    if (err != CL_SUCCESS) {
      throw OpenClContextException("Failed to set kernel arguments", err);
    }
  }

  output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                          sizeof(float) * width*height, NULL, NULL);
  if (!output)
    throw OpenClContextException("Failed to allocate device memory!",
                                 err);

  err |= clSetKernelArg(kernel, iParam++, sizeof(cl_mem), &output);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to set kernel arguments", err);
  }

}

template <typename T>
void OpenClContext<T>::setParam(cl_uint arg_index,  size_t arg_size,
                                const void *arg_value) {
  cl_int err;                            // error code returned from api calls
  err = clSetKernelArg(kernel, arg_index+(bHasInput ? 2 : 1),
                       arg_size, arg_value);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to set kernel arguments", err);
  }
}

template <typename T>
void OpenClContext<T>::setInput(T* data) {
  cl_int err;                            // error code returned from api calls

  if (!input) {
    throw OpenClContextException("Input does not exist (yet)!");
  }

  // Write our data set into the input array in device memory
  //
  err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0,
                             sizeof(T) * width*height, data, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to write to source array!",
                                         err);
  }
  
}

template <typename T>
void OpenClContext<T>::run(size_t ySize) {
  cl_int err;                            // error code returned from api calls
  size_t local;                       // local domain size for our calculation
  
  // Get the maximum work group size for executing the kernel on the device
  //
  err = clGetKernelWorkGroupInfo(kernel, deviceID,
                                 CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(local), &local, NULL);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to retrieve "
                                 "kernel work group info!", err);
  }

  // Execute the kernel over the entire range of our input data set
  // using the maximum number of work group items for this device
  size_t global[] = {(size_t)width, (size_t)height};
  size_t localArray[] = {local/ySize, ySize};
  err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL,
                               global, localArray, 0, NULL, NULL);
  if (err) {
    // on some systems the reportet CL_KERNEL_WORK_GROUP_SIZE
    // does not work but a 1x1 kernel works, so try that before
    // giving up
    localArray[0] = 1;
    localArray[1] = 1;
    err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL,
                                 global, {localArray}, 0, NULL, NULL);
    if (err) {
      throw OpenClContextException("Failed to execute kernel!", err);
    }
  }
  
}

template <typename T>
void OpenClContext<T>::getOutput(T* results) {
  cl_int err;                            // error code returned from api calls
  
  // Wait for the command commands to get serviced before reading back results
  //
  clFinish(commands);
  
  // Read back the results from the device
  err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0,
                            sizeof(T) * width*height, results, 0, NULL, NULL );
  if (err != CL_SUCCESS)
  {
    throw OpenClContextException("Failed to read output array!", err);
  }
}

template <typename T>
void OpenClContext<T>::destroy() {
  clReleaseMemObject(input);
  clReleaseMemObject(output);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);
}
