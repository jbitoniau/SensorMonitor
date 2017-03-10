#include "UDPSocket.h"

#include <assert.h>

#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	#include <winsock.h>         // For socket(), connect(), send(), and recv()
	typedef int socklen_t;
	typedef char raw_type;       // Type used for raw data on this platform
	#include <iphlpapi.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <stdio.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <errno.h>
	#include <arpa/inet.h>
	#include <fcntl.h>
	#include <cstring>	    // for memset
    #include <ifaddrs.h>
	#include <unistd.h>
	typedef void raw_type;  // Type used for raw data on this platform
#endif

namespace Loco
{

UDPSocket::LocalHostInfo UDPSocket::mLocalHostInfo;
	
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS

// Inspired from 
// http://msdn.microsoft.com/en-us/library/aa366028%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/aa366351(v=vs.85).aspx
bool UDPSocket::windowsGetLocalHostName( std::string& localHostName )
{
	FIXED_INFO* pFixedInfo;
    ULONG ulOutBufLen;
    DWORD dwRetVal;
    pFixedInfo = (FIXED_INFO*)malloc( sizeof(FIXED_INFO) );
    ulOutBufLen = sizeof(FIXED_INFO);
	if ( GetNetworkParams(pFixedInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW ) 
	{
        free(pFixedInfo);
        pFixedInfo = (FIXED_INFO *) malloc(ulOutBufLen);
        if (pFixedInfo == NULL) 
		{
			//printf("Error allocating memory needed to call GetNetworkParams\n");
			return false;		// error
        }
    }

    if ( dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen) != NO_ERROR) 
	{
        //printf("GetNetworkParams failed with error %d\n", dwRetVal);
        if (pFixedInfo) 
		{
            free(pFixedInfo);
        }
		return false; // error
    }     
		
	localHostName = pFixedInfo->HostName;		// Other info in pFixedInfo include DomainName, DnsServerList, etc...
	
	if (pFixedInfo) 
	{
		free(pFixedInfo);
        pFixedInfo = NULL;
	}
	return true;
}

// Inspired from 
// http://msdn.microsoft.com/en-us/library/aa366028%28v=VS.85%29.aspx
// http://msdn.microsoft.com/en-us/library/aa366309(v=vs.85).aspx
bool UDPSocket::windowsGetLocalIPAddresses( std::string& ipAddress, std::string& ipMask, std::string& broadcastIPAddress )
{
	MIB_IPADDRTABLE  *pIPAddrTable;
	DWORD            dwSize = 0;
	DWORD            dwRetVal;

	pIPAddrTable = (MIB_IPADDRTABLE*) malloc( sizeof(MIB_IPADDRTABLE) );
	if ( GetIpAddrTable(pIPAddrTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER ) 
	{
		free( pIPAddrTable );
		pIPAddrTable = (MIB_IPADDRTABLE *) malloc ( dwSize );
	}
	
	if ( (dwRetVal = GetIpAddrTable( pIPAddrTable, &dwSize, 0 )) != NO_ERROR ) 
	{ 
		//printf("GetIpAddrTable call failed with %d\n", dwRetVal);
		return false;
	}

	in_addr l_IPAddress;
	l_IPAddress.S_un.S_addr = pIPAddrTable->table[0].dwAddr;
	ipAddress = inet_ntoa(l_IPAddress);
	
	in_addr l_IPMask;
	l_IPMask.S_un.S_addr = pIPAddrTable->table[0].dwMask;
	ipMask = inet_ntoa(l_IPMask);

	// The dwBCastAddr member doesn't seem to provide the expected broadcast address
	// We calculate it ourselves here
	in_addr l_BroadcastIPAddress;
	l_BroadcastIPAddress.S_un.S_addr = ~l_IPMask.S_un.S_addr | l_IPAddress.S_un.S_addr; 
	broadcastIPAddress = inet_ntoa( l_BroadcastIPAddress );

	if (pIPAddrTable)
		free(pIPAddrTable);

	return true;
}

#else

bool UDPSocket::nonWindowsGetLocalHostName( std::string& localHostName )
{
    // This code also works for Windows
    localHostName = "";
	const int maxHostNameLen = 256;
	char hostName[maxHostNameLen];
	hostName[0] = 0;
	if ( gethostname( hostName, maxHostNameLen )!=0 )
        return false;
    
    localHostName = hostName;
	return true;
}


bool UDPSocket::nonWindowsGetIPAddresses( std::string& ipAddress, std::string& ipMask, std::string& broadcastIPAddress )
{
    ipAddress = "";
    ipMask = "";
    broadcastIPAddress = "";
    
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;

	// retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0)
	{
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL)
		{
            if(temp_addr->ifa_addr->sa_family == AF_INET)
			{
                if ( strcmp( temp_addr->ifa_name, "en0")==0 ||
					 strcmp( temp_addr->ifa_name, "eth0")==0 )
                {
                    ipAddress = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr);
                    ipMask = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_netmask)->sin_addr);
                    broadcastIPAddress = inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_dstaddr)->sin_addr);
                    break;
                }
            }
            temp_addr = temp_addr->ifa_next;
        }
    }
    else 
    {
        return false;
    }
    
    // Free memory
    freeifaddrs(interfaces);
    return true;
}
#endif

