//#include "test_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <typeinfo>

#include <assert.h>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#define PORT_SERVER			27200	// Default server port, UDP/TCP

// It's 2021 and the C language doesn't have a cross-platform way to
// compare strings in a case-insensitive way
#ifdef _MSC_VER
	#define strcasecmp(a,b) stricmp(a,b)
#endif

/////////////////////////////////////////////////////////////////////////////
//
// Fuzzing stuff
//
// I have opted for a usage-based forkserver approach since the api is liable
// to change and the C-interface is not easy to use or guranteed to remain 
// stable.
//
/////////////////////////////////////////////////////////////////////////////

static SteamNetworkingMicroseconds g_logTimeZero;

enum Forks {
	CreateConnectName,
	CreateP2P,
	TypeSteamNetworkingIPAddr
};

static void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg )
{
	SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;

	if ( eType <= k_ESteamNetworkingSocketsDebugOutputType_Msg )
	{
		printf( "%10.6f %s\n", time*1e-6, pszMsg );
		fflush(stdout);
	}
	if ( eType == k_ESteamNetworkingSocketsDebugOutputType_Bug )
	{
		fflush(stdout);
		fflush(stderr);

		// !KLUDGE! Our logging (which is done while we hold the lock)
		// is occasionally triggering this assert.  Just ignroe that one
		// error for now.
		// Yes, this is a kludge.
		if ( strstr( pszMsg, "SteamNetworkingGlobalLock held for" ) )
			return;

		assert( !"TEST FAILED" );
	}
}

void socket_init(){
		SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Debug, DebugOutput );
		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_LogLevel_P2PRendezvous, k_ESteamNetworkingSocketsDebugOutputType_Debug );
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
		{
			fprintf( stderr, "GameNetworkingSockets_Init failed.  %s", errMsg );
			exit(1);
		}
}

static void ForkCreateConnectName(SteamNetworkingIPAddr* bindServerAddress, SteamNetworkingIPAddr* connectToServerAddress, const char * name)
{
	socket_init();
	static HSteamListenSocket g_hSteamListenSocket = k_HSteamListenSocket_Invalid;
	static HSteamNetConnection m_hSteamNetConnection = k_HSteamNetConnection_Invalid;
	//SteamNetworkingIPAddr connectToServerAddress;
	//connectToServerAddress.SetIPv4( 0x7f000001, PORT_SERVER );

	
	//debug
	//(*bindServerAddress).Clear();
	//(*bindServerAddress).m_port = 22;

	//buffer
	char* buf;
	buf = (char*)malloc(bindServerAddress->k_cchMaxString*sizeof(char));

	//target lines
	printf("Making socket object\n");
	ISteamNetworkingSockets *pSteamSocketNetworking = SteamNetworkingSockets();
	printf("Creating ListenSocket with IP\n");
	//std::cout << typeid(bindServerAddress).name() << '\n';
	(*bindServerAddress).ToString(buf, (*bindServerAddress).k_cchMaxString, true);
	printf("Address: %s\n", buf);
	//std::cout << buf << '\n';
	g_hSteamListenSocket = pSteamSocketNetworking->CreateListenSocketIP( *bindServerAddress, 0, nullptr ); //TODO options would be nice
	printf("Connecting by IP address\n");
	m_hSteamNetConnection = pSteamSocketNetworking->ConnectByIPAddress( *connectToServerAddress, 0, nullptr ); //TODO same ^
	printf("Set connection name\n");
	pSteamSocketNetworking->SetConnectionName( m_hSteamNetConnection, name);
	printf("Test Finished\n");
}

static void ForkCreateP2P(SteamNetworkingConfigValue_t opt, int g_nVirtualPortLocal){
	//SteamNetworkingConfigValue_t opt;
	//int g_nVirtualPortLocal = 0;
	//opt.SetInt32( k_ESteamNetworkingConfig_SymmetricConnect, 1 ); // << Note we set symmetric mode on the listen socket
	
	//target lines
	HSteamListenSocket g_hListenSock;
	g_hListenSock = SteamNetworkingSockets()->CreateListenSocketP2P( g_nVirtualPortLocal, 1, &opt );
}

static void FuzzTypeSteamNetworkingIPAddr(){
	//parsestring
	//SetIpv6
	//setipv4
	//tostring
}
//todo 
// ConnectP2P
// AcceptConnection?
// CloseConnection
// CloseListenSocket
// SetConnectionUserData
// GetConnectionUserData
// GetConnectionName?
// SendMessageToConnection
// SendMessages
// FlushMessagesOnConnection
// ReceiveMessagesOnConnection
// GetConnectionInfo
// GetConnectionRealTimeStatus
// GetDetailedConnectionStatus
// GetListenSocketAddress
// CreateSocketPair
// ConfigureConnectionLanes
// GetIdentity
// InitAuthentication
// GetAuthenticationStatus
// CreatePollGroup
// DestroyPollGroup
// SetConnectionPollGroup
// ReceiveMessagesOnPollGroup
// ReceivedRelayAuthTicket
// FindRelayAuthTicketForServer
// ConnectToHostedDedicatedServer
// GetHostedDedicatedServerPort
// GetHostedDedicatedServerPOPID
// GetHostedDedicatedServerAddress
// CreateHostedDedicatedServerListenSocket
// GetGameCoordinatorServerLogin
// ConnectP2PCustomSignaling
// ReceivedP2PCustomSignal
// GetCertificateRequest
// SetCertificate
// ResetIdentity
// RunCallbacks
// BeginAsyncRequestFakeIP
// GetFakeIP
// CreateListenSocketP2PFakeIP
// GetRemoteFakeIPForConnection
// CreateFakeUDPPort

///generators
SteamNetworkingIPAddr* steamnetworkingipaddr(const char * ip){
	static SteamNetworkingIPAddr snia;
	snia.Clear();
	//every single value needs to be parameter tuned
	//ipv4 or ipv6
	//probably use parsestring
	//printf("Address: %s\n", ip);
	snia.ParseString(ip);
	//std::cout << std::hex << static_cast<unsigned int>(static_cast<unsigned char>((*snia.m_ipv6)));
	//printf("%d", snia.m_ipv4.m_ip);
	//char* buf;
	//buf = (char*)malloc(snia.k_cchMaxString*sizeof(char));
	//(snia).ToString(buf, snia.k_cchMaxString, true);
	//printf("Address: %s\n", buf);
	return &snia;
}

int main( int argc, const char *argv[] )
{
	//printf("main");
	//const char * test = argv[1];
  	int testenum;
	testenum = atoi(argv[1]);
	//printf("%s", std::to_string(testenum).c_str());
	switch(testenum){
		case Forks::CreateConnectName: // ./command testenum SteamNetworkingIPAddr SteamNetworkingIPAddr string
			ForkCreateConnectName(steamnetworkingipaddr(argv[2]), steamnetworkingipaddr(argv[3]), argv[4]);
			break;
		default:
			break;
	}
}