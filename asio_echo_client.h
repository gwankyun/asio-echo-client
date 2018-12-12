#pragma once
#include "base.h"

class application_t
{
public:
	application_t();
	~application_t();

	io_context_t io_context;

private:

};

application_t::application_t()
{
}

application_t::~application_t()
{
}

void write_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	application_t &app);

void read_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	application_t &app);

void getline(shared_ptr<session_t> session, application_t &app);