// From http://www.benripley.com/development/ios/udp-broadcasting-on-iphone-using-bsd-sockets/
bool UDPSocket::mIsInitialized = false;

void UDPSocket::initialize()
{
    if ( mIsInitialized )
		return;

#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 0);
    if ( WSAStartup(wVersionRequested, &wsaData)!=0 )
	printf("Failed to start WinSock");
#endif

	updateLocalHostInfo();

    mIsInitialized = true;
}

void UDPSocket::updateLocalHostInfo()
{
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	bool ret;
	ret = windowsGetLocalHostName( mLocalHostInfo.Name );
	assert(ret);
	ret = windowsGetLocalIPAddresses( mLocalHostInfo.IPAddress, mLocalHostInfo.IPMask, mLocalHostInfo.BroadcastIPAddress );
	assert(ret);
#else
    bool ret;
    ret = nonWindowsGetLocalHostName( mLocalHostInfo.Name );
    assert(ret);
    ret = nonWindowsGetIPAddresses( mLocalHostInfo.IPAddress, mLocalHostInfo.IPMask, mLocalHostInfo.BroadcastIPAddress );
    assert(ret);
#endif
}

void UDPSocket::shutDown()
{
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	if (WSACleanup() != 0)
		printf("Failed to stop WinSock");
#endif
    mIsInitialized = false;
}

UDPSocket::UDPSocket( unsigned short int _LocalPort )
	:   mDefaultSendBufferSize(0),
		mDefaultReceiveBufferSize(0),
		mLocalPort( _LocalPort ),
		mSocketDesc( -1 )
{
	initialize();

    mSocketDesc = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( mSocketDesc<0 )
		printf("Failed to create socket\n");
	
    // Set the socket as non blocking on reception
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	unsigned long argp = 1;
	ioctlsocket( mSocketDesc, FIONBIO, &argp );
#else
    fcntl( mSocketDesc, F_SETFL, O_NONBLOCK | FASYNC );
#endif

    // Bind the socket to the port
    sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons( mLocalPort );

    if ( bind(mSocketDesc, (sockaddr*)&localAddr, sizeof(sockaddr_in) ) < 0)
		printf("Failed to bind socket to the specified port");

    // Enable broadcast
    int broadcastEnabled = 1;
    if ( setsockopt(mSocketDesc, SOL_SOCKET, SO_BROADCAST, (raw_type*)&broadcastEnabled, sizeof(broadcastEnabled))<0 )
		printf("Failed to enable broadcast on socket");

	mDefaultSendBufferSize = getSendBufferSizeInBytes();
	mDefaultReceiveBufferSize = getReceiveBufferSizeInBytes();
	
//	printf("default send buffer size: %d\n", m_DefaultSendBufferSize );
//	printf("default receive buffer size: %d\n", m_DefaultReceiveBufferSize );
}

UDPSocket::~UDPSocket()
{
#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	::closesocket(mSocketDesc);
#else
	::close(mSocketDesc);
#endif
	mSocketDesc = -1;
}

