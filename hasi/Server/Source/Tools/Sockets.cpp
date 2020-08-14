#include "Sockets.h"

//#define UPDATE_TIMEOUT
#ifdef UPDATE_TIMEOUT
#include "Timer.h"
#endif

#ifdef DETECTED_OS_WINDOWS
#include <Ws2tcpip.h>
#include <Mstcpip.h>
#define REQUEST_WINSOCK_VERSION (BYTE)2
#define REQUEST_WINSOCK_SUBVERSION (BYTE)2
#define ssize_t SSIZE_T
//#define EXCEPTION_ON_LOWER_VERSION
#ifdef EXCEPTION_ON_LOWER_VERSION
#include <sstream>
#endif
void IVDA::Socket::StartWinsock(BYTE requestVersion, BYTE requestSubVersion) 
{
	WSADATA wsa;
	int result = WSAStartup(MAKEWORD(requestVersion, requestSubVersion), &wsa);   
	if (result != 0) throw SocketInitException("Could not initialize WinSock.", "WSAStartup", result);
#ifdef EXCEPTION_ON_LOWER_VERSION
	BYTE version = LOBYTE(wsa.wVersion);
	BYTE subVersion = HIBYTE(wsa.wVersion);
	if (version != requestVersion || subVersion != requestSubVersion) 
	{
		EndWinsock();
		std::stringstream ss;
		ss << "Lower WinSock version (" << (uint32_t)version << "." << (uint32_t)subVersion << ") returned than requested (" << (uint32_t)requestVersion << "." << (uint32_t)requestSubVersion << ").";
		throw SocketInitException(ss.str(), "WSAStartup", result);
	}
#endif
}
void IVDA::Socket::EndWinsock()
{
	WSACleanup();
}
#define GetError() WSAGetLastError()
#define E_AGAIN WSAEWOULDBLOCK
#define E_IN_PROGRESS E_AGAIN
#define E_CONNECTION_RESET WSAECONNRESET
#define E_INVALID WSAEINVAL
#define E_CONNECTION_REFUSED WSAECONNREFUSED
#define E_NETWORK_UNREACHABLE WSAENETUNREACH
#define E_TIMED_OUT WSAETIMEDOUT
#else
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ifaddrs.h>
#define SOCKET_ERROR (-1)
#define INVALID_SOCKET (-1)
#define GetError() errno
#define E_AGAIN EAGAIN
#define E_IN_PROGRESS EINPROGRESS
#define E_CONNECTION_RESET ECONNRESET
#define E_INVALID EINVAL
#define E_CONNECTION_REFUSED ECONNREFUSED
#define E_NETWORK_UNREACHABLE ENETUNREACH
#define E_TIMED_OUT ETIMEDOUT
#endif

#ifdef DETECTED_OS_APPLE
#include <sys/sysctl.h> // to query TCP keepalive parameters
#endif

namespace IVDA
{
	NetworkAddress::NetworkAddress()
	{
		InitNativeAddress();
	}

	NetworkAddress::NetworkAddress(const std::string & ipAdressOrHostname)
	{
		InitNativeAddress();
		SetAddress(ipAdressOrHostname, 0);
	}

	NetworkAddress::NetworkAddress(const std::string & ipAdressOrHostname, uint16_t port)
	{
		InitNativeAddress();
		SetAddress(ipAdressOrHostname, port);
	}

	NetworkAddress::NetworkAddress(uint32_t addressBinary)
	{
		InitNativeAddress();
		SetAddress(addressBinary, 0);
	}

	NetworkAddress::NetworkAddress(uint32_t addressBinary, uint16_t networkPort)
	{
		InitNativeAddress();
		SetAddress(addressBinary, networkPort);
	}

	NetworkAddress::NetworkAddress(SpecialAddress specialAddress)
	{
		InitNativeAddress();
		SetAddress(specialAddress, 0);
	}

	NetworkAddress::NetworkAddress(SpecialAddress specialAddress, uint16_t port)
	{
		InitNativeAddress();
		SetAddress(specialAddress, port);
	}

	NetworkAddress::~NetworkAddress()
	{
	}

	void NetworkAddress::InitNativeAddress()
	{
		memset(&nativeAddress, 0, sizeof(sockaddr));
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		inAddress.sin_family = AF_INET;
		inAddress.sin_port = 0;
		// s_addr is usually unsigned long (Windows) or may be uint32_t (Linux)
		// but it is always a 32bit IPv4 address, so we can safely use uint32_t in our API
		// similar for sin_port, which is always 16bit unsigned short
		inAddress.sin_addr.s_addr = INADDR_ANY;
	}

	void NetworkAddress::SetAddress(const std::string & ipAdressOrHostname, uint16_t port)
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		uint32_t addressBinary = inAddress.sin_addr.s_addr;
		if (Socket::IsIP(ipAdressOrHostname)) Socket::IpToBinary(ipAdressOrHostname, addressBinary);
		else Socket::GetIP(ipAdressOrHostname, addressBinary); // throws WinsockUninitialzedException
		SetAddress(addressBinary, htons(port));
	}

	void NetworkAddress::SetAddress(uint32_t addressBinary, uint16_t networkPort)
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		inAddress.sin_port = networkPort;
		inAddress.sin_addr.s_addr = addressBinary;
	}

	void NetworkAddress::SetAddress(SpecialAddress specialAddress, uint16_t port)
	{
		uint16_t networkPort = htons(port);
		if (specialAddress == NetworkAddress::Any) SetAddress(INADDR_ANY, networkPort);
		else if (specialAddress == NetworkAddress::LocalHost) SetAddress(htonl(INADDR_LOOPBACK), networkPort);
		else if (specialAddress == NetworkAddress::Broadcast) SetAddress(INADDR_BROADCAST, networkPort);
	}

	void NetworkAddress::GetAddress(std::string & address, uint16_t & port) const
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		Socket::BinaryToIP(inAddress.sin_addr.s_addr, address);
		port = ntohs(inAddress.sin_port);
	}

	void NetworkAddress::GetAddress(uint32_t & addressBinary, uint16_t & networkPort) const
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		addressBinary = inAddress.sin_addr.s_addr;
		networkPort = inAddress.sin_port;
	}

	// NOTE: for NetworkAddress, it is alwasy sin_family = AF_INET and sin_zero = 0 (no AF_INET6 support yet)
	bool NetworkAddress::operator==(const NetworkAddress & otherAddress) const
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		struct sockaddr_in & otherInAddress = (struct sockaddr_in&)otherAddress.nativeAddress;
		return ((inAddress.sin_addr.s_addr == otherInAddress.sin_addr.s_addr) && (inAddress.sin_port == otherInAddress.sin_port));
	}

	bool NetworkAddress::operator==(SpecialAddress specialAddress) const
	{
		struct sockaddr_in & inAddress = (struct sockaddr_in&)nativeAddress;
		bool addressEqual = false;
		if (specialAddress == NetworkAddress::Any) addressEqual = (inAddress.sin_addr.s_addr == INADDR_ANY);
		else if (specialAddress == NetworkAddress::LocalHost) addressEqual = (inAddress.sin_addr.s_addr == htonl(INADDR_LOOPBACK));
		else if (specialAddress == NetworkAddress::Broadcast) addressEqual = (inAddress.sin_addr.s_addr == INADDR_BROADCAST);
		return addressEqual;
	}

	bool NetworkAddress::operator!=(const NetworkAddress & otherAddress) const
	{
		return !(*this == otherAddress);
	}

	bool NetworkAddress::operator!=(SpecialAddress specialAddress) const
	{
		return !(*this == specialAddress);
	}

	Socket::Socket() :
#ifdef DETECTED_OS_WINDOWS
  m_bNonBlocking(false), // per default all sockets are blocking unless you explicitly ioctlsocket() them with FIONBIO or hand them to either WSAAsyncSelect or WSAEventSelect. The latter two functions "secretly" change the socket to non-blocking.
#endif
#ifdef DETECTED_OS_LINUX
  m_SendFlags(MSG_NOSIGNAL),
#else
  m_SendFlags(0),
#endif
  m_Socket(INVALID_SOCKET)
	{
#ifdef DETECTED_OS_WINDOWS
		StartWinsock(REQUEST_WINSOCK_VERSION, REQUEST_WINSOCK_SUBVERSION); // throws SocketInitException
#endif
	}

	Socket::~Socket()
	{
		// Close will throw an exception here if we have called it before, as close/closesocket will return ENOTSOCK/WSAENOTSOCK
		// -> will not happen anymore
		if (m_Socket != INVALID_SOCKET)
		{
			try
			{
				Close();
			}
			catch (const SocketException &)
			{
			}
		}

#ifdef DETECTED_OS_WINDOWS
		EndWinsock();
#endif
	}

	NetworkAddress Socket::GetLocalNetworkAddress() const
	{
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		socklen_t size = sizeof(sockaddr_in);
		if (getsockname(m_Socket, (sockaddr*)&address, &size) == SOCKET_ERROR) throw SocketException("Could not retrieve local name for socket.", "getsockname", GetError());
		return NetworkAddress(address.sin_addr.s_addr, address.sin_port);
	}

	bool Socket::IsBound() const
	{
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		socklen_t size = sizeof(sockaddr_in);
		int result = getsockname(m_Socket, (sockaddr*)&address, &size);
		if (result == SOCKET_ERROR) 
		{
#ifdef DETECTED_OS_WINDOWS
			if (GetError() == E_INVALID) // socket not bound
			{
				return false;
			}
#endif
			throw SocketException("Could not retrieve local name for socket.", "getsockname", GetError());
		}
#ifdef DETECTED_OS_WINDOWS
		return true;
#else
		return (address.sin_port != 0);
#endif
	}

	void Socket::Bind(const NetworkAddress & address)
	{
		if (bind(m_Socket, &address.GetNativeAddress(), sizeof(sockaddr)) == SOCKET_ERROR) throw SocketException("Could not bind socket.", "bind", GetError());

		// below applies for listen sockets
		// we may need to consider this and reimplement Bind for ListenSocket
		// use getsockname here to determine address and port assigned to socket (should be any address, 0.0.0.0)
		// if 0 was passed as port, an available port from the dynamic port range is choosen automatically
		/*memset(&address, 0, sizeof(sockaddr_in));
		int size = sizeof(sockaddr_in);
		int result = getsockname(m_Socket, (sockaddr*)&address, &size);
		if (result == SOCKET_ERROR)
		{
			// according to http://msdn.microsoft.com/en-us/library/ms738543%28v=VS.85%29.aspx, we would have to handle WSAEINVAL separately in windows
			// investigate this
			// without handling this special case, this is the same implementation as UpdateLocalName in Socket
	//#ifdef DETECTED_OS_WINDOWS
	//		if (GetError() == WSAEINVAL) ...
			throw SocketException("Could not retrieve local name for socket.", "getsockname", GetError());
		}
		BinaryToIP(address.sin_addr.s_addr, localAddress);
		localPort = ntohs(address.sin_port);*/
	}

	void Socket::Close()
	{
		//if (m_Socket == INVALID_SOCKET) return; // to prevent WSAENOTSOCK error
	#ifdef DETECTED_OS_WINDOWS
		if (closesocket(m_Socket) == SOCKET_ERROR) throw SocketException("Could not close socket.", "closesocket", GetError());
	#else
		if (close(m_Socket) == SOCKET_ERROR) throw SocketException("Could not close socket.", "close", GetError());
	#endif
		m_Socket = INVALID_SOCKET;
	}

  void Socket::SetNonBlocking(bool bNonBlocking)
  {
#ifdef DETECTED_OS_WINDOWS
    u_long blocking = (bNonBlocking ? 1 : 0);
    if (ioctlsocket(m_Socket, FIONBIO, &blocking) == SOCKET_ERROR) throw SocketOptionException("Could not set blocking mode option.", "ioctlsocket", GetError());
    m_bNonBlocking = bNonBlocking; // we need to remember the non blocking property because there is no API call to query this property under Windows
#else
    int options = fcntl(m_Socket, F_GETFL, 0);
    if (options != SOCKET_ERROR)
    {
      if (bNonBlocking) options |= O_NONBLOCK;
      else options &= !O_NONBLOCK;
      if (fcntl(m_Socket, F_SETFL, options) != SOCKET_ERROR) return;
    }
    throw SocketOptionException("Could not set blocking mode option.", "fcntl", GetError());
#endif
  }

  bool Socket::GetNonBlocking() const
  {
#ifdef DETECTED_OS_WINDOWS
    return m_bNonBlocking;
#else
    int options = fcntl(m_Socket, F_GETFL, 0);
	if (options == SOCKET_ERROR) throw SocketOptionException("Could not query blocking mode option.", "fcntl", GetError());
    return options & O_NONBLOCK;
#endif    
  }

	void Socket::SetReuseAddress(bool bReuseAddress)
	{
		socklen_t newReuseAddress = (bReuseAddress ? 1 : 0);
		if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&newReuseAddress, sizeof(newReuseAddress)) == SOCKET_ERROR) throw SocketOptionException("Could not set reuse address option.", "setsockopt", GetError());
	}

  bool Socket::GetReuseAddress() const
  {
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query reuse address option.", "getsockopt", GetError());
    return optval != 0;
  }

  // only supported for Window platforms
