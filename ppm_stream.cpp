#include "ppm_stream.h"
#include <sstream>
#include <unistd.h>

ppm_stream::ppm_stream(std::ostream& target)
	: m_target(target)
{}

void ppm_stream::setChannelValue(unsigned char channel, float value)
{
	// convert value to PPM us
	// value is -1 ... 1 -> 1000 .. 2000
	value = value * 500.0f + 1500.0f;
	// define some upper and lower limit
	if (value > 2100.0f)
		value = 2100.0f;
	if (value < 900.0f)
		value = 900.0f;
	
	// output that value as int
	this->m_target << "ppm:" << (int)channel << "," << (int)value << "\n" << std::flush;
}

ppm_file::ppm_file(int fd)
	: m_fd(fd)
{}

void ppm_file::setChannelValue(unsigned char channel, float value)
{
	std::stringstream ss;

	// convert value to PPM us
	// value is -1 ... 1 -> 1000 .. 2000
	value = value * 500.0f + 1500.0f;
	// define some upper and lower limit
	if (value > 2100.0f)
		value = 2100.0f;
	if (value < 900.0f)
		value = 900.0f;
	
	// output that value as int
	ss << "ppm:" << (int)channel << "," << (int)value << "\n" << std::flush;
	
	// write to fd
	write(this->m_fd, ss.str().data(), ss.str().size());
}