int UDPSocket::receive( char* data, unsigned int dataSizeInBytes, std::string& sourceAddress, unsigned short int& sourcePort)
{
    sockaddr_in sourceAddressStruct;
	socklen_t sourceAddressStructSize = sizeof(sourceAddressStruct);
	int numBytesReceived = recvfrom(mSocketDesc, (raw_type*)data, dataSizeInBytes, 0, (sockaddr *)&sourceAddressStruct, (socklen_t *)&sourceAddressStructSize);
    sourceAddress = inet_ntoa(sourceAddressStruct.sin_addr);
	sourcePort = ntohs(sourceAddressStruct.sin_port);
	
	return numBytesReceived;
}

unsigned int UDPSocket::receive2( char* data, unsigned int dataSizeInBytes, std::string& sourceAddress, unsigned short int& sourcePort, int& errorCode )
{
	errorCode = 0;
	unsigned int numBytesReceived = 0;
	int ret = receive( data, dataSizeInBytes, sourceAddress, sourcePort );
	if ( ret>=0 )
	{
		numBytesReceived = ret;
	}
	else
	{
#if _WIN32
		// On Windows, when a non-blocking socket doesn't have anything to return to the receive method,
		// it returns -1 bytes received and the error code is 0.
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
		errorCode = WSAGetLastError();
#else
		// On Linux (Raspberry Pi), the non-blocking socket will return -1 bytes received and EAGAIN as error code
		// errno is in /usr/include/asm-generic/errno.h
		errorCode = errno;
		if ( errorCode==EAGAIN )
			errorCode = 0;
#endif
	}
	return numBytesReceived;
}

int UDPSocket::send( const char* data, unsigned int dataSizeInBytes, const std::string& destinationAddress, unsigned short int destinationPort )
{
	sockaddr_in destAddr;
    fillSockAddr(destinationAddress, destinationPort, destAddr);

    int numBytesSent = sendto(mSocketDesc, (raw_type*)data, dataSizeInBytes, 0,(sockaddr *) &destAddr, sizeof(destAddr));
	return numBytesSent;
}

// Returns the number of bytes sent (which might be less than requested!)
// If the emission failed, the errorCode is set. It's zero otherwise
unsigned int UDPSocket::send2( const char* data, unsigned int dataSizeInBytes, const std::string& destinationAddress, unsigned short int destinationPort, int& errorCode )
{
	errorCode = 0;
	unsigned int numBytesSent = 0;
	int ret = send( data, dataSizeInBytes, destinationAddress, destinationPort );
	if ( ret>=0 )
	{
		numBytesSent = ret;
	}
	else
	{
#if _WIN32
		// Windows: if no error occurs, sendto returns the total number of bytes sent, which can be less than the number indicated by len. 
		// Otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms740668(v=vs.85).aspx
		errorCode = WSAGetLastError();
#else
		// Linux: Upon successful completion, sendto() shall return the number of bytes sent. Otherwise, -1 shall be returned and 
		// errno set to indicate the error.
		// errno is in /usr/include/asm-generic/errno.h
		errorCode = errno;
#endif
	}
	return numBytesSent;
}

unsigned int UDPSocket::getSendBufferSizeInBytes() const
{
	unsigned int bufferSize = 0;
	bool ret = getSocketSendBufferSize( mSocketDesc, bufferSize );
	assert( ret );
	return bufferSize;
}

bool UDPSocket::setSendBufferSizeInBytes( unsigned int bufferSize )
{
//printf("setting send buffer size: %d", bufferSize );
	bool ret = setSocketSendBufferSize( mSocketDesc, bufferSize );
	assert( ret==0 );
//	assert( GetSendBufferSizeInBytes()==bufferSize );		// JBM: the resulting size isn't necessarily the one requested. Linux for eg, allocates twice as much size 
//printf("done!\n");
	return (ret==0);
}

unsigned int UDPSocket::getReceiveBufferSizeInBytes() const
{
	unsigned int bufferSize = 0;
	bool ret = getSocketReceiveBufferSize( mSocketDesc, bufferSize );
	assert( ret );
	return bufferSize;
}