#if defined(DETECTED_OS_WINDOWS)
  void Socket::SetExclusiveAddressUse(bool bExclusiveAddressUse)
  {
    socklen_t newExclusiveAddressUse = (bExclusiveAddressUse ? 1 : 0);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char*)&newExclusiveAddressUse, sizeof(newExclusiveAddressUse)) == SOCKET_ERROR) throw SocketOptionException("Could not set exclusive address use option.", "setsockopt", GetError());
  }
#else
  void Socket::SetExclusiveAddressUse(bool) {}
#endif

  bool Socket::GetExclusiveAddressUse() const
  {
#ifdef DETECTED_OS_WINDOWS
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query exclusive address use option.", "getsockopt", GetError());
    return optval != 0;
#else
    return false;
#endif
  }

	void Socket::SetSendBufferSize(int32_t size)
	{
		if (setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(int32_t)) == SOCKET_ERROR) throw SocketOptionException("Could not set send buffer size.", "setsockopt", GetError());
	}

	void Socket::SetReceiveBufferSize(int32_t size)
	{
		if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(int32_t)) == SOCKET_ERROR) throw SocketOptionException("Could not set receive buffer size.", "setsockopt", GetError());
	}

	int32_t Socket::GetSendBufferSize() const
	{
		socklen_t size = sizeof(int32_t);
		int32_t sendBufferSize = 0;
		if (getsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &size) == SOCKET_ERROR) throw SocketOptionException("Could not retrieve send buffer size.", "getsockopt", GetError());
		return sendBufferSize;
	}

	int32_t Socket::GetReceiveBufferSize() const
	{
		socklen_t size = sizeof(int32_t);
		int32_t receiveBufferSize = 0;
		if (getsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&receiveBufferSize, &size) == SOCKET_ERROR) throw SocketOptionException("Could not retrieve receive buffer size.", "getsockopt", GetError());
		return receiveBufferSize;
	}

	// default implementation for TCP here
	// other sockets should overwrite, otherwise setsockopt will return WSAEINVAL/EINVAL
	void Socket::SetNoDelay(bool bNoDelay)
	{
		int optval = (bNoDelay ? 1 : 0);
		socklen_t optlen = sizeof(optval);
		if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, optlen) == SOCKET_ERROR) throw SocketOptionException("Could not set no delay option.", "setsockopt", GetError());
	}

  bool Socket::GetNoDelay() const
  {
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query no delay option.", "getsockopt", GetError());
    return optval != 0;
  }

	void Socket::SetNoSigPipe(bool bNoSigPipe)
	{
#ifndef DETECTED_OS_WINDOWS
#ifndef DETECTED_OS_LINUX
		socklen_t newSigPipe = (bNoSigPipe ? 1 : 0);
		if (setsockopt(m_Socket, SOL_SOCKET, SO_NOSIGPIPE, (const void*)&newSigPipe, sizeof(newSigPipe)) == SOCKET_ERROR) throw SocketOptionException("Could not set no sigpipe option.", "setsockopt", GetError());
#else
		m_SendFlags = (bNoSigPipe ? MSG_NOSIGNAL : 0);
#endif
#endif
	}

  bool Socket::GetNoSigPipe() const
  {
#ifndef DETECTED_OS_WINDOWS
#ifndef DETECTED_OS_LINUX
    // OSX
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_NOSIGPIPE, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query no sigpipe option.", "getsockopt", GetError());
    return optval != 0;
#else
    // Linux
    return m_SendFlags == MSG_NOSIGNAL;
#endif
#else
    // Windows
    return false;
#endif
  }

	bool Socket::IsIP(const std::string & ipAdressOrHostname)
	{
		struct sockaddr_in sa;
		return (inet_pton(AF_INET, ipAdressOrHostname.c_str(), &(sa.sin_addr)) == 1);
	}

	bool Socket::IpToBinary(const std::string & ipString, uint32_t & ipBinary)
	{
		return (inet_pton(AF_INET, ipString.c_str(), (struct in_addr*)&ipBinary) == 1);
	}

	bool Socket::BinaryToIP(uint32_t ipBinary, std::string & ipString)
	{
		char address[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, (struct in_addr*)&ipBinary, address, sizeof(address)))
		{
			ipString = address;
			return true;
		}
		return false;
	}

	bool Socket::GetIP(const std::string & hostname, uint32_t & ipBinary)
	{
		hostent * host = gethostbyname(hostname.c_str());
		if (host && host->h_addr_list[0] != NULL)
		{
			ipBinary = (uint32_t)((in_addr*)(host->h_addr_list[0]))->s_addr;
			return true;
		}
#ifdef DETECTED_OS_WINDOWS
		if (GetError() == WSANOTINITIALISED) throw WinsockUninitializedException("Winsock has not been initialized.", "gethostbyname", GetError());
#endif
		return false;
	}

	bool Socket::GetIP(const std::string & hostname, std::string & ipString)
	{
		uint32_t ipBinary = 0;
		if (GetIP(hostname, ipBinary)) return BinaryToIP(ipBinary, ipString); // throws WinsockUninitialzedException
		return false;
	}

	// there are various ways to do this
	// this includes using WSAIoctl/ioctl (http://tangentsoft.net/wskfaq/examples/getifaces.html), getaddrinfo/GetAddrInfo , GetAdaptersAddresses, GetAdaptersInfo, GetIpAddrTable
	// the last three give more information than we need here and may be much slower as indicated in http://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx, Remarks
	// therefore we go with getaddrinfo
	// getaddrinfo can be used on Windows, OS X and Linux, but the behaviour is not consistent at all (http://klickverbot.at/blog/2012/01/getaddrinfo-edge-case-behavior-on-windows-linux-and-osx/)
	// Linux seems to never return the actual addresses we want
	// therefore, we use getifaddr on Linux and OS X
	bool Socket::GetMyNetworkAddresses(std::vector<NetworkAddress> & networkAddresses)
	{
		networkAddresses.clear();
#ifdef DETECTED_OS_WINDOWS
		struct addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_INET;

		struct addrinfo * interfaceList = NULL;
		INT result = getaddrinfo("", NULL, &hints, &interfaceList);
		if (result != 0)
		{
			if (result == WSANOTINITIALISED) throw WinsockUninitializedException("Winsock has not been initialized.", "getaddrinfo", result);
			return false;
		}
		
		struct addrinfo * interfacePtr = NULL;
		for (interfacePtr=interfaceList; interfacePtr!=NULL; interfacePtr=interfacePtr->ai_next)
		{
			sockaddr_in * address = (sockaddr_in*)interfacePtr->ai_addr;
			networkAddresses.push_back(NetworkAddress(address->sin_addr.s_addr, address->sin_port));
		}
		freeaddrinfo(interfaceList);
#else
		struct ifaddrs * interfaceList = NULL;
        if (getifaddrs(&interfaceList) != 0) return false;

		struct ifaddrs * interfacePtr = NULL;
        for (interfacePtr=interfaceList; interfacePtr!=NULL; interfacePtr=interfacePtr->ifa_next) 
		{
			if (!interfacePtr->ifa_addr) continue;

			if (interfacePtr->ifa_addr->sa_family == AF_INET)
			{
				sockaddr_in * address = (sockaddr_in*)interfacePtr->ifa_addr;
                if (address->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) networkAddresses.push_back(NetworkAddress(address->sin_addr.s_addr, address->sin_port));
			}
        }
        freeifaddrs(interfaceList);
#endif
		return true;
	}

	uint64_t Socket::NetworkInt64(uint64_t hostInt)
	{
		static const int num = 42;
		if (*reinterpret_cast<const char*>(&num) == num)
		{
			// we are little endian, so swap
			const uint32_t high_part = htonl(static_cast<uint32_t>(hostInt >> 32));
			const uint32_t low_part = htonl(static_cast<uint32_t>(hostInt & 0xFFFFFFFFLL));

			return (static_cast<uint64_t>(low_part) << 32) | high_part;
		} 
		else return hostInt;
	}

	uint32_t Socket::NetworkInt32(uint32_t hostInt) 
	{ 
		return htonl(hostInt); 
	}
	
	uint16_t Socket::NetworkInt16(uint16_t hostInt) 
	{ 
		return htons(hostInt); 
	}
	
	uint64_t Socket::HostInt64(uint64_t networkInt)
	{
		return NetworkInt64(networkInt);
	}

	uint32_t Socket::HostInt32(uint32_t networkInt)
	{ 
		return ntohl(networkInt); 
	}
		
	uint16_t Socket::HostInt16(uint16_t networkInt) 
	{ 
		return ntohs(networkInt); 
	}

	IOSocket::IOSocket() : Socket()
	{
	}

	IOSocket::~IOSocket()
	{
	}

	// attention, this will also return true if a graceful disconnect has been done
	// we handle this case in ReadData
	// use CheckForDisconnect to determine if the connection is still active
	// this will also return true if a socket is listening and a new connection is pending
	bool IOSocket::IsReadyToReadData(uint32_t & timeout)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_Socket, &readSet);

		int result;
		if (timeout != INFINITE_TIMEOUT)
		{
			timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout % 1000) * 1000;
