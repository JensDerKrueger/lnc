#ifndef IVDA_SOCKETS
#define IVDA_SOCKETS

#include "StdDefines.h"
#include <stdexcept>
#include <string>
#include <vector>

#ifdef DETECTED_OS_WINDOWS

// include Sockets.h and other files that include it before anything else that might include windows.h to make sure windwos.h is not included before winsock2.h
// see http://msdn.microsoft.com/en-us/library/ms737629%28VS.85%29.aspx
#ifdef _WINDOWS_
  #error windows.h included before Sockets.h
#endif

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wtsapi32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

// TODO
// exception types revamp -> done
// switch to NetworkAddress (see class below) instead of using local/peerAddress and local/peerPort -> done
// IOSocket extension with support for source/destination addresses (using NetworkAddress and GetNativeAddress()/SetNativeAddress() for performance reasons, instead of building sockaddr for each send) -> done
// UDPServer -> this will be removed/not implemented as functionality will be available in UDPSocket with IOSocket extension mentioned above -> done
// handle timeouts better in waitFor methods (due to accuracy issues/differences, i.e. timeout with 50, select might return 0 (timeout), but timer.Elapsed() returns 49.3, but we would expect 50+ to be returned) -> done
// probably add Bind method to Socket, otherwise localAddress/localPort makes no sense at this level -> done
// allow SO_SNDBUF and SO_RCVBUF to be set (IOSocket) -> done in Socket, SO_EXCLUSIVEADDRUSE (ListenSocket)
// add helper functions to convert between network and host byte order -> done
// remove timeout option from Send/Receive/Int/String methods as partial results and timeouts are a mess to handle consistenly here (see comments in code) -> done
// helper functions using gethostname/gethostbyname will fail on windows without WSAStartup, so they require a Socket being created before, which affects NetworkAddress in return which uses the helper functions (restructure this)
// for NetworkAddress, INADDR_BROADCAST = INADDR_NONE, so we might want to remove usage of INADDR_NONE at all
// right now, it is not possible to set TCP_NODELAY and SO_SNDBUF/SO_RCVBUF in TCPServer which makes sense code structure wise as a TCPServer never sends or receives data and thus does not require these settinsg itself
// but TCPSockets created by the server inherit these settings, so we might want to set them for the server so there is no need to individually set them for each TCPSocket the server produces
// more important: it may not be possible to set SO_SNDBUF/SO_RCVBUF after a TCPSocket has been connected, and sockets produced by the TCPServer are connected
// above is solved by moving the mentioned settings to a higher level -> done
// at some point, implement support for a proper graceful TCP shutdown as described here: http://msdn.microsoft.com/en-us/library/ms738547%28v=VS.85%29.aspx
// probably distinguish between hard close and graceful disconnect exception type wise
// use auto/smart pointers when using new and exceptions (see CreateConnectionSocket, AcceptNewConnection in ListenSocket)
// handle SIG_PIPE signal with signal handler -> done (differently)

namespace IVDA
{
	// Maximum transmission unit (MTU) which guarantees no datagram fragmentation from the IP layer
	// these may be higher for some devices/routers
	// to determine the actual (and possibly optimal) MTU on the path from one host to another, we woul have to do path MTU discovery via ICMP
	// this can be enabled for TCP, so TCP does it automatically and datagrams will not get fragmented (see http://linux.die.net/man/7/ip)
	// for UDP, we would have to do it on our own to prevent fragmentation
	// below are the minimum MTUs guaranteed to prevent fragmentation
	static const uint32_t ETHERNET_MTU = 1500;
	static const uint32_t IP4_MTU = 576;
	static const uint32_t IP6_MTU = 1280;

