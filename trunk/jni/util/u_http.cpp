// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2009-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "u_http.h"
#include "u_base.h"

#include "m_assert.h"
#include "m_sstream.h"
#include "m_utility.h"
#include "m_stdio.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

using namespace util;


static unsigned const DefaultReadTimout	= 10000;
static unsigned const DefaultRedirects		= 3;

static int const PortNumber = 80;


#ifdef __WIN32__

# include "winsock2.h"

# define HOST_NOT_FOUND		WSAHOST_NOT_FOUND
# define NO_DATA				WSANO_DATA
# define NO_RECOVERY			WSANO_RECOVERY
# define TRY_AGAIN			WSATRY_AGAIN

# define ECONNABORTED		WSAECONNABORTED
# define ECONNREFUSED		WSAECONNREFUSED
# define ECONNRESET			WSAECONNRESET
# define EIO					WSAEMSGSIZE
# define EMFILE				WSAEMFILE
# define ENETDOWN				WSAENETDOWN
# define ENETRESET			WSAENETRESET
# define ENETUNREACH			WSAENETUNREACH
# define ENOBUFS				WSAENOBUFS
# define EFAULT				WSAEFAULT
# define EPROTONOSUPPORT	WSAEPROTONOSUPPORT
# define ETIMEDOUT			WSAETIMEDOUT

inline static int
get_errno()
{
	int rc = WSAGetLastError();
	WSACleanup();
	return rc;
}

# ifdef h_errno
#  undef errno
#  undef h_errno
# endif

# define errno		get_errno()
# define h_errno	get_errno()

WSADATA f_wsaData;
static bool wsaDataInizialized = false;

#else

# include <errno.h>
# include <unistd.h>
# include <netdb.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>

# ifndef NO_DATA
#  define NO_DATA NO_ADDRESS
# endif

enum { SOCKET_ERROR = -1 };

typedef int SOCKET;

static SOCKET const INVALID_SOCKET = -1;

inline static int closesocket(SOCKET sock) { return close(sock); }

#endif


struct Http::Socket
{
	Socket();
	~Socket();

	int connect(mstl::string host);
	int waitForInput(unsigned timeout);
	int recv(unsigned nbytes, char* buf);
	int send(char const* buf, unsigned size);

	SOCKET m_fd;
};


Http::Socket::Socket()
	:m_fd(INVALID_SOCKET)
{
#ifdef __WIN32__
	if (!::wsaDataInizialized)
	{
		if (::WSAStartup(MAKEWORD(2,2), &::f_wsaData) != 0)
		{
			::WSACleanup();
			fprintf(stderr, "WSAStartup() failed");
			abort();
		}
		::wsaDataInizialized = true;
	}
#endif
}


Http::Socket::~Socket()
{
	if (m_fd != INVALID_SOCKET)
		::closesocket(m_fd);

#ifdef __WIN32__
	::WSACleanup();
#endif
}


int
Http::Socket::connect(mstl::string host)
{
	//M_ASSERT(m_fd == INVALID_SOCKET);

	int port;

	// Check for port number specified in URL
	char const* p = ::strchr(host, ':');

	if (p)
	{
		port = ::atoi(p + 1);
		host.erase(host.data(), p);
	}
	else
	{
		port = ::PortNumber;
	}

	struct ::hostent* ent = ::gethostbyname(host);

	if (!ent)
	{
		switch (h_errno)
		{
			case HOST_NOT_FOUND:	return Http::Host_Not_Found;
			case NO_DATA:			return Http::No_Address;
			case NO_RECOVERY:		return Http::Name_Server_Error;
			case TRY_AGAIN:		return Http::Try_Again;
		}

		return Http::Resolve_Failed;
	}

	struct sockaddr_in server;

	server.sin_family = ent->h_addrtype;	// Set service sin_family to PF_INET.
	server.sin_port = htons(port);			// Put port number into sockaddr.

	// Copy host address from hostent to (server) socket address,
	::memcpy(&server.sin_addr, ent->h_addr, ent->h_length);

	m_fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (m_fd == INVALID_SOCKET)
	{
		switch (errno)
		{
			case ENETDOWN:				return Http::Network_Down;				// win32 specific
			case EMFILE:				return Http::Too_Many_Open_Files;
			case ENOBUFS:				return Http::Out_Of_Memory;
			case EPROTONOSUPPORT:	return Http::TCP_Not_Supported;
#ifndef __WIN32__
			case EACCES:				return Http::No_Permission;
			case ENFILE:				return Http::Too_Many_Open_Files;
			case ENOMEM:				return Http::Out_Of_Memory;
#endif
		}

		return Socket_Failed;
	}

	::setsockopt(m_fd, SOL_SOCKET, SO_KEEPALIVE, 0, 0);

#ifdef __WIN32__
	WSACleanup();
#endif

	if (::connect(m_fd, reinterpret_cast<struct sockaddr const*>(&server), sizeof(server))
			== SOCKET_ERROR)
	{
		switch (errno)
		{
			case ENETDOWN:			return Http::Network_Down;		// win32 specific
			case ENOBUFS:			return Http::Out_Of_Memory;	// win32 specific
			case ECONNREFUSED:	return Http::Connection_Refused;
			case ENETUNREACH:		return Http::Network_Unreachable;
			case ETIMEDOUT:		return Http::Timeout;
#ifndef __WIN32__
			case EACCES:			// fallthru
			case EPERM:				return Http::Firewall_Denied;
			case EAGAIN:			return Http::No_Free_Local_Port;
			case EINPROGRESS:		return Http::Connection_Interrupt;
#endif
		}

		return Connect_Failed;
	}

#ifdef __WIN32__
	WSACleanup();
#endif

	return 0;
}


int
Http::Socket::waitForInput(unsigned timeout)
{
	::fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(m_fd, &rfds);

	int rc;

	if (timeout)
	{

		struct timeval tv;
		tv.tv_sec = timeout/1000;
		tv.tv_usec = timeout % 1000;

		rc = ::select(m_fd + 1, &rfds, 0, 0, &tv);
	}
	else // No timeout, can block indefinitely
	{
		rc = ::select(m_fd + 1, &rfds, 0, 0, 0);
	}

	switch (rc)
	{
		case 0:
			return Http::Timeout;

		case SOCKET_ERROR:
			switch (errno)
			{
				case ENETDOWN:	return Http::Network_Down;		// win32 specific
				case EFAULT:	return Http::Out_Of_Memory;	// win32 specific
#ifndef __WIN32__
				case EINTR:		return Http::Interrupted;
#endif
			}
			return Http::Select_Failed;
	}

#ifdef __WIN32__
	WSACleanup();
#endif

	return 0;
}


int
Http::Socket::recv(unsigned nbytes, char* buf)
{
	int rc = ::recv(m_fd, buf, nbytes, 0);

	if (rc == SOCKET_ERROR)
	{
		switch (errno)
		{
			case ENETDOWN:		return Http::Network_Down;			// win32 specific
			case ETIMEDOUT:	return Http::Connection_Closed;	// win32 specific
			case ENOBUFS:		return Http::Out_Of_Memory;		// unix specific
#ifdef __WIN32__
			case ECONNRESET:
			case ECONNABORTED:
			case ENETRESET:	return Http::Connection_Closed;
#else
			case EINTR:			return Http::Interrupted;
#endif
		}

		return Read_Failed;
	}

#ifdef __WIN32__
	WSACleanup();
#endif

	return rc;
}


int
Http::Socket::send(char const* buf, unsigned size)
{
	int rc = ::send(m_fd, buf, size, 0);

	if (rc == SOCKET_ERROR)
	{
		switch (errno)
		{
			case ENETDOWN:			return Http::Network_Down;			// win32 specific
			case ECONNRESET:		return Http::Connection_Closed;
			case ENOBUFS:			return Http::Output_Queue_Full;
#ifdef __WIN32__
			case ENETRESET:		return Http::Connection_Closed;
			case ETIMEDOUT:		// fallthru
			case ECONNABORTED:	return Http::Connection_Closed;
#else
			case EHOSTUNREACH:	return Http::Host_Unreachable;
			case EMSGSIZE:			return Http::Low_Level_IO_Error;
			case ENOMEM:			return Http::Out_Of_Memory;
			case EPIPE:				return Http::Connection_Closed;
			case EINTR:				return Http::Interrupted;
#endif
		}

		return Http::Write_Failed;
	}

#ifdef __WIN32__
	WSACleanup();
#endif

	return rc;
}


static void
copyURL(mstl::string const& src, mstl::string& dst)
{
	for (mstl::string::const_iterator i = src.begin(); i != src.end(); ++i)
	{
		if (*i == ' ')
			dst += "%20";
		else
			dst += *i;
	}
}


Http::Http()
	:m_hideUserAgent(false)
	,m_timeout(DefaultReadTimout)
{
	setup();
}


Http::Http(char const* host)
	:m_hideUserAgent(false)
	,m_timeout(DefaultReadTimout)
{
	setup();
	setHost(host);
}


void
Http::setHost(char const* host)
{
	//M_REQUIRE(host);
	m_host = host;
}


void
Http::setReferer(char const* referer)
{
	//M_REQUIRE(referer);
	m_referer = referer;
}


void
Http::hideUserAgent(bool flag)
{
	m_hideUserAgent = flag;
}


int
Http::get(char const* url, mstl::string& result)
{
	mstl::ostringstream stream;
	int rc = get(url, stream);
	result = stream.str();
	return rc;
}


int
Http::get(char const* url, mstl::ostream& stream)
{
	//M_REQUIRE(url);
	//M_REQUIRE(!*url);
	//M_REQUIRE(!host().empty());

	mstl::string	myurl;
	mstl::string	header;
	Socket			sock;
	unsigned			redirectsFollowed = 0;

	::copyURL(url, myurl);

	// This loop allows us to follow redirects if need be.
	do
	{
		// Compose a request string.
		mstl::string request;
		makeRequest("GET", myurl, request);
		request += "Connection: Close\r\n\r\n";

		myurl.clear();

		int rc = sock.connect(m_proxyServer.empty() ? m_host : m_proxyServer);

		if (rc < 0)
			return rc;

		rc = sock.send(request, request.size());

		if (rc < int(request.size()))
			return Write_Failed;

		// Grab enough of the response to get the metadata.
		rc = readLine(sock, header);

		if (rc < 0)
			return rc;

		if (::sscanf(header, "HTTP/1.%*d %03d", &rc) != 1)
			return Invalid_Response;

		char const* p = header.c_str();

		while (*p != ' ')
			++p;

		int code;

		if (::sscanf(p + 1, "%d", &code) != 1)
			return Invalid_Response;

		if (200 > code || code > 307)
			return Unexpected_Status_Code;

		// If a redirect, repeat operation until final URL is found or we
		// redirect DefaultRedirects times.
		if (code >= 300)
		{
			if (++redirectsFollowed == DefaultRedirects)
				return Redirects_Exceeded;

			// Pick up redirect URL and repeat process.
			p = ::strstr(header.c_str(), "Location:");

			if (!p && !(p = ::strstr(header.c_str(), "location:")))
				return Cannot_Redirect;

			p += 9;	// skip 'Location:'

			// Skip any whitespace...
			while (*p && ::isspace(*p))
				++p;

			if (!p)
				return Cannot_Redirect;

			char const* e = p;

			// Find end of URL.
			while (*e && !::isspace(*e))
				++e;

			if (p < e)
			{
				myurl.assign(p, e);
			}
			else
			{
				// Found 'Location:' but contains no URL! We'll handle it as
				// 'found', hopefully the resulting document will give the user
				// a hint as to what happened.
				redirectsFollowed = DefaultRedirects;	// terminate redirection
			}
		}
		else
		{
			redirectsFollowed = DefaultRedirects;	// terminate redirection
		}
	}
	while (redirectsFollowed < DefaultRedirects);

	int contentSize = -1;

	// Parse out about how big the data segment is.
	//	Note that under current HTTP standards (1.1 and prior), the
	//	Content-Length field is not guaranteed to be accurate or even present.
	// Note that some servers use different capitalization.
	do
	{
		int rc = readLine(sock, header);

		if (rc < 0)
			return rc;

		for (char* p = header.data(); *p && *p != ':'; ++p)
			*p = ::tolower(*p);

		::sscanf(header, "content-length: %d", &contentSize);
	}
	while (!header.empty());

	if (contentSize < 0)
		return Invalid_Response;

	int bytesRead = 0;

	while (contentSize > 0)
	{
		int rc = sock.waitForInput(m_timeout);

		if (rc < 0)
			return rc;

		char buffer[500];

		rc = sock.recv(mstl::min(sizeof(buffer), size_t(contentSize)), buffer);

		if (rc <= 0)
			return rc == 0 ? bytesRead : rc;

		stream.write(buffer, rc);
		contentSize -= rc;
		bytesRead += rc;
	}

	return bytesRead;
}


// Reads the metadata of an HTTP response.
int
Http::readLine(Socket& sock, mstl::string& result)
{
	result.clear();

	while (result.size() < 1024)
	{
		int	rc = sock.waitForInput(m_timeout);
		char	c;

		if (rc < 0)
			return rc;

		rc = sock.recv(1, &c);

		if (rc < 0)
			return rc;

		if (rc == 0)
			return Read_Failed;

		switch (c)
		{
			case '\r':	// ignore CR
				break;

			case '\n':	// LF is a separator
				return result.size();

			default:
				result += c;
				break;
		}
	}

	return result.size();
}


void
Http::makeRequest(char const* command, mstl::string const& url, mstl::string& result)
{
	static mstl::string const HttpVersion("HTTP/1.0");
	static mstl::string const DefaultUserAgent("HTTP/Scidb ($Date$)");

	result += command;
	result += " ";

	if (!m_proxyServer.empty())
	{
		result += "http://";
		result += m_host;
	}

	result += "/";
	result += url;

	result += HttpVersion;
	result += "\r\n";

	// Use Host: even though 1.0 doesn't specify it. Some servers
	// won't play nice if we don't send Host.
	result += "Host: ";
	result += m_host;
	result += "\r\n";

	if (!m_referer.empty())
	{
		result += "Referer: ";
		result += m_referer;
		result += "\r\n";
	}

	if (!m_hideUserAgent)
	{
		result += "User-Agent: ";
		result += m_userAgent.empty() ? DefaultUserAgent : m_userAgent;
		result += "\r\n";
	}
}


void
Http::setup()
{
	char const* proxy = ::getenv("http_proxy");

	if (proxy)
		m_proxyServer = proxy;
}

// vi:set ts=3 sw=3:
