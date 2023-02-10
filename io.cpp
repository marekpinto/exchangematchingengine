// This file contains I/O functions.
// There should be no need to modify this file.

#include <fcntl.h>
#include <unistd.h>

#include "io.hpp"
#include "engine.hpp"

// out of line definitions for the mutexes in SyncCerr/SyncCout
std::mutex SyncCerr::mut;
std::mutex SyncCout::mut;

void ClientConnection::freeHandle()
{
	if(m_handle != -1)
	{
		close(m_handle);
		m_handle = -1;
	}
}

ReadResult ClientConnection::readInput(ClientCommand& read_into)
{
	switch(read(m_handle, &read_into, sizeof(ClientCommand)))
	{
		case 0: //
			return ReadResult::EndOfFile;

		case sizeof(ClientCommand): //
			return ReadResult::Success;

		default: //
			return ReadResult::Error;
	}
}
