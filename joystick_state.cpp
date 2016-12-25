#include "joystick_state.h"
#include <sstream>

void joystick_state::put_axis(unsigned char number, float value)
{
	this->m_mapAxes[number] = value;
	this->onAxisUpdated(number);
}

void joystick_state::put_button(unsigned char number, bool value)
{
	this->m_mapButtons[number] = value;
	this->onButtonUpdated(number);
}

float joystick_state::get_axis(unsigned char number)
{
	return this->m_mapAxes[number];
}

bool joystick_state::get_button(unsigned char number)
{
	return this->m_mapButtons[number];
}

std::string joystick_state::toString() const
{
	std::stringstream ss;
	
	for (auto kvp : this->m_mapAxes)
	{
		ss << "ax" << (int)kvp.first << "=" << kvp.second << " ";
	}
	ss << std::endl;
	
	for (auto kvp : this->m_mapButtons)
	{
		ss << "bt" << (int)kvp.first << "=" << (kvp.second ? "ON" : "off") << " ";
	}
	ss << std::endl;
	
	return ss.str();
}

void joystick_state::onButtonUpdated(unsigned char number) {}
void joystick_state::onAxisUpdated(unsigned char number) {}