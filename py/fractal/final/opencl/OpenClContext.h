/*************************************
*        Quick und Dirty             *
*       OpenCL Fractal Test          *
*                                    *
*        (c) 2016 Jens Krüger        *
*  This code is in the public domain *
**************************************/

#pragma once

#include <OpenCL/opencl.h>
#include <exception>
#include <string>
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

template <typename T> class OpenClContext {
public:
  OpenClContext(unsigned int w, unsigned int h)
  : width(w)
  , height(h)
  , bHasInput(true)
  , num_devices(0)
  , deviceIndex(0)
  , context(0)
  , commands(0)
  , program(0)
  , kernel(0)
  , input(0)
  , output(0)
  {
  }
  
  void init(bool useGPU=true, size_t deviceIndex=0);
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
  
private:
  unsigned int width;
  unsigned int height;
  bool bHasInput;
  
  cl_uint num_devices;
  cl_uint deviceIndex;
  cl_device_id device_ids[16];        // compute device id
  cl_context context;                 // compute context
  cl_command_queue commands;          // compute command queue
  cl_program program;                 // compute program
  cl_kernel kernel;                   // compute kernel
  
  cl_mem input;                       // device memory used for the input array
  cl_mem output;                      // device memory used for the output array
  
};

template <typename T>
void OpenClContext<T>::init(bool useGPU, size_t deviceIndex) {
  int err;                            // error code returned from api calls
  
  this->deviceIndex = deviceIndex;
  
  // Connect to a compute device
  //
  err = clGetDeviceIDs(NULL, useGPU ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU,
                       16, device_ids, &num_devices);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to create a device group!",
                                         err);
  }
  
  // Create a compute context
  //
  context = clCreateContext(0, 1, &(device_ids[deviceIndex]), NULL, NULL, &err);
  if (!context) {
    throw OpenClContextException("Failed to create a compute context!",
                                         err);
  }
  
  // Create a command commands
  //
  commands = clCreateCommandQueue(context, device_ids[deviceIndex], 0, &err);
  if (!commands) {
    throw OpenClContextException("Failed to create a command commands!",
                                         err);
  }
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
  int err;                            // error code returned from api calls

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
    clGetProgramBuildInfo(program, device_ids[deviceIndex],
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

  int iParam = 0;

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
  int err;                            // error code returned from api calls
  err = clSetKernelArg(kernel, arg_index+(bHasInput ? 2 : 1),
                       arg_size, arg_value);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to set kernel arguments", err);
  }
}

template <typename T>
void OpenClContext<T>::setInput(T* data) {
  int err;                            // error code returned from api calls

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
  int err;                            // error code returned from api calls
  size_t local;                       // local domain size for our calculation
  
  // Get the maximum work group size for executing the kernel on the device
  //
  err = clGetKernelWorkGroupInfo(kernel, device_ids[deviceIndex],
                                 CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(local), &local, NULL);
  if (err != CL_SUCCESS) {
    throw OpenClContextException("Failed to retrieve "
                                         "kernel work group info!", err);
  }
  
  
  // Execute the kernel over the entire range of our input data set
  // using the maximum number of work group items for this device
  //
  size_t global[] = {(size_t)width, (size_t)height};

  size_t localArray[] = {local/ySize, ySize};
  err = clEnqueueNDRangeKernel(commands, kernel, 2, NULL,
                               global, localArray, 0, NULL, NULL);
  if (err) {
    std::cout << err << std::endl;

    throw OpenClContextException("Failed to execute kernel!", err);
  }
}

template <typename T>
void OpenClContext<T>::getOutput(T* results) {
  int err;                            // error code returned from api calls
  
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