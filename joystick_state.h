#ifndef JOYSTICK_STATE_H
#define JOYSTICK_STATE_H

#include <sys/types.h>
#include <map>
#include <string>

class joystick_state
{
public:
	void put_axis(unsigned char number, float value);
	void put_button(unsigned char number, bool value);

	float get_axis(unsigned char number);
	bool get_button(unsigned char number);
	
	std::string toString() const;

private:
	virtual void onAxisUpdated(unsigned char number);
	virtual void onButtonUpdated(unsigned char number);

private:
	std::map<unsigned char, float> m_mapAxes;
	std::map<unsigned char, bool> m_mapButtons;
};

#endif