	// header size
	// for IP, this is the typical size and may be more (especially in IPv6 with header extensions)
	// also, encapsulating in other protocols on the path may introduce additional header overhead (i.e. PPPoE with additional 8 bytes)
	// see http://www.tecchannel.de/pc_mobile/linux/429773/tcp_ip_tuning_fuer_linux/index3.html, http://stackoverflow.com/questions/1098897/what-is-the-largest-safe-udp-packet-size-on-the-internet
	static const uint32_t IP4_HEADER_SIZE = 20;
	static const uint32_t IP6_HEADER_SIZE = 40;
	static const uint32_t TCP_HEADER_SIZE = 20;
	static const uint32_t UDP_HEADER_SIZE = 8;

	// Maximum segment size (MSS)
	// space available for user data guaranteeing no datagram fragmentation
	// calculated as MTU - (IP header size + protocol header size)
	// may use these when sending data (mainly intended for UDP, but can be used for TCP as well)
	// note: these are not related to the size limit of a datagram itself, which is 65507 (65535 - (20 + 8)) (also see GetMaxDatagramSize() in UDPSocket)
	// they are to prevent fragmentationb of a datagram of maximum this size
	// 576 - (20 + 20)
	static const uint32_t TCP_MSS = 536;
	// 1280 - (40 + 20)
	static const uint32_t TCP6_MSS = 1220;
	// 576 - (20 + 8)
	static const uint32_t UDP_MSS = 548;
	// 1280 - (40 + 8)
	static const uint32_t UDP6_MSS = 1232;

	// due to additional header overhead being possible (see header size above)
	// these are definitions regarded safe when sending data to prevent fragmentation
	static const uint32_t TCP_MSS_SAFE = 512;
	static const uint32_t TCP6_MSS_SAFE = 1024;
	static const uint32_t UDP_MSS_SAFE = 512;
	static const uint32_t UDP6_MSS_SAFE = 1024;
}

#ifdef DETECTED_OS_WINDOWS
typedef int socklen_t;
#else
typedef int SOCKET;
#endif

namespace IVDA
{
	// general error on a socket (may call Close after this)
	class SocketException : public std::runtime_error 
	{
	public:
		SocketException() : std::runtime_error(""), m_strOperation(""), m_iErrorCode(0) {}
		SocketException(const std::string & message, const std::string & operation, int errorCode) : std::runtime_error(message.c_str()), m_strOperation(operation), m_iErrorCode(errorCode) {}
		~SocketException() throw() {}
    
		const char * where() const { return m_strOperation.c_str(); }
		int withErrorCode() const { return m_iErrorCode; }
	protected:
		std::string m_strOperation;
		int m_iErrorCode;
	};

#ifdef DETECTED_OS_WINDOWS
	// call failed due to Winsock not being initialized, requires creating a socket instance
	// explicit initialization is not allowed by this API
	class WinsockUninitializedException : public SocketException
	{
	public:
		WinsockUninitializedException() : SocketException() {}
		WinsockUninitializedException(const std::string & message, const std::string & operation, int errorCode) : SocketException(message, operation, errorCode) {}
	};
#endif

	// error from constructor while initializing network subsystem (Windows) or a socket and its settings (socket will be NULL after this)
	class SocketInitException : public SocketException
	{
	public:
		SocketInitException() : SocketException() {}
		SocketInitException(const std::string & message, const std::string & operation, int errorCode) : SocketException(message, operation, errorCode) {}
	};

	// for ConnectionSocket: received graceful or hard disconnect (may call Disconnect and/or Close after this, do not call Connect again after this); for Connect/WaitForConnected method, connection attempt completed, but unsuccessfully (may call Connect again after this)
	// for ListenSocket: connection terminated before it could be accepted
	class SocketConnectionException : public SocketException
	{
	public:
		SocketConnectionException() : SocketException() {}
		SocketConnectionException(const std::string & message, const std::string & operation, int errorCode) : SocketException(message, operation, errorCode) {}
	};

	// could not set or get an option (when using the Set/Get calls)
	// may be critical depending on context
	class SocketOptionException : public SocketException
	{
	public:
		SocketOptionException() : SocketException() {}
		SocketOptionException(const std::string & message, const std::string & operation, int errorCode) : SocketException(message, operation, errorCode) {}
	};

