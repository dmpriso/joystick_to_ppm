#ifndef PPM_STREAM_H
#define PPM_STREAM_H

#include <ostream>

class ppm_target
{
public:
	virtual void setChannelValue(unsigned char channel, float value) = 0;
};

class ppm_stream : public ppm_target
{
public:
	ppm_stream(std::ostream& target);
	
public:
	void setChannelValue(unsigned char channel, float value) override;
	
private:
	std::ostream& m_target;
	
};

class ppm_file : public ppm_target
{
public:
	ppm_file(int fd);
	
public:
	void setChannelValue(unsigned char channel, float value) override;
	
private:
	int m_fd;
	
};

#endif