#ifdef UPDATE_TIMEOUT
			Timer timer;
			timer.Start();
#endif
			result = select((int)m_Socket + 1, &readSet, NULL, NULL, &tv);
#ifdef UPDATE_TIMEOUT
			if (result == 0) // timeout
			{
				timeout = 0;
				return false;
			}
			uint32_t elapsedTime = (uint32_t)timer.Elapsed();
			timeout -= ((elapsedTime > timeout) ? timeout : elapsedTime);
#endif
		}
		else result = select((int)m_Socket + 1, &readSet, NULL, NULL, NULL);

		if (FD_ISSET(m_Socket, &readSet)) return true;
		else if (result == 0) return false; // timeout
		else throw SocketException("Could not determine readability of socket.", "select", GetError()); // result == SOCKET_ERROR
	}

	// this will also return true after a non-blocking connect call has been processed (does not necessarily mean connect was successful, see Connect method)
	bool IOSocket::IsReadyToWriteData(uint32_t & timeout)
	{
		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(m_Socket, &writeSet);

		int result;
		if (timeout != INFINITE_TIMEOUT)
		{
			timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout % 1000) * 1000;
#ifdef UPDATE_TIMEOUT
			Timer timer;
			timer.Start();
#endif
			result = select((int)m_Socket + 1, NULL, &writeSet, NULL, &tv);
#ifdef UPDATE_TIMEOUT
			if (result == 0) // timeout
			{
				timeout = 0;
				return false;
			}
			uint32_t elapsedTime = (uint32_t)timer.Elapsed();
			timeout -= ((elapsedTime > timeout) ? timeout : elapsedTime);
#endif
		}
		else result = select((int)m_Socket + 1, NULL, &writeSet, NULL, NULL);

		if (FD_ISSET(m_Socket, &writeSet)) return true;
		else if (result == 0) return false; // timeout
		else throw SocketException("Could not determine writabilty of socket.", "select", GetError()); // result == SOCKET_ERROR
	}

	// for a proper timeout with below methods, we need to use gettimeofday/GetSystemTime and track the time remaining ourselfs
	// timeval may be updated by select, but this is not guaranteed (see http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#select)

	// those do not split data further, but the UDPSocket overwrites SendData to do so
	// also, these use send/recv for both TCP and UDP which works since we have set up a virtual UDP connection with connect
	// and therefore have a default destination send uses -> implemented differently now using only sendto/recvfrom
  uint32_t IOSocket::SendDataImplementation(const int8_t * data, uint32_t size, const sockaddr * destination, socklen_t destinationSize, uint32_t & timeout)
  {
    uint32_t bytesSend = 0;
    do
    {
      ssize_t result = sendto(m_Socket, (const char*)(data + bytesSend), size - bytesSend, m_SendFlags, destination, destinationSize);
      if (result > 0) bytesSend += (uint32_t)result;
      // we can not capture a graceful disconnect as with ReceiveData here
      else if (result == SOCKET_ERROR)
      {
        // consider returning here instantly if timeout = 0 and error is E_AGAIN
        if (GetError() == E_AGAIN) // non-blocking socket, we wait until we can write
        {
          bool readyWrite = IsReadyToWriteData(timeout); // throws SocketException
          if (!readyWrite) return bytesSend; // timeout
        }
        // hard close
        else if (GetError() == E_CONNECTION_RESET) throw SocketConnectionException("Connection error.", "sendto", GetError());
        else throw SocketException("Could not send data.", "sendto", GetError());
      }
    }
    while (bytesSend < size);

    return bytesSend;
  }

  uint32_t IOSocket::ReceiveDataImplementation(int8_t * data, uint32_t size, sockaddr * source, socklen_t * sourceSize, uint32_t & timeout)
  {
    uint32_t bytesReceived = 0;
    do
    {
      ssize_t result = recvfrom(m_Socket, (char*)(data + bytesReceived), size - bytesReceived, 0, source, sourceSize);
      if (result > 0) bytesReceived += (uint32_t)result;
      else if (result == SOCKET_ERROR)
      {
        // consider returning here instantly if timeout = 0 and error is E_AGAIN
        if (GetError() == E_AGAIN) // non-blocking socket, we wait until we can read
        {
          bool readyRead = IsReadyToReadData(timeout); // throws SocketException
          if (!readyRead) return bytesReceived; // timeout
        }
        // hard close
        else if (GetError() == E_CONNECTION_RESET) throw SocketConnectionException("Connection error.", "recvfrom", GetError());
        else throw SocketException("Could not receive data.", "recvfrom", GetError());
      }
      // graceful disconnect
      // note: for Windows, recvfrom returns with SOCKET_ERROR and E_AGAIN when trying to read 0 bytes and there is nothing to read
      // in the end, this will result in 0 being returned (see returning bytesReceived at timeout above)
      // on UNIX though, 0 is always returned when reading 0 bytes (maybe on Windows as well if there is something to read -> test this)
      // so on UNIX, we always have to check for size > 0 here to make sure we have a graceful disconnect
      // if result = 0 and size = 0, we end up returning 0 as well and do not report a false disconnect
      // to have consistent behavior, we check for size > 0 in general
      // so ReceiveData(NULL, 0, 0) can not be used to poll for a graceful disconnect, use IsConnected() instead
      // note: if Close() is called locally while a recvfrom call is ongoing (i.e. a blocking one), this triggers also a graceful disconnect, but not from the remote side
      // therefore, we choose below general exception message instead of something like "Peer disconnected."
      else if (result == 0 && size > 0) throw SocketConnectionException("Graceful disconnection.", "recvfrom", 0);
    }
    while (bytesReceived < size);

    return bytesReceived;
  }

	uint32_t IOSocket::SendData(const int8_t * data, uint32_t size, uint32_t timeout)
	{
		return SendDataImplementation(data, size, NULL, 0, timeout); // throws SocketException, SocketConnectionException
	}

	uint32_t IOSocket::ReceiveData(int8_t * data, uint32_t size, uint32_t timeout)
	{
		return ReceiveDataImplementation(data, size, NULL, NULL, timeout); // throws SocketException, SocketConnectionException
	}

	uint32_t IOSocket::SendData(const int8_t * data, uint32_t size, const NetworkAddress & destination, uint32_t timeout)
	{
		return SendDataImplementation(data, size, &destination.GetNativeAddress(), sizeof(sockaddr), timeout); // throws SocketException, SocketConnectionException
	}

	uint32_t IOSocket::ReceiveData(int8_t * data, uint32_t size, NetworkAddress & source, uint32_t timeout)
	{
		socklen_t sourceSize = sizeof(sockaddr);
		return ReceiveDataImplementation(data, size, &source.GetNativeAddressRW(), &sourceSize, timeout); // throws SocketException, SocketConnectionException
	}

	// NOTE: timeout with partial send (i.e. only 2 bytes of integer send) will be difficult to handle here
	// as we would have to resend only the remaining (not send) bytes of our integer, so we would need to cast our integer into a byte pointer first
	// a mess considering the incomplete integer we send is in network byte order and we would need to convert our integer to network order first before casting it to a byte pointer
	// to send the remaining bytes
	uint32_t IOSocket::SendInt(int32_t integer)
	{
		int32_t integerNetwork = htonl(integer);
		return SendData((const int8_t*)&integerNetwork, sizeof(int32_t), INFINITE_TIMEOUT); // throws SocketException, SocketConnectionException
	}

	// NOTE: timeout with partial read (i.e. only 2 bytes of integer received) will be difficult to handle here
	// as we would have to read the missing bytes and put partial results together to create the actual integer
	// a mess considering the incomplete integer we get is in host byte order and we would need to convert it back to network order to retrieve the right bytes
	// the final composited integer needs to be converted to host byte order then
	uint32_t IOSocket::ReceiveInt(int32_t & integer)
	{
		int32_t integerNetwork = 0;
		uint32_t bytesReceived = ReceiveData((int8_t*)&integerNetwork, sizeof(int32_t), INFINITE_TIMEOUT); // throws SocketException, SocketConnectionException
		integer = ntohl(integerNetwork);
		return bytesReceived;
	}

	// those are unsafe calls as we do not handle partial results and timeouts from SendInt or SendDataInternal, which is difficult as described above
	// we would need to handle timeout in SendInt and SendDataInternal separately to know where and how to continue sending after a partial result
	uint32_t IOSocket::SendString(const std::string & string)
	{
		SendInt((int32_t)string.size());
		if (string.empty()) return 0;
		return SendData((const int8_t*)string.c_str(), (uint32_t)string.size() * sizeof(int8_t)); // throws SocketException, SocketConnectionException
	}

	uint32_t IOSocket::ReceiveString(std::string & string)
	{
		int32_t size = 0;
		ReceiveInt(size);
		if (size == 0)
		{
			string.clear();
			return 0;
		}

    int8_t * buffer;
    try {
		  buffer = new int8_t[(uint32_t)size + 1]; // throws std::bad_alloc
    } catch (const std::bad_alloc&) {
      throw SocketException("invalid string size", "ReceiveString", -1);
    }
		uint32_t bytesReceived = ReceiveData(buffer, (uint32_t)size, INFINITE_TIMEOUT); // throws SocketException, SocketConnectionException
		buffer[bytesReceived] = 0;
		
		string = (char*)buffer;
		delete [] buffer;
		return bytesReceived;
	}

	ConnectionSocket::ConnectionSocket() :
  IOSocket(),
  m_bConnectionInProgress(false), 
  m_bConnected(false)
	{
	}

	// server has to make sure to set m_bConnected, m_bNoDelay, m_bBlocking right (see ListenSocket)
	// as this socket will inherit the attributes from the listening server socket (except m_bConnected obviously, but a socket here comes fresh from a successful connection the server accepted)
	// we could check those attributes here as well, but we do not need this for now as the server knows them
	ConnectionSocket::ConnectionSocket(SOCKET socketDescriptor) :
  IOSocket(),
  m_bConnectionInProgress(false),
  m_bConnected(false)
	{
		// we do not do additional error checking here (i.e. using select with exceptfds)
		m_Socket = socketDescriptor;
	}

	ConnectionSocket::~ConnectionSocket()
	{
		// Disconnect will throw an exception here if we have called it before, as shutdown will return WSAENOTCONN/ENOTCONN
		// -> does not happen anymore
		if (m_bConnected)
		{
			try
			{
				Disconnect();
			}
			catch (const SocketException &)
			{
			}
		}
	}

	NetworkAddress ConnectionSocket::GetPeerNetworkAddress() const
	{
		sockaddr_in address;
		memset(&address, 0, sizeof(sockaddr_in));
		socklen_t size = sizeof(sockaddr_in);
		if (getpeername(m_Socket, (sockaddr*)&address, &size) == SOCKET_ERROR) throw SocketException("Could not retrieve peer name for socket.", "getpeername", GetError());
		return NetworkAddress(address.sin_addr.s_addr, address.sin_port);
	}

	bool ConnectionSocket::WaitForConnected(uint32_t timeout)
	{
		// this call should only be used after a Connect call indicating a connection in progress (by returning with a timeout)
		if (!m_bConnectionInProgress) return false;

		// select on Windows behaves different than on Apple/Linux when it comes to a non-blocking connect call beforehand
		// on Apple/Linux, if the connection attempt completes, writability is signalised and SO_ERROR has to be checked with getsockopt to determine success or failure
		// should the connection attempt fail, the error is NOT signalised by select, so we only need to check for writability here
		// on Windows, if the connection attempt completes, either writability is signalised (success) or an error (failure), but not both
		// is an error signalised, we use getsockopt to retrieve the exact error code

#ifndef DETECTED_OS_WINDOWS
		bool readyWrite = IsReadyToWriteData(timeout); // throws SocketException
		if (!readyWrite) return false; // timeout

		// check whether connection was completed successfully
		// we could probably replace this by additionally checking for exceptfds with select
		// if the socket is set in exceptfds and writefds, connection should have been failed
		// if the socket is only set in writefds, connection should have been successfull
		// test this -> done, does not work, the behaviour of select in this case is different on Windows than on Apple/Linux, it won't work on either of them!
		
		m_bConnectionInProgress = false;
		socklen_t error;
		socklen_t size = sizeof(socklen_t);
		if (getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, (char*)&error, &size) == SOCKET_ERROR) throw SocketException("Could not determine state of completed connection.", "getsockopt", GetError());
		else if (error != 0)
		{
			if (error == E_CONNECTION_REFUSED || error == E_NETWORK_UNREACHABLE || error == E_TIMED_OUT) throw SocketConnectionException("Connection attempt failed.", "connect", error);
			else throw SocketException("Connection attempt error.", "connect", error);
		}