	// IPv4 address
	// address and port should probably be separated at some point, which makes API design more easy
	// otherwise, we could add some convenience overloads for calls like Bind
	class NetworkAddress
	{
		friend class Socket;
		friend class IOSocket;
		friend class ConnectionSocket;

		struct sockaddr nativeAddress;

		void InitNativeAddress();
		const sockaddr & GetNativeAddress() const { return nativeAddress; }
		sockaddr & GetNativeAddressRW() { return nativeAddress; }
	public:
		enum SpecialAddress
		{
			Any,
			LocalHost,
			Broadcast // not used anywhere yet
		};

		// NOTE: addressBinary is always network order
		NetworkAddress();
		NetworkAddress(const std::string & ipAdressOrHostname);
		NetworkAddress(const std::string & ipAdressOrHostname, uint16_t port);
		NetworkAddress(SpecialAddress specialAddress);
		NetworkAddress(SpecialAddress specialAddress, uint16_t port);
		explicit NetworkAddress(uint32_t addressBinary);
		NetworkAddress(uint32_t addressBinary, uint16_t networkPort);
		~NetworkAddress();

		// on Windows, for the first call, when using a host name, an exception will be thrown if Winsock is not initialized (since GetIP is used internally)
		void SetAddress(const std::string & ipAdressOrHostname, uint16_t port); // throws WinsockUninitializedException
		void SetAddress(SpecialAddress specialAddress, uint16_t port);
		void SetAddress(uint32_t addressBinary, uint16_t networkPort);

		void GetAddress(std::string & address, uint16_t & port) const;
		void GetAddress(uint32_t & addressBinary, uint16_t & networkPort) const;

		bool operator==(const NetworkAddress & otherAddress) const;
		bool operator==(SpecialAddress specialAddress) const;
		bool operator!=(const NetworkAddress & otherAddress) const;
		bool operator!=(SpecialAddress specialAddress) const;
	};

	class Socket
	{
	protected:
#ifdef DETECTED_OS_WINDOWS
		static void StartWinsock(BYTE requestVersion, BYTE requestSubVersion);
		static void EndWinsock();
		bool m_bNonBlocking; // it is not possible to query blocking option for a socket under Windows
#endif
		int m_SendFlags;
		SOCKET m_Socket;

		Socket();
	public:
		virtual ~Socket();

		virtual void Bind(const NetworkAddress & address);
		virtual bool IsBound() const;
		virtual void Close(); // If there is still data waiting to be transmitted over the connection, normally close tries to complete this transmission.

		// helper functions
		// these won't throw exceptions and won't return specific error codes other than true or false
		// NOTE: ipBinary is always network order
		static bool IsIP(const std::string & ipAdressOrHostname);
		static bool IpToBinary(const std::string & ipString, uint32_t & ipBinary);
		static bool BinaryToIP(uint32_t ipBinary, std::string & ipString);
		// 64 bit versions work for LE and BE only, but there is almost nothing else
		static uint64_t NetworkInt64(uint64_t hostInt);
		static uint32_t NetworkInt32(uint32_t hostInt);
		static uint16_t NetworkInt16(uint16_t hostInt);
		static uint64_t HostInt64(uint64_t networkInt);
		static uint32_t HostInt32(uint32_t networkInt);
		static uint16_t HostInt16(uint16_t networkInt);

		// on Windows, these require a socket instance to work (so Winsock is initialized)
		// they will throw an exception if Winsock is not initialzed
		static bool GetIP(const std::string & hostname, uint32_t & ipBinary); // throws WinsockUninitializedException
		static bool GetIP(const std::string & hostname, std::string & ipString); // throws WinsockUninitializedException
		// returns a NetworkAddress list of non-loop-back interfaces on the machine (port is unspecified)
		static bool GetMyNetworkAddresses(std::vector<NetworkAddress> & networkAddresses); // throws WinsockUninitializedException

