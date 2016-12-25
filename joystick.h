#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "joystick_ppm.h"
#include <string>

class joystick : public joystick_ppm
{
public:
	joystick(std::string strConfigPath, 
		std::string strTrimSavePath,
		ppm_target& target);
		
private:
	void saveTrimValue(unsigned char axis_number, float trim) override;
	void saveSubtrimValue(unsigned char axis_number, float trim) override;

private:	
	static configuration loadConfig(std::string strConfigPath);
	void loadTrims();
	void saveTrims();
		
private:
	std::string m_strTrimSavePath;
		
};

#endif

