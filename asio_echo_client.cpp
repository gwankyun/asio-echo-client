// asio_echo_client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#include "pch.h"
#include "asiio_echo_client.h"

std::size_t session_t::read_size = 2;
std::size_t session_t::write_size = 2;

session_t::session_t(io_context_t &io_context)
	: socket(io_context)
{
}

session_t::~session_t()
{
}

void session_t::clear()
{
	INFO("log");
	buffer.clear();
	read_offset = 0;
}

void read_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	io_context_t &io_context)
{
	INFO("log");
	if (ec)
	{
		INFO("log", ec.message());
	}
	else
	{
		session->read_offset += size;
		auto &buffer = session->buffer;
		auto &socket = session->socket;

		INFO("log", "session->size:{0} g_one_size:{1} size:{2}", session->read_offset, session_t::read_size, size);

		if (size < session_t::read_size)
		{
			INFO("log");
			vector<char> buff;
			copy_n(buffer.begin(), buffer.size(), back_inserter(buff));
			buff.push_back('\0');
			INFO("log", "async_read_some:{0}", buff.data());
			session->clear();

			string message;
			if (getline(cin, message))
			{
				INFO("log", "message:{0}", message);

				vector<char> vec;
				copy_n(message.begin(), message.size(), back_inserter(vec));
				session->write_queue.push(std::move(vec));

				async_write(session, io_context);
			}
		}
		else if (size == session_t::read_size)
		{
			INFO("log");
			async_read(session, io_context);
		}
	}
}

void write_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	io_context_t &io_context)
{
	INFO("log");
	if (ec)
	{
		INFO("log", ec.message());
	}
	else
	{
		auto &write_offset = session->write_offset;
		auto &socket = session->socket;
		auto &write_queue = session->write_queue;
		auto &front = write_queue.front();
		auto &buffer = session->buffer;

		INFO("log", "write size:{0}", size);

		write_offset += size;

		if (write_offset == front.size())
		{
			write_offset = 0;
			write_queue.pop();
			if (write_queue.empty())
			{
				session->clear();
				async_read(session, io_context);
			}
			else
			{
				async_write(session, io_context);
			}
		}
		else
		{
			async_write(session, io_context);
		}
	}
}

bool async_write(shared_ptr<session_t> session, io_context_t &io_context)
{
	INFO("log");
	auto &socket = session->socket;
	auto &write_queue = session->write_queue;
	if (!write_queue.empty())
	{
		auto &front = write_queue.front();
		auto size = std::min(session_t::write_size, front.size() - session->write_offset);
		socket.async_write_some(
			asio::buffer(front.data() + session->write_offset, size), 
			[session, &io_context](const error_code_t &ec, std::size_t size)
		{
			write_handler(ec, size, session, io_context);
		});
		return true;
	}
	else
	{
		return false;
	}
}

void async_read(shared_ptr<session_t> session, io_context_t &io_context)
{
	INFO("log");
	auto &buffer = session->buffer;
	auto &socket = session->socket;
	buffer.resize(session->read_offset + session_t::read_size);
	socket.async_read_some(
		asio::buffer(buffer.data() + session->read_offset, session_t::read_size),
		[session, &io_context](const error_code_t &ec, std::size_t size)
	{
		read_handler(ec, size, session, io_context);
	});
}

int main()
{
	auto logger = spdlog::stdout_color_mt("log");
	io_context_t io_context;

	auto session = make_shared<session_t>(io_context);
	auto &socket = session->socket;

	socket.async_connect(endpoint_t(address_t::from_string("127.0.0.1"), 12500), 
		[session, &io_context](const error_code_t &ec)
	{
		if (ec)
		{
			INFO("log", ec.message());
		}
		else
		{
			auto &socket = session->socket;
			auto re = socket.remote_endpoint();
			auto address = re.address().to_string();
			auto port = re.port();
			cout << address << endl;
			cout << port << endl;
			INFO("log", "address:{0} port:{1}", address, port);

			string message;
			if (getline(cin, message))
			{
				INFO("log", "message:{0}", message);

				vector<char> vec;
				copy_n(message.begin(), message.size(), back_inserter(vec));
				session->write_queue.push(std::move(vec));

				async_write(session, io_context);
			}
		}
	});

	io_context.run();

	return 0;
}