		// non-blocking mode
		// if a socket is non-blocking, timeout methods work as follows: -1/UINT32_MAX for no timeout (blocking call), > 0 allows at most timeout to complete, 0 for immediate return (non-blocking call)
		// otherwise, timeout will be ignored (internally, this means select will never be called to wait for readyRead/readyWrite)
		void SetNonBlocking(bool bNonBlocking);
		bool GetNonBlocking() const;
		// this can be called on Linux to get around the TIME_WAIT state after closing a socket (see http://www.ibm.com/developerworks/library/l-sockpit/, 
		// http://www.serverframework.com/asynchronousevents/2011/01/time-wait-and-its-design-implications-for-protocols-and-scalable-servers.html)
		// also, on Linux, if there is a listening socket already bound to the address, bind will fail even with the reuse option set to true
		// on Windows, this exception seems not to apply (could verifiy this at some point)
		// there may also be a SO_REUSEPORT flag on some systems (http://www.unixguide.net/network/socketfaq/4.11.shtml)
		// note: on some systems, this could also be handled using the SO_LINGER option
		// but generally, SO_LINGER is not intended for this usage; it may be interpreted differently across implementations and some systems do not support it at all
		// e.g. see http://unlser1.unl.csi.cuny.edu/faqs/sock-faq/html/unix-socket-faq.html, http://developerweb.net/viewtopic.php?id=2982
		void SetReuseAddress(bool bReuseAddress);
		bool GetReuseAddress() const;
    // NOTE: only available under Windows
    void SetExclusiveAddressUse(bool bExclusiveAddress);
    bool GetExclusiveAddressUse() const;
		// TCP window scale options
    // we have these at this level so a ListenSocket may set it and ConnectionSockets created by the server will inherit the settings
		// ListenSocket (i.e. TCPServer) itself does not need these, only IOSockets are affected
		// for UDP, SO_SNDBUF is only a upper bound for the maximum size of datagrams
		void SetSendBufferSize(int32_t size);
		void SetReceiveBufferSize(int32_t size);
		// note: these may not return what was set in the Set-methods
		// i.e. on Linux, the number set will get doubled and returned here, which is normal behavior there
		int32_t GetSendBufferSize() const;
		int32_t GetReceiveBufferSize() const;

		// optimize socket for no delay
		// i.e. applicable for TCP, which is the default implementation here
		// for TCP, disables Nagle's algorithm
		// we do not need this setting for TCPServer itself
		// but we have it at this level so TCPSockets created by TCPServer may inherit this setting from the server
		// sockets not supporting this or supporting it differently should overwrite SetNoDelay (see UDPSocket)
		void SetNoDelay(bool bNoDelay);
		bool GetNoDelay() const;

		// this is not applicable to ListenSocket (not connected, can not read/write), IOSocket and UDPSocket (not connected)
		// but we have it at this level so a ListenSocket may set it and ConnectionSockets created by the server will inherit the setting
		// this is not applicable to Windows
		void SetNoSigPipe(bool bNoSigPipe);
		bool GetNoSigPipe() const;

		NetworkAddress GetLocalNetworkAddress() const;
		std::string GetLocalAddress() const { std::string address; uint16_t port; GetLocalNetworkAddress().GetAddress(address, port); return address; }
		uint16_t GetLocalPort() const { std::string address; uint16_t port; GetLocalNetworkAddress().GetAddress(address, port); return port; }

		void pai();
	};

	class IOSocket : public Socket
	{
	protected:
		IOSocket();

		// in general, probably do not throw exceptions with methods like select, getsockopt, etc., as they return few errors and subsequent calls to accept, send, recv, etc. should fail afterwards anyway and throw their exceptions
		// (WSAEFAULT with select might result in subsequent calls not failing though, but then we may not need an exception as well)
		// NOTE: ListenSocket needs IsReadyToReadData as well, so maybe put this on top in Socket; right now, server has a copy of this method named WaitForNewConnection
		bool IsReadyToReadData(uint32_t & timeout);
		bool IsReadyToWriteData(uint32_t & timeout);

