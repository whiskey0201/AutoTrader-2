#include "stdafx.h"
#include "socket_server_impl.h"
#include "socket_server.h"

namespace Transmission{

	socket_server::socket_server(short port)
		:server_impl_(std::make_shared<Transmission::Detail::server>(port))
	{
	}

	void socket_server::run(){
		server_impl_->run();
	}

	void socket_server::stop(){
		server_impl_->stop();
	}
}