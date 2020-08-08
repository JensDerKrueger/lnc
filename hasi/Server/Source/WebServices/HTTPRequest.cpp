#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(const std::string& server, const std::string& path, uint16_t port, uint32_t iTimeout) :
m_server(server),
m_path(path),
m_port(port),
m_iTimeout(iTimeout)
{
}

bool HTTPRequest::send(std::string& result) const {
  IVDA::TCPSocket tcpSocket;
  
  tcpSocket.SetNonBlocking(m_iTimeout == IVDA::INFINITE_TIMEOUT ? false : true);
  tcpSocket.SetNoDelay(false);
  tcpSocket.SetKeepalive(false);
  
  std::string receivedData="";
  
  try {
    if (tcpSocket.IsConnected()) {
      tcpSocket.Close();
    }
  } catch (IVDA::SocketException const&) {
    // std::cout << "1";
  }
  
  // try to connect
  IVDA::NetworkAddress address(m_server, m_port);
  try {
    tcpSocket.Connect(address, m_iTimeout);
  } catch (IVDA::SocketException const&) {
    
    std::string strAddress;
    uint16_t p ;
    address.GetAddress(strAddress, p);
    return false;
  }
  
  // connected! send request
  try {
    sendString(std::string("GET " + m_path + " HTTP/1.0"),tcpSocket);
    sendString(std::string("HOST: " + m_server),tcpSocket);
    sendString("",tcpSocket);
  } catch (IVDA::SocketException const&) {
//    std::cout << "3";
    return false;
  }
  
  // get data loop
  try {
    int8_t datum = 0;
    while (tcpSocket.IsConnected()) {
      uint32_t const bytes = tcpSocket.ReceiveData(&datum, 1, m_iTimeout);
      if (bytes > 0 && receivedData.length() < 100000) {
        receivedData += datum;
      } else {
        break;
      }
    }
  } catch (IVDA::SocketException const&) {
//    std::cout << "4";
    return false;
  }
  
  // close socket if necessary before we handle the next connection
  try {
    tcpSocket.Close();
  } catch (IVDA::SocketException const&) {
//    std::cout << "5";
  }
  
  result = processData(receivedData);
  return true;
}

void HTTPRequest::sendString(const std::string& str,
                             IVDA::TCPSocket& tcpSocket) const {
  std::string s = str + "\n";
  tcpSocket.SendData((const int8_t*)(s.data()), (uint32_t)s.length(), m_iTimeout);
}

std::string HTTPRequest::processData(const std::string& str) const {
  std::size_t found=str.find("\r\n\r\n");
  if (found==std::string::npos)
    return "";
  else
    return str.substr(found+4);
}


/*
 The MIT License
 
 Copyright (c) 2013 Jens Krueger
 
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