		virtual uint32_t SendDataImplementation(const int8_t * data, uint32_t size, const sockaddr * destination, socklen_t destinationSize, uint32_t & timeout);
		virtual uint32_t ReceiveDataImplementation(int8_t * data, uint32_t size, sockaddr * source, socklen_t * sourceSize, uint32_t & timeout);
	public:
		virtual ~IOSocket();

		uint32_t SendData(const int8_t * data, uint32_t size, uint32_t timeout = INFINITE_TIMEOUT);
		uint32_t ReceiveData(int8_t * data, uint32_t size, uint32_t timeout = INFINITE_TIMEOUT);
		uint32_t SendData(const int8_t * data, uint32_t size, const NetworkAddress & destination, uint32_t timeout = INFINITE_TIMEOUT);
		uint32_t ReceiveData(int8_t * data, uint32_t size, NetworkAddress & source, uint32_t timeout = INFINITE_TIMEOUT);
	
		// below do not support destination/source for unconnected sockets like UDP, so especially do not use below when using UDPSocket as a server
		// they however work with a virtually connected UDPSocket
		// unsafe methods with timeout, see comments in code
		// therefore, no timeout allowed here
		// only use those if sure this will work OK, generally unsafe (especially string functions) for several reasons explained in comments below and in code (most obvious they are always blocking of course)
		uint32_t SendInt(int32_t integer);
		uint32_t ReceiveInt(int32_t & integer);
		// those allow at most FILENAME_MAX sized strings, otherwise we would have to handle bad_alloc exceptions in ReceiveString
		// they return 0 if string is too large, which does not go along well with this API, but this is a special case and we do not want to throw a SocketException here
		// -> as above does not work well in ReceiveString (after returning 0, we already have read the size, so how can we recover?), there is no string length limit
		// and ReceiveString may return a bad_alloc exception
		uint32_t SendString(const std::string & string);
		uint32_t ReceiveString(std::string & string);

		template <class T> uint32_t Send(const T & object, uint32_t timeout = INFINITE_TIMEOUT) 
		{
			return SendDataImplementation((const int8_t*)&object, sizeof(T), NULL, 0, timeout); // throws SocketException, SocketConnectionException
		}
		template <class T> uint32_t Receive(T & object, uint32_t timeout = INFINITE_TIMEOUT)
		{
			return ReceiveDataImplementation((int8_t*)&object, sizeof(T), NULL, NULL, timeout); // throws SocketException, SocketConnectionException
		}
	};

	class ConnectionSocket : public IOSocket
	{
		friend class ListenSocket;
	protected:
		bool m_bConnectionInProgress;
		bool m_bConnected;

		ConnectionSocket();
		// this is used to create new ConnectionSocket's in ListenSocket implementation classes (need to be friend class of ConnectionSocket implementation class)
		ConnectionSocket(SOCKET socketDescriptor);

	public:
		virtual ~ConnectionSocket();

		// NOTE: this is an asynchronous call for non-blocking sockets
		// if a timeout occurs, it may still continue to complete in the background
		// use WaitForConnected() to check for completion before trying to connect again
		// both methods work the following: timeout (false) means connection attempt still continuing in the background, true success, SocketConnectionException for failed (Connect can safely be called again after this), 
		// SocketException on general error or a failed connection attempt, where another Connect call with the same parameters will fail again (best to close socket)
		// Connect will throw an exception due to connect returning with WSAEALREADY (WaitForConnected() returning with timeout, connection attempt ongoing, may call WaitForConnected() again) or WSAEISCONN (WaitForConnected() returning with success, already connected)
		virtual bool Connect(const NetworkAddress & address, uint32_t timeout = INFINITE_TIMEOUT);
		virtual bool WaitForConnected(uint32_t timeout = INFINITE_TIMEOUT);
		virtual bool IsConnected();
    
    virtual bool IsConnecting() const {
      return m_bConnectionInProgress;
    }
    
