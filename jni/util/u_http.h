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

#ifndef _util_http_included
#define _util_http_included

#include "m_string.h"
#include "m_utility.h"

namespace mstl { class ostream; }

namespace util {

class Http  : public mstl::noncopyable
{
public:

	enum
	{
		// The following should never happen:
		Write_Failed				= -1,	// Unexpected write error.
		Read_Failed					= -2,	// Unexpected read error.
		Connect_Failed				= -3,	// Unexpected error returned from connect().
		Select_Failed				= -4,	// Unexpected error returned from select().
		Resolve_Failed				= -5,	// Unexpected error returned from gethostbyname().
		Socket_Failed				= -6,	// Unexpected error returned from socket().

		// The following can happen:
		Host_Not_Found				= -6,	// The specified host is unknown.
		No_Address					= -7,	// The requested name is valid but does not have an IP address.
		Name_Server_Error			= -8,	// A non-recoverable name server error occurred.
		Try_Again					= -9,	// A temporary error occurred on an authoritative name server.
												// Try again later.
		Interrupted					= -10,// The call was interrupted by a signal.
		Low_Level_IO_Error		= -11,// A low-level I/O error occurred.
		Connection_Closed			= -12,// Reading end is closed.
		Timeout						= -13,// Timeout occured while waiting for response.
		Out_Of_Memory				= -14,// Insufficient memory is available.
		No_Permission				= -15,// Permission to create a socket for HTTP is denied.
		Too_Many_Open_Files		= -16,// Process file table overflow OR the system limit  on  the
												// total number of open files has been reached.
		TCP_Not_Supported			= -17,// TCP protocol is not supported within this domain.
		Firewall_Denied			= -18,// The connection request failed because of a local firewall rule.
		No_Free_Local_Port		= -19,// No more free local ports or insufficient entries in the routing
												// cache.
		Connection_Refused		= -20,// No one listening on the remote address.
		Connection_Interrupt		= -21,// The connection cannot be completed immediately.
		Network_Unreachable		= -22,// Network is unreachable.
		Invalid_Response			= -23,// Unexpected HTTP response.
		Unexpected_Status_Code	= -24,// Request returned an unexpected status code.
		Cannot_Redirect			= -25,// Redirection failed.
		Redirects_Exceeded		= -26,// Followed the maximum number of redirects.
		Network_Down				= -27,// Network is down.
		Host_Unreachable			= -28,// Host is (currently) unreachable.
		Output_Queue_Full			= -29,// The output queue for a network interface was full.
	};

	Http();	// not yet usable
	explicit Http(char const* host);

	void setHost(char const* host);
	void setReferer(char const* host);
	void setTimeout(unsigned milliseconds);
	void hideUserAgent(bool flag = true);

	mstl::string const& host() const;

	int get(char const* url, mstl::string& result);
	int get(char const* url, mstl::ostream& stream);

private:

	class Socket;
	friend class Socket;

	// Opens a TCP socket and returns the descriptor.
	int makeSocket();

	int readLine(Socket& sock, mstl::string& result);
	void setup();

	void makeRequest(char const* command, mstl::string const& url, mstl::string& result);

	mstl::string	m_host;
	mstl::string	m_referer;
	mstl::string	m_userAgent;
	bool				m_hideUserAgent;
	unsigned			m_timeout;
	mstl::string	m_proxyServer;
};

} // namespace util

#include "u_http.ipp"

#endif // _util_http_included

// vi:set ts=3 sw=3:
