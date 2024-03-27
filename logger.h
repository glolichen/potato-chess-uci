#ifndef LOGGER_H
#define LOGGER_H

#include <istream>
#include <iostream>
#include <string>

namespace logger {
	// https://stackoverflow.com/a/674120
	class Logger : public std::ostream {
	private:
		bool is_logging;
		Logger(std::string file) {
			is_logging = true;
			std::freopen(file.c_str(), "w", stderr);
		}
		Logger() {
			is_logging = false;
		}

	public:
		static Logger *get_instance(std::string file) {
			static Logger instance(file);
			return &instance;
		}
		static Logger *get_instance() {
			static Logger instance;
			return &instance;
		}

		// log output
		template <class T>
		Logger &operator<<(const T &v) {
			std::cout << v;
			if (is_logging)
				std::cerr << time(0) << " O: " << v;
			return *this;
		}

		// log input
		void log_input(const std::string text) {
			if (is_logging)
				std::cerr << time(0) << " I: " << text << "\n";
		}
	};
}

#endif