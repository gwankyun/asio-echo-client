#pragma once
#include "base.h"

void write_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	io_context_t &io_context);

void read_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	io_context_t &io_context);

void getline(shared_ptr<session_t> session, io_context_t &io_context);
