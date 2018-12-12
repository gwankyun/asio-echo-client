#pragma once
#include "base.h"

void write_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	shared_ptr<application_t> app);

void read_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	shared_ptr<application_t> app);

void getline(shared_ptr<session_t> session, shared_ptr<application_t> app);