#else
		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(m_Socket, &writeSet);
		fd_set errorSet;
		FD_ZERO(&errorSet);
		FD_SET(m_Socket, &errorSet);

		int result;
		if (timeout != INFINITE_TIMEOUT)
		{
			timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout % 1000) * 1000;

			result = select((int)m_Socket + 1, NULL, &writeSet, &errorSet, &tv);
		}
		else result = select((int)m_Socket + 1, NULL, &writeSet, &errorSet, NULL);

		if (FD_ISSET(m_Socket, &writeSet)) // connection attempt succeeded
		{
			m_bConnectionInProgress = false;
		}
		else if (result == 0) return false; // timeout, connection attempt still ongoing
		else if (FD_ISSET(m_Socket, &errorSet)) // connection attempt failed
		{
			m_bConnectionInProgress = false;
			socklen_t error;
			socklen_t size = sizeof(socklen_t);
			if (getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, (char*)&error, &size) == SOCKET_ERROR) throw SocketException("Could not determine error of failed connection attempt.", "getsockopt", GetError());
			// error will also contain a SOCKET_ERROR type error produced by connect
			if (error == E_CONNECTION_REFUSED || error == E_NETWORK_UNREACHABLE || error == E_TIMED_OUT) throw SocketConnectionException("Connection attempt failed.", "connect", error);
			else throw SocketException("Connection attempt error.", "error", error);
		}
		else throw SocketException("Could not determine writabilty and error state of socket.", "select", GetError());
