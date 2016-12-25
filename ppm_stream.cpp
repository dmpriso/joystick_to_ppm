#include "ppm_stream.h"

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
	this->m_target << "ppm" << (int)channel << " " << (int)value << "\n" << std::flush;
}
