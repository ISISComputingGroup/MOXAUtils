#include <iostream>
#include <string>
#include <stdexcept>

#include "windows.h"

#include <IPSerial.h>

#include <CLI/CLI.hpp>

struct MoxaError
{
	int code;
	const char* message;
};

static MoxaError errorLookup[] = { 
	{ 0, "NSIO_OK - OK" },
	{ -1, "NSIO_BADPORT -No such port or port not opened. " },
	{ -2, "NSIO_BADPARM - Bad parameter. " },
	{ -3, "NSIO_THREAD_ERR - Create background thread fail. " },
	{ -4, "NSIO_MEMALLOCERR - memory allocate error. " },
	{ -100, "NSIO_INVALID_PASSWORD - Invalid NPort console password. "  },
	{ -101, "NSIO_RESET_TIMEOUT - Reset NPort timeout. Maybe the NPort is power down or network cable is disconnected. " },
	{ -102, "NSIO_NOT_ALIVE - NPort is not alive. Maybe the NPort is power down or network cable is disconnected. "},
	{ -200, "NSIO_CONNECT_FAIL - Connect to NPort fail. Maybe the NPort is power down or network cable is disconnected. "},
	{ -201, "NSIO_SOCK_INIT_FAIL - Socket initialize error. "},
	{ -202, "NSIO_SOCK_ERR - Communication with NPort error. "},
	{ -203, "NSIO_TIMEOUT - Communication with NPort timeout. "},
	{ -204, "NSIO_EXCEED - Connection exceed. "},
	{ -301, "NSIO_THREAD_TIMEOUT - Thread wait for resources timeout. " }
};

class MoxaException : public std::runtime_error
{
	public:
	explicit MoxaException(int code, const char* func) : std::runtime_error(translateCode(code, func)) { }
	static std::string translateCode(int code, const char* func)
	{
		for(int i=0; i<sizeof(errorLookup) / sizeof(MoxaError); ++i)
		{
			if (errorLookup[i].code == code)
			{
				return std::string(func) + ": " + errorLookup[i].message;
			}
		}
		return std::string(func) + ": unknown error";
	};
};

#define ERROR_CHECK1(__arg) \
    if ( (__arg) < 0 ) \
	{ \
	    throw MoxaException(__arg, #__arg); \
	}
	
#define ERROR_CHECK2(__code,__func) \
    if ( (__code) < 0 ) \
	{ \
	    throw MoxaException(__code,__func); \
	}

int main(int argc, char* argv[])
{
    CLI::App app;

    std::string moxa_ip_s, moxa_password_s;
	bool alive = false, reset_server = false, reset_port = false, status = false;
	int ret, port = -1, port_id = -1;
	double timeout = 1.0;

    Sleep(10000);
    app.add_option("--ip,ip", moxa_ip_s, "MOXA IP Address")->mandatory();
    CLI::Option* pass_opt = app.add_option("--password", moxa_password_s, "MOXA Password");
    app.add_flag("--alive", alive, "Check moxa is alive");
    app.add_flag("--resetserver", reset_server, "Reset server")->needs(pass_opt);
    app.add_flag("--timeout", timeout, "Timeout (s)");
    CLI::Option* port_opt = app.add_option("--port", port, "moxa port to use");
    app.add_flag("--status", status, "Status")->needs(port_opt);
    app.add_flag("--resetport", reset_port, "Reset specific moxa port")->needs(port_opt)->needs(pass_opt);

    CLI11_PARSE(app, argc, argv);
	
	int timeout_ms = static_cast<int>(timeout * 1000.0 + .5);
	char* moxa_ip = const_cast<char*>(moxa_ip_s.c_str());
	char* moxa_password = const_cast<char*>(moxa_password_s.c_str());

	try
	{
	    ERROR_CHECK1(nsio_init());
		if (alive)
		{
			ERROR_CHECK1(nsio_checkalive(moxa_ip, timeout_ms));
			std::cerr << "MOXA " << moxa_ip << " is alive" << std::endl;
		}
		else if (reset_server)
		{
			ERROR_CHECK1(nsio_resetserver(moxa_ip, moxa_password));
		}
		else if (port != -1)
		{
			int port_id = nsio_open(moxa_ip, port, timeout_ms);
			ERROR_CHECK2(port_id,"nsio_open");
			if (reset_port)
			{
			    ERROR_CHECK1(nsio_resetport(port_id, moxa_password));
			}
			else if (status)
			{
				ret = nsio_iqueue(port_id);
				ERROR_CHECK2(ret,"nsio_iqueue");
				std::cerr << "Input queue size: " << ret << std::endl;				
				ret = nsio_oqueue(port_id);
				ERROR_CHECK2(ret,"nsio_oqueue");
				std::cerr << "Output queue size: " << ret << std::endl;				
				ret = nsio_data_status(port_id);
				ERROR_CHECK2(ret,"nsio_data_status");
				std::cerr << "Data status: " << (ret > 0 ? "error" : "ok") << std::endl;				
				ret = nsio_breakcount(port_id);
				ERROR_CHECK2(ret,"nsio_breakcount");
				std::cerr << "Break count: " << ret << std::endl;				
			}
		}
	    ERROR_CHECK1(nsio_end());
	}
	catch(const MoxaException& mex)
	{
		std::cerr << "Moxa error: " << mex.what() << std::endl;
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Other error: " << ex.what() << std::endl;
	}
	if (port_id >= 0)
	{
		nsio_close(port_id);
	}
	nsio_end();
	return 0;
}
