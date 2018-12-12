// asio_echo_client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#include "pch.h"
#include "asio_echo_client.h"

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
	application_t &app)
{
	INFO("log");
	if (ec)
	{
		INFO("log", ec.message());
		exit(1);
	}
	else
	{
		auto &read_offset = session->read_offset;
		read_offset += size;
		auto &buffer = session->buffer;
		auto &socket = session->socket;
		session->is_read = false;

		INFO("log", "session->size:{0} g_one_size:{1} size:{2}", session->read_offset, session_t::read_size, size);

		if (buffer[read_offset - 1] == '\0')
		{
			INFO("log");
			vector<char> buff;
			copy_n(buffer.begin(), buffer.size(), back_inserter(buff));
			buff.push_back('\0');
			INFO("log", "async_read_some:{0}", buff.data());
			session->clear();

			getline(session, app);
		}
		else
		{
			INFO("log");
			auto r = async_read(session,
				[session, &app](const error_code_t &ec, std::size_t size)
			{
				read_handler(ec, size, session, app);
			});
			if (!r)
			{
				app.io_context.run();
				return;
			}
		}
	}
}

void write_handler(const error_code_t &ec,
	std::size_t size,
	shared_ptr<session_t> session,
	application_t &app)
{
	INFO("log");
	if (ec)
	{
		INFO("log", ec.message());
		exit(1);
	}
	else
	{
		auto &write_offset = session->write_offset;
		auto &socket = session->socket;
		auto &write_queue = session->write_queue;
		auto &front = write_queue.front();
		auto &buffer = session->buffer;
		session->is_write = false;

		INFO("log", "write size:{0}", size);

		write_offset += size;

		if (write_offset == front.size())
		{
			write_offset = 0;
			write_queue.pop();
			if (write_queue.empty())
			{
				session->clear();
				auto r = async_read(session,
					[session, &app](const error_code_t &ec, std::size_t size)
				{
					read_handler(ec, size, session, app);
				});
				if (!r)
				{
					app.io_context.run();
					return;
				}
				return;
			}
		}
		auto w = async_write(session,
			[session, &app](const error_code_t &ec, std::size_t size)
		{
			write_handler(ec, size, session, app);
		});
		if (!w)
		{
			app.io_context.run();
			return;
		}
	}
}

void getline(shared_ptr<session_t> session, application_t &app)
{
	string message;
	if (std::getline(cin, message))
	{
		INFO("log", "message:{0}", message);

		vector<char> vec;
		copy_n(message.begin(), message.size(), back_inserter(vec));
		vec.push_back('\0');
		session->write_queue.push(std::move(vec));

		auto w = async_write(session,
			[session, &app](const error_code_t &ec, std::size_t size)
		{
			write_handler(ec, size, session, app);
		});
		if (!w)
		{
			app.io_context.run();
			return;
		}
	}
}

int main()
{
	auto logger = spdlog::stdout_color_mt("log");
	application_t app;
	auto &io_context = app.io_context;

	auto session = make_shared<session_t>(app.io_context);
	auto &socket = session->socket;

	socket.async_connect(endpoint_t(address_t::from_string("127.0.0.1"), 12500), 
		[session, &app](const error_code_t &ec)
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

			getline(session, app);
		}
	});

	io_context.run();

	return 0;
}
