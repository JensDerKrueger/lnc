/*
 For more information, please see: http://software.sci.utah.edu
 
 The MIT License
 
 Copyright (c) 2008 Scientific Computing and Imaging Institute,
 University of Utah.
 
 
 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 */

/**
 \file    SysTools.h
 \brief    Simple routines for filename handling
 \author    Jens Krueger, IVDA, SCI, Intel VCI
 */

#pragma once

#ifndef IVDA_SYSTOOLS
#define IVDA_SYSTOOLS

#include "StdDefines.h"

#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOGDI
#define NOGDI
#endif

#ifndef _WINDOWS_
  #include <Tools/Sockets.h>
  #define NOMINMAX
  #include <windows.h>
#endif
#include <direct.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#else
#include <wchar.h>
typedef wchar_t WCHAR;
typedef unsigned char CHAR;
#include <unistd.h>
#include <sys/stat.h>
#endif

namespace IVDA
{
	namespace SysTools {
	  template <typename T> std::string ToString(const T& aValue)
	  {
		std::stringstream ss;
		ss << aValue;
		return ss.str();
	  }
  
	  template <typename T> std::wstring ToWString(const T& aValue)
	  {
		std::wstringstream ss;
		ss << aValue;
		return ss.str();
	  }
  
	  template <typename T> bool FromString(T& t, const std::string& s,
											std::ios_base& (*f)(std::ios_base&) = std::dec)
	  {
		std::istringstream iss(s);
		return !(iss >> f >> t).fail();
	  }
  
  
	  template <typename T> bool FromString(T& t, const std::wstring& s,
											std::ios_base& (*f)(std::ios_base&) = std::dec)
	  {
		std::wistringstream iss(s);
		return !(iss >> f >> t).fail();
	  }
  
	  template <typename T> T FromString(const std::string& s,
										 std::ios_base& (*f)(std::ios_base&) = std::dec)
	  {
		T t = T(0);
		std::istringstream iss(s);
		iss >> f >> t;
		return t;
	  }
  
	  template <typename T> T FromString(const std::wstring& s,
										 std::ios_base& (*f)(std::ios_base&) = std::dec)
	  {
		T t = T(0);
		std::wistringstream iss(s);
		iss >> f >> t;
		return t;
	  }
  
	  std::wstring ToLowerCase(const std::wstring& str);
	  std::string ToLowerCase(const std::string& str);
	  std::wstring ToUpperCase(const std::wstring& str);
	  std::string ToUpperCase(const std::string& str);
  
	  enum EProtectMode {
		PM_NONE,
		PM_BRACKETS,
		PM_QUOTES,
    PM_CUSTOM_DELIMITER
	  };
  
	  std::vector< std::string > Tokenize(const std::string& strInput, EProtectMode mode = PM_QUOTES, char customDelimiter=' ');
	  std::vector< std::wstring > Tokenize(const std::wstring& strInput, EProtectMode mode = PM_QUOTES,  wchar_t customDelimiter=' ');
  
	#ifndef NO_MAC_RESOURCE_LOADER	
	  std::string GetFromResourceOnMac(const std::string& fileName);
	  std::wstring GetFromResourceOnMac(const std::wstring& fileName);
	#endif
	
    bool IsDirectory(const std::string& pathName);
    bool IsDirectory(const std::wstring& pathName);

    bool IsFile(const std::string& fileName, bool isRegularFile=true);
    bool IsFile(const std::wstring& fileName, bool isRegularFile=true);

	  bool FileExists(const std::string& pathOrFileName);
	  bool FileExists(const std::wstring& pathOrFileName);
  
	  std::string GetExt(const std::string& fileName);
	  std::wstring GetExt(const std::wstring& fileName);
  
	  std::string GetPath(const std::string& fileName);
	  std::wstring GetPath(const std::wstring& fileName);
  
	  std::string GetFilename(const std::string& fileName);
	  std::wstring GetFilename(const std::wstring& fileName);
  
	  std::string FindPath(const std::string& fileName, const std::string& path);
	  std::wstring FindPath(const std::wstring& fileName,
							const std::wstring& path);
  
	  std::string  RemoveExt(const std::string& fileName);
	  std::wstring RemoveExt(const std::wstring& fileName);
  
	  std::string  CheckExt(const std::string& fileName,
							const std::string& newext);
	  std::wstring CheckExt(const std::wstring& fileName,
							const std::wstring& newext);
  
	  std::string  ChangeExt(const std::string& fileName,
							 const std::string& newext);
	  std::wstring ChangeExt(const std::wstring& fileName,
							 const std::wstring& newext);
  
	  std::string  AppendFilename(const std::string& fileName,
								  const int iTag);
	  std::wstring AppendFilename(const std::wstring& fileName,
								  const int iTag);
	  std::string  AppendFilename(const std::string& fileName,
								  const std::string& tag);
	  std::wstring AppendFilename(const std::wstring& fileName,
								  const std::wstring& tag);
  