#endif
		
		m_bConnected = true;
		return true;
	}

	bool ConnectionSocket::Connect(const NetworkAddress & address, uint32_t timeout)
	{	
		int result = connect(m_Socket, &address.GetNativeAddress(), sizeof(sockaddr));
		if (result == SOCKET_ERROR)
		{
			int error = GetError();
			// non-blocking socket, we wait for connect to complete and handle connect result and erros in WaitForConnected
			// note: this is different in UNIX and Windows, first uses EINPROGRESS, second WSAWOULDBLOCK to indicate a connection in progress
			if (error == E_IN_PROGRESS)
			{
				m_bConnectionInProgress = true;
				bool connectionSuccess = WaitForConnected(timeout); // throws SocketException, SocketConnectionException
				return connectionSuccess;
			}
			// probably handle WSAECONNREFUSED, WSAENETUNREACH, WSAETIMEDOUT separately here as trying with another Connect call is valid after those errors -> done
			// probably WSAEISCONN as well (no, we want an exception in this case)
			// also, EINTR return value might be handled specifically on Apple/Linux (see http://pubs.opengroup.org/onlinepubs/009695399/9)
			if (error == E_CONNECTION_REFUSED || error == E_NETWORK_UNREACHABLE || error == E_TIMED_OUT) throw SocketConnectionException("Connection attempt failed.", "connect", error);
			else throw SocketException("Connection attempt error.", "connect", error);
		}

		m_bConnected = true;
	
		return true;
	}

	// we could do this check with select (check for read) and ioctl(sock) with FIONREAD as well
	// if select returns success (can read from socket) and ioctl returns zero pending data, graceful disconnection has occurred
	// this works for both blocking and non-blocking sockets then
	// -> done
	// issue: this does not capture a hard close (see comment below)
	bool ConnectionSocket::IsConnected()
	{
		if (!m_bConnected) return false;

		uint32_t timeout = 0;
		bool readyRead = IsReadyToReadData(timeout); // throws SocketException

		if (readyRead)
		{
#ifdef DETECTED_OS_WINDOWS
			u_long bytesToRead = 0;
			if (ioctlsocket(m_Socket, FIONREAD, &bytesToRead) == SOCKET_ERROR) throw SocketException("Could not determine bytes ready for reading on socket.", "ioctlsocket", GetError());
			if (bytesToRead == 0) m_bConnected = false;
#else
			int bytesToRead = 0;
			if (ioctl(m_Socket, FIONREAD, &bytesToRead) == SOCKET_ERROR) throw SocketException("Could not determine bytes ready for reading on socket.", "ioctl", GetError());
			if (bytesToRead == 0) m_bConnected = false;
#endif
		}
	
		return m_bConnected;

		// below works as expected for stream-based, non-blocking sockets only, need to use waitForReadyRead(0) before using with blocking sockets so we guarantee recv does not block
		// we could use it, maybe this is faster
		// also, we could handle a hard close here since recv can return WSAECONNRESET/ECONNRESET
		// for message based, connection-oriented protocols (i.e. SCTP), we would need to peek with SO_MAX_MSG_SIZE to guarantee a datagram can be fully read, otherwise WSAEMSGSIZE error would be returned
		// so we need to handle WSAEMSGSIZE specifically or peek with SO_MAX_MSG_SIZE in this case
		// here, we do not need to do this though, as we only use UDP for now and it is connection-less anyway, so this is not necessary at all
		/*char buffer[1];
		int bytesReceived = recv(m_Socket, buffer, 1, MSG_PEEK);
		return ((bytesReceived == SOCKET_ERROR && GetError() == E_AGAIN) || (bytesReceived > 0)) && m_bConnected;*/
	}

	void ConnectionSocket::Disconnect()
	{
		m_bConnectionInProgress = false;
		m_bConnected = false;
	}

	void ConnectionSocket::Close()
	{
		m_bConnected = false;
		Socket::Close(); // throws SocketException
	}

	ListenSocket::ListenSocket() : Socket(), m_bListening(false)
	{
	}

	ListenSocket::~ListenSocket()
	{
	}

  void ListenSocket::Listen(uint32_t backlog)
  {
    if (listen(m_Socket, (backlog > SOMAXCONN ? SOMAXCONN : backlog)) == SOCKET_ERROR) throw SocketException("Could not set socket in listening state.", "listen", GetError());

    m_bListening = true;
  }

	bool ListenSocket::IsListening()
	{
		if (!m_bListening) return false;
		socklen_t isListening = 0;
		socklen_t size = sizeof(socklen_t);
		int result = getsockopt(m_Socket, SOL_SOCKET, SO_ACCEPTCONN, (char*)&isListening, &size);
		if (result == SOCKET_ERROR) throw SocketException("Could not determine listening status.", "getsockopt", GetError());
		m_bListening = (isListening > 0);
		return m_bListening;
	}

	// code copied from IOSocket waitForReadyRead
	bool ListenSocket::WaitForNewConnection(uint32_t & timeout)
	{
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(m_Socket, &readSet);

		int result;
		if (timeout != INFINITE_TIMEOUT)
		{
			timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = (timeout % 1000) * 1000;
#ifdef UPDATE_TIMEOUT
			Timer timer;
			timer.Start();
#endif
			result = select((int)m_Socket + 1, &readSet, NULL, NULL, &tv);
#ifdef UPDATE_TIMEOUT
			if (result == 0) // timeout
			{
				timeout = 0;
				return false;
			}
			uint32_t elapsedTime = (uint32_t)timer.Elapsed();
			timeout -= ((elapsedTime > timeout) ? timeout : elapsedTime);
#endif
		}
		else result = select((int)m_Socket + 1, &readSet, NULL, NULL, NULL);
	
		if (FD_ISSET(m_Socket, &readSet)) return true;
		else if (result == 0) return false; // timeout
		else throw SocketException("Could not determine readability of socket.", "select", GetError()); // result == SOCKET_ERROR
	}

	bool ListenSocket::AcceptNewConnection(ConnectionSocket ** connectedSocket, uint32_t timeout)
	{
		do
		{
			sockaddr_in connectionInfo;
			memset(&connectionInfo, 0, sizeof(sockaddr_in));
			socklen_t connectionInfoLength = sizeof(sockaddr_in);
			SOCKET socketDescriptor = accept(m_Socket, (struct sockaddr*)&connectionInfo, &connectionInfoLength);

			if (socketDescriptor == INVALID_SOCKET)
			{
				if (GetError() == E_AGAIN) // non-blocking socket, no connection currently available, so we wait for a new one
				{
					bool readyRead = WaitForNewConnection(timeout); // throws SocketException
					if (!readyRead) return false; // timeout
					else continue; // pending connection available, try to accept it
				}
				// handle this exception separately, as it is not critical and the server may continue to accept connections as normal
				else if (GetError() == E_CONNECTION_RESET) throw SocketConnectionException("Connection terminated before it could be accepted.", "accept", GetError());
				else throw SocketException("Unable to accept incoming connections.", "accept", GetError());
			}

			if (connectedSocket) 
			{
				ConnectionSocket * connectionSocket = CreateConnectionSocket(socketDescriptor); // throws SocketInitException
				connectionSocket->m_bConnected = true;
        try
        {
          // some systems inherit the non blocking property (Windows, OSX, iOS), some don't (some Linuxes)
          // to be on the safe side we explicitly set this property here
          connectionSocket->SetNonBlocking(GetNonBlocking()); // throws SocketOptionException
        }
        catch (...)
        {
          delete connectionSocket;
          throw;
        }
				*connectedSocket = connectionSocket;
			}
			return true;
		}
		while (true);
	}

	TCPSocket::TCPSocket()
    : ConnectionSocket()
