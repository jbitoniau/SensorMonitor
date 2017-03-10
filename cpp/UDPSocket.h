#pragma once

#include <string>
#include "Platform.h"

struct sockaddr_in;

namespace Loco
{

/*
	UDPSocket

	A non-blocking UDP socket...
*/
class UDPSocket
{
public:
    UDPSocket( unsigned short int localPort );
    virtual ~UDPSocket();
    
	unsigned short int		getLocalPort() const { return mLocalPort; }

    int						receive( char* data, unsigned int dataSizeInBytes, std::string& sourceAddress, unsigned short int& sourcePort );
	unsigned int			receive2( char* data, unsigned int dataSizeInBytes, std::string& sourceAddress, unsigned short int& sourcePort, int& errorCode );
	
	int						send( const char* data, unsigned int dataSizeInBytes, const std::string& destinationAddress, unsigned short int destinationPort );
	unsigned  int			send2( const char* data, unsigned int dataSizeInBytes, const std::string& destinationAddress, unsigned short int destinationPort, int& errorCode );

	unsigned int			getSendBufferSizeInBytes() const;
	unsigned int			getDefaultSendBufferSizeInBytes() const					{ return mDefaultSendBufferSize; }
	bool					setSendBufferSizeInBytes( unsigned int bufferSize );
	
	unsigned int			getReceiveBufferSizeInBytes() const;
	unsigned int			getDefaultReceiveBufferSizeInBytes() const				{ return mDefaultReceiveBufferSize; }
	bool					setReceiveBufferSizeInBytes( unsigned int bufferSize );

    static void				initialize();
	static bool				isInitialized()				{ return mIsInitialized; }
    static void				shutDown();
    
	static std::string		getLocalHostName()			{ initialize(); return mLocalHostInfo.Name; }
	static std::string		getLocalAddress()	/* ! */ { initialize(); return mLocalHostInfo.IPAddress; }
	static std::string		getLocalIPMask()			{ initialize(); return mLocalHostInfo.IPMask; }
	static std::string		getBroadcastIPAddress()		{ initialize(); return mLocalHostInfo.BroadcastIPAddress; }

	static std::string		hostNameToAddress( const std::string& hostName );
	static std::string		addressToHostName( const std::string& address );
	
private:
	struct LocalHostInfo
	{
		std::string	Name;
		std::string	IPAddress;
		std::string	IPMask;
		std::string	BroadcastIPAddress;
	};

	static LocalHostInfo	mLocalHostInfo;
	static void				updateLocalHostInfo();

    static void				fillSockAddr( const std::string& address, unsigned short int port, sockaddr_in& addrStruct );
    static bool				getSocketSendBufferSize( int socketDesc, unsigned int& sendBufferSize );
	static bool				setSocketSendBufferSize( int socketDesc, unsigned int sendBufferSize );
	static bool				getSocketReceiveBufferSize( int socketDesc, unsigned int& receiveBufferSize );
	static bool				setSocketReceiveBufferSize( int socketDesc, unsigned int receiveBufferSize );

    static bool				mIsInitialized;
    
	unsigned int			mDefaultSendBufferSize;
	unsigned int			mDefaultReceiveBufferSize;
    unsigned short int		mLocalPort;
    int						mSocketDesc;

#if LOCO_PLATFORM==LOCO_PLATFORM_WINDOWS
	static bool				windowsGetLocalHostName( std::string& localHostName );
	static bool				windowsGetLocalIPAddresses( std::string& ipAddress, std::string& ipMask, std::string& broadcastIPAddress );
#else	
    static bool				nonWindowsGetLocalHostName( std::string& localHostName );
    static bool				nonWindowsGetIPAddresses( std::string& ipAddress, std::string& ipMask, std::string& broadcastIPAddress );
#endif
};

}