bool UDPSocket::setReceiveBufferSizeInBytes( unsigned int bufferSize )
{
//printf("setting receive buffer size: %d", bufferSize );
	bool ret = setSocketReceiveBufferSize( mSocketDesc, bufferSize );
	assert( ret==0 );
//	assert( GetReceiveBufferSizeInBytes()==bufferSize );		// JBM: the resulting size isn't necessarily the one requested. Linux for eg, allocates twice as much size 
//printf("done!\n");
	return (ret==0);
}

void UDPSocket::fillSockAddr( const std::string& address, unsigned short int port, sockaddr_in& addrStruct )
{
    memset( &addrStruct, 0, sizeof(sockaddr_in));
    addrStruct.sin_family = AF_INET;

	hostent* host = gethostbyname(address.c_str());
    if ( !host )
    {
        printf("Failed to resolve host name");
        return;
	}
	addrStruct.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);
	addrStruct.sin_port = htons(port);
}

bool UDPSocket::getSocketSendBufferSize( int socketDesc, unsigned int& sendBufferSize )
{
	sendBufferSize = 0;
	unsigned int bufferSize = 0;
	char* optionValue = reinterpret_cast<char*>(&bufferSize);
	socklen_t optionSize = sizeof( bufferSize );
	int ret = getsockopt( socketDesc, SOL_SOCKET, SO_SNDBUF, optionValue, &optionSize );
	if ( ret!=0 )
	{
		assert(false);
		return false;
	}
	sendBufferSize = bufferSize;
	return true;
}

bool UDPSocket::setSocketSendBufferSize( int socketDesc, unsigned int sendBufferSize )
{
	const char* optionValue = reinterpret_cast<const char*>(&sendBufferSize);
	int optionSize = sizeof(sendBufferSize);
	int ret = setsockopt( socketDesc, SOL_SOCKET, SO_SNDBUF, optionValue, optionSize );
	if ( ret!=0 )
	{
		assert(false);
		return false;
	}
	return false;
}

bool UDPSocket::getSocketReceiveBufferSize( int socketDesc, unsigned int& receiveBufferSize )
{
	receiveBufferSize = 0;
	unsigned int bufferSize = 0;
	char* optionValue = reinterpret_cast<char*>(&bufferSize);
	socklen_t optionSize = sizeof( bufferSize );
	int ret = getsockopt( socketDesc, SOL_SOCKET, SO_RCVBUF, optionValue, &optionSize );
	if ( ret!=0 )
	{
		assert(false);
		return false;
	}
	receiveBufferSize = bufferSize;
	return true;
}

bool UDPSocket::setSocketReceiveBufferSize( int socketDesc, unsigned int receiveBufferSize )
{
	const char* optionValue = reinterpret_cast<const char*>(&receiveBufferSize);
	int optionSize = sizeof(receiveBufferSize);
	int ret = setsockopt( socketDesc, SOL_SOCKET, SO_RCVBUF, optionValue, optionSize );
	if ( ret!=0 )
	{
		assert(false);
		return false;
	}
	return false;
}

// Returns the first known IP address corresponding to a host name
std::string	UDPSocket::hostNameToAddress( const std::string& hostName )
{
	initialize();

	hostent* host = gethostbyname( hostName.c_str() );
	if ( host == NULL ||
	host->h_addrtype != AF_INET ||
	host->h_addr_list == 0 )
	{
		printf("Failed to resolve name to address");
		return std::string();
	}

	struct in_addr addr;
	memset( &addr, 0, sizeof(addr) );
	addr.s_addr = *(unsigned long *) host->h_addr_list[0];		// Only consider the 0th address

    std::string address = inet_ntoa(addr);
	return address;
}

std::string	UDPSocket::addressToHostName( const std::string& address )
{
	initialize();

	sockaddr_in addr;
	fillSockAddr( address, 0, addr );

	hostent* host = gethostbyaddr( (char*)&addr.sin_addr, 4, AF_INET );
	if ( host == NULL )
	{
		printf("Failed to resolve address to name");
	}
	return host->h_name;
}

}