#ifdef DETECTED_OS_WINDOWS
    , m_IdleTimeInSec(7200) // per connection default (2 hours)
    , m_IntervalInSec(1) // per connection default (every second)
#endif
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);

		if (m_Socket == INVALID_SOCKET) 
		{
#ifdef DETECTED_OS_WINDOWS
			EndWinsock();
#endif
			throw SocketInitException("Could not create socket.", "socket", GetError());
		}
	}

	TCPSocket::TCPSocket(SOCKET socketDescriptor) : ConnectionSocket(socketDescriptor)
	{
	}

	TCPSocket::~TCPSocket()
	{
	}

  void TCPSocket::SetKeepalive(bool bKeepalive)
  {
    int optval = (bKeepalive ? 1 : 0);
    socklen_t optlen = sizeof(optval);
    if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&optval, optlen) == SOCKET_ERROR) throw SocketOptionException("Could not set keepalive option.", "setsockopt", GetError());
  }

  bool TCPSocket::GetKeepalive() const
  {
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "getsockopt", GetError());
    return optval != 0;
  }

  void TCPSocket::SetKeepaliveParameters(bool bKeepalive, uint32_t idleTimeInSec, uint32_t intervalInSec)
  {
#ifdef DETECTED_OS_WINDOWS
    tcp_keepalive optval;
    optval.onoff = bKeepalive;
    optval.keepalivetime = idleTimeInSec * 1000;
    optval.keepaliveinterval = intervalInSec * 1000;
    DWORD optlen = sizeof(optval);
    DWORD bytesReturned = 0;
    if (WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &optval, optlen, NULL, 0, &bytesReturned, NULL, NULL) == SOCKET_ERROR) throw SocketOptionException("Could not set keepalive parameters.", "WSAIoctl", GetError());
    m_IdleTimeInSec = idleTimeInSec;
    m_IntervalInSec = intervalInSec;
