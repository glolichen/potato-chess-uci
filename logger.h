#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace logger {
	void init(std::string file);
	void init();

	void log_input(std::string text);
	void log_output(std::string text);
	void log_output_silent(std::string text);

	void close();
}

#endif