	  std::string  FindNextSequenceName(const std::string& strFilename);
	  std::wstring FindNextSequenceName(const std::wstring& wStrFilename);
  
	  std::string  FindNextSequenceName(const std::string& fileName,
										const std::string& ext,
										const std::string& dir="");
	  std::wstring FindNextSequenceName(const std::wstring& fileName,
										const std::wstring& ext,
										const std::wstring& dir=L"");
  
	  uint32_t FindNextSequenceIndex(const std::string& fileName,
									 const std::string& ext,
									 const std::string& dir="");
	  uint32_t FindNextSequenceIndex(const std::wstring& fileName,
									 const std::wstring& ext,
									 const std::wstring& dir=L"");
  
    // *dir functions return true on success
    // ATTENTION: that is the opposite behavior than usual for the corresponding C API functions that return 0 on success
	  bool Chdir(const std::string& path);
    bool Mkdir(const std::string& path);
    bool Rmdir(const std::string& path);
  
	  bool GetHomeDirectory(std::string& path);
	  bool GetHomeDirectory(std::wstring& path);
/*
    bool GetTempDirectory(std::string& path);
	  bool GetTempDirectory(std::wstring& path);
 */
  
	#ifdef DETECTED_OS_WINDOWS
	  std::vector<std::wstring> GetDirContents(const std::wstring& dir,
											   const std::wstring& fileName=L"*",
											   const std::wstring& ext=L"*");
	  std::vector<std::string> GetDirContents(const std::string& dir,
											  const std::string& fileName="*",
											  const std::string& ext="*");
	#else
	  std::vector<std::wstring> GetDirContents(const std::wstring& dir,
											   const std::wstring& fileName=L"*",
											   const std::wstring& ext=L"");
	  std::vector<std::string> GetDirContents(const std::string& dir,
											  const std::string& fileName="*",
											  const std::string& ext="");
	#endif
  
	  std::vector<std::wstring> GetSubDirList(const std::wstring& dir);
	  std::vector<std::string> GetSubDirList(const std::string& dir);
  
  #ifdef DETECTED_OS_WINDOWS
    typedef struct ::_stat64 IVDA_stat;
  #else
    typedef struct ::stat IVDA_stat;
  #endif

    bool GetFileStats(const std::string& strFileName, IVDA_stat& stat_buf);
    bool GetFileStats(const std::wstring& wstrFileName, IVDA_stat& stat_buf);

	  void ReplaceAll(std::string& input, const std::string& search, const std::string& replace);
	  void ReplaceAll(std::wstring& input, const std::wstring& search, const std::wstring& replace);
   
    
	  std::wstring TrimStrLeft(const std::wstring &str,
							   const std::wstring& c = L" \r\n\t");
	  std::string TrimStrLeft(const std::string &str,
							  const std::string& c = " \r\n\t");
	  std::wstring TrimStrRight(const std::wstring &str,
								const std::wstring& c = L" \r\n\t");
	  std::string TrimStrRight(const std::string &str,
							   const std::string& c = " \r\n\t");
	  std::wstring TrimStr(const std::wstring &str,
						   const std::wstring& c = L" \r\n\t");
	  std::string TrimStr(const std::string &str,
						  const std::string& c = " \r\n\t");
  
	#ifdef _WIN32
	  bool GetFilenameDialog(const std::string& lpstrTitle,
							 const CHAR* lpstrFilter,
							 std::string &filename, const bool save,
							 HWND owner=NULL, DWORD* nFilterIndex=NULL);
	  bool GetFilenameDialog(const std::wstring& lpstrTitle,
							 const WCHAR* lpstrFilter,
							 std::wstring &filename, const bool save,
							 HWND owner=NULL, DWORD* nFilterIndex=NULL);
	#endif
  
    void msleep(unsigned long milisec);
    
	  class CmdLineParams {
	  public:
	#ifdef _WIN32
		CmdLineParams();
	#endif
		CmdLineParams(int argc, char** argv);
    
		bool SwitchSet(const std::string& parameter);
		bool SwitchSet(const std::wstring& parameter);
    
		bool GetValue(const std::string& parameter, double& value);
		bool GetValue(const std::wstring& parameter, double& value);
		bool GetValue(const std::string& parameter, float& value);
		bool GetValue(const std::wstring& parameter, float& value);
		bool GetValue(const std::string& parameter, int& value);
		bool GetValue(const std::wstring& parameter, int& value);
		bool GetValue(const std::string& parameter, unsigned int& value);
		bool GetValue(const std::wstring& parameter, unsigned int& value);
		bool GetValue(const std::string& parameter, std::string& value);
		bool GetValue(const std::wstring& parameter, std::wstring& value);
    
	  protected:
		std::vector<std::string> m_strArrayParameters;
		std::vector<std::string> m_strArrayValues;
    
		std::string m_strFilename;
	  };
	}
}

#endif // SYSTOOLS_H
