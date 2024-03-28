#include <iostream>
#include <fstream>
#include <string>

#include "logger.h"

bool is_logging;
FILE *log;

void logger::init(std::string file) {
	is_logging = true;
	log = std::freopen(file.c_str(), "w", stderr);
}
void logger::init() {
	is_logging = false;
}

void logger::log_input(std::string text) {
	if (is_logging)
		std::cerr << time(0) << " I: " << text << "\n";
}

void logger::log_output(std::string text) {
	std::cout << text << "\n";
	if (is_logging)
		std::cerr << time(0) << " O: " << text << "\n";
}

void logger::log_output_silent(std::string text) {
	if (is_logging)
		std::cerr << time(0) << " S: " << text << "\n";
}

void logger::close() {
	if (is_logging)
		std::fclose(log);
}