		// socket may not reusable after this and should not be reconnected, so it should be discarded
		// for UDP obviously, this has no effect
		virtual void Disconnect(); // probably add timeout here to indicate how long to wait for possibly still pending data being send, requires full reimplementation
		// overwrite to keep m_bConnected consistent, since only calling Close will implicitly disconnect if this has not happened before
		// this way, we prevent Disconnect from being called in the destructor raising an WSAENOTCONN/ENOTCONN exception
		virtual void Close();

		NetworkAddress GetPeerNetworkAddress() const;
		std::string GetPeerAddress() const { std::string address; uint16_t port; GetPeerNetworkAddress().GetAddress(address, port); return address; }
		uint16_t GetPeerPort() const { std::string address; uint16_t port; GetPeerNetworkAddress().GetAddress(address, port); return port; }
	};
  
	class ListenSocket : public Socket
	{
	protected:
		bool m_bListening;

		ListenSocket();

		virtual ConnectionSocket * CreateConnectionSocket(SOCKET socketDescriptor) = 0;
		bool WaitForNewConnection(uint32_t & timeout);
	public:
		virtual ~ListenSocket();

		virtual void Listen(uint32_t backlog = std::numeric_limits<uint32_t>::max());
		virtual bool IsListening();
		// caller is responsible to delete connectedSocket
		// this may throw SocketException, SocketInitException, SocketConnectionException (and SocketOptionException on Linux)
		virtual bool AcceptNewConnection(ConnectionSocket ** connectedSocket, uint32_t timeout = INFINITE_TIMEOUT);
	};

	class TCPSocket : public ConnectionSocket
	{
		friend class TCPServer;

#ifdef DETECTED_OS_WINDOWS
    uint32_t m_IdleTimeInSec;
    uint32_t m_IntervalInSec;
#endif

		TCPSocket(SOCKET socketDescriptor);
	public:
		TCPSocket();
		~TCPSocket();

    // enable keepalive to detect broken connection (e.g. network cable detached)
    void SetKeepalive(bool bKeepAlive);
    bool GetKeepalive() const;

    // @idleTimeInSec send a keepalive packet after x seconds of inactivity
    // @intervalInSec send a keepalive packet out every x seconds (after the idle period)
    // NOTE: not available for Apple OSs (throws SocketOptionException)
    void SetKeepaliveParameters(bool bKeepalive, uint32_t idleTimeInSec = 10, uint32_t intervalInSec = 1);

    uint32_t GetKeepaliveProbeCount() const;
    uint32_t GetKeepaliveIdleTime() const;
    uint32_t GetKeepaliveInterval() const;

    // returns the overall keepalive time in seconds before any broken connection will be detected
    uint32_t GetKeepaliveTime() const { return GetKeepaliveIdleTime() + GetKeepaliveInterval() * GetKeepaliveProbeCount(); }

		// we overwrite as TCP should use shutdown
		void Disconnect();
	};

	class TCPServer : public ListenSocket
	{
		ConnectionSocket * CreateConnectionSocket(SOCKET socketDescriptor);
	public:
		TCPServer();
		~TCPServer();
	};

	// this is only a virtual connection here
	// this can be used as a non-virtually-connected UDP socket and as a UDP server
	class UDPSocket : public ConnectionSocket
	{
		uint32_t m_iMaxMessageSize;

		// we overwrite as we need to do splitting into several datagrams with UDP if data to send is larger than m_iMaxMessageSize
		// TCP does this automatically
		// we DO NOT need to do this when receiving, as we at most receive one datagram with a maximum size of m_iMaxMessageSize per recvfrom call
		uint32_t SendDataImplementation(const int8_t * data, uint32_t size, const sockaddr * destination, socklen_t destinationSize, uint32_t & timeout);
	public:
		UDPSocket();
		~UDPSocket();

		uint32_t GetMaxDatagramSize() const { return m_iMaxMessageSize; }

		// for UDP, we have no actual connection, so we assume we are "connected" after a successful Connect call
		bool IsConnected() { return m_bConnected; }

		// no delay option not applicable to UDP
		void SetNoDelay(bool) {}
	};
}

#endif