#elif defined (DETECTED_OS_LINUX)
    SetKeepalive(bKeepalive);
    int arg = idleTimeInSec; // send a keepalive packet after x seconds of inactivity
    if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, &arg, sizeof(arg)) == SOCKET_ERROR) throw SocketOptionException("Could not set idle keepalive parameter.", "setsockopt", GetError());
    arg = intervalInSec; // send a keepalive packet out every x seconds (after the idle period)
    if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, &arg, sizeof(arg)) == SOCKET_ERROR) throw SocketOptionException("Could not set interval keepalive parameter.", "setsockopt", GetError());
    arg = 10; // send up to 10 (default for Windows Vista and later that can not be changed) keepalive packets out, then disconnect if no response
    if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, &arg, sizeof(arg)) == SOCKET_ERROR) throw SocketOptionException("Could not set probe count keepalive parameter.", "setsockopt", GetError());
#endif
  }

  uint32_t TCPSocket::GetKeepaliveProbeCount() const
  {
#ifdef DETECTED_OS_WINDOWS
    // On Windows Vista and later, the number of keep-alive probes (data retransmissions) is set to 10 and cannot be changed.
    // On Windows Server 2003, Windows XP, and Windows 2000, the default setting for number of keep-alive probes is 5.
    return 10;    
#elif defined (DETECTED_OS_LINUX)
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "getsockopt", GetError());
    return optval;
#elif defined (DETECTED_OS_APPLE)
    return 8;
#else
    return 0;
#endif
  }
  
  uint32_t TCPSocket::GetKeepaliveIdleTime() const
  {
#ifdef DETECTED_OS_WINDOWS
    return m_IdleTimeInSec;
#elif defined (DETECTED_OS_LINUX)
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "getsockopt", GetError());
    return optval;
#elif defined (DETECTED_OS_APPLE)
    int curval = 0;
    size_t curvalsize = sizeof(curval);
    int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, 6 /*TCPCTL_KEEPIDLE*/ };
    if (sysctl(mib, 4, &curval, &curvalsize, NULL, 0) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "sysctl", GetError());
    return curval / 1000;
#else
    return 0;
#endif
  }

  uint32_t TCPSocket::GetKeepaliveInterval() const
  {
#ifdef DETECTED_OS_WINDOWS
    return m_IntervalInSec;
#elif defined (DETECTED_OS_LINUX)
    int optval = 0; // do not forget to initialize var
    socklen_t optlen = sizeof(optval);
    if (getsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, (char*)&optval, &optlen) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "getsockopt", GetError());
    return optval;
#elif defined (DETECTED_OS_APPLE)
    int curval = 0;
    size_t curvalsize = sizeof(curval);
    int mib[] = { CTL_NET, PF_INET, IPPROTO_TCP, 7 /*TCPCTL_KEEPINTVL*/ };
    if (sysctl(mib, 4, &curval, &curvalsize, NULL, 0) == SOCKET_ERROR) throw SocketOptionException("Could not query keepalive option.", "sysctl", GetError());
    return curval / 1000;
#else
    return 0;
#endif
  }

	// NOTE: we do not call shutdown for server/listen sockets as it will return WSAENOTCONN/ENOTCONN (socket not connected)
	// consequently, we do not need to call it for UDP sockets as well
	void TCPSocket::Disconnect()
	{
		//if (!m_bConnected || !IsConnected()) return; // to prevent WSAENOTCONN error (i.e. trying to disconnect a TCP socket never connected)
#ifdef DETECTED_OS_WINDOWS
		if (shutdown(m_Socket, SD_BOTH) == SOCKET_ERROR) throw SocketException("Could not disconnect.", "shutdown", GetError());
#else
		if (shutdown(m_Socket, SHUT_RDWR) == SOCKET_ERROR) throw SocketException("Could not disconnect.", "shutdown", GetError());
#endif
		ConnectionSocket::Disconnect();
	}

	TCPServer::TCPServer() : ListenSocket()
	{
		m_Socket = socket(AF_INET, SOCK_STREAM, 0);

		if (m_Socket == INVALID_SOCKET) 
		{
#ifdef DETECTED_OS_WINDOWS
			EndWinsock();
#endif
			throw SocketInitException("Could not create socket.", "socket", GetError());
		}
	}

	TCPServer::~TCPServer()
	{
	}

	ConnectionSocket * TCPServer::CreateConnectionSocket(SOCKET socketDescriptor)
	{
		TCPSocket * newSocket = new TCPSocket(socketDescriptor); // throws SocketInitException
		return newSocket;
	}

	UDPSocket::UDPSocket() : ConnectionSocket(), m_iMaxMessageSize(0)
	{
		m_Socket = socket(AF_INET, SOCK_DGRAM, 0);

		if (m_Socket == INVALID_SOCKET) 
		{
#ifdef DETECTED_OS_WINDOWS
			EndWinsock();
#endif
			throw SocketInitException("Could not create socket.", "socket", GetError());
		}

#ifdef DETECTED_OS_WINDOWS
		socklen_t size = sizeof(uint32_t);
		if (getsockopt(m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&m_iMaxMessageSize, &size) == SOCKET_ERROR) 
		{
			// free resources as destructor of this object will not get called if constructor fails (object has never been created)
			// same order as in destructor Socket::~Socket()
			try
			{
				Close();
			}
			catch (const SocketException &)
			{
			}
			// decrement our internal winsockCounter and possibly free resources
			EndWinsock();
			throw SocketInitException("Could not retrieve maximum datagram size.", "getsockopt", GetError());
		}
#else
		// figure out a way how to retrieve this properly here
		// should always be this value though
		m_iMaxMessageSize = 65507;
#endif
	}

	UDPSocket::~UDPSocket()
	{
	}

	uint32_t UDPSocket::SendDataImplementation(const int8_t * data, uint32_t size, const sockaddr * destination, socklen_t destinationSize, uint32_t & timeout)
	{
		uint32_t bytesSend = 0;
		do
		{
			uint32_t bytesToSend = size - bytesSend;
			uint32_t bytesToSendDatagram = (bytesToSend > m_iMaxMessageSize) ? m_iMaxMessageSize : bytesToSend;
			uint32_t bytesSendDatagram = ConnectionSocket::SendDataImplementation(data + bytesSend, bytesToSendDatagram, destination, destinationSize, timeout); // throws SocketException, SocketConnectionException
			bytesSend += bytesSendDatagram;
			if (bytesSendDatagram < bytesToSendDatagram) return bytesSend; // timeout
		}
		while (bytesSend < size);

		return bytesSend;
	}
}
