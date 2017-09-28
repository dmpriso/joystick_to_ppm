#include "joystick.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <algorithm>

#include "libini.h"


std::string to_lower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

bool is_prefix(std::string section_name, std::string prefix, int& value)
{
	section_name = to_lower(section_name);
	
	if (section_name.substr(0, prefix.size()) == prefix)
	{
		value = std::stoi(section_name.substr(prefix.size()));
		return true;
	}
	return false;
}

float getNum(std::string str)
{
	return ::atof(str.c_str());
}

float getPercentNumAsFactor(std::string str)
{
	// parse out % value, if present
	auto f = str.find("%");
	if (f != std::string::npos)
		str = str.substr(0, f - 1);
	
	return ::atof(str.c_str()) / 100.0f;
}



joystick::joystick(std::string strConfigPath, 
		std::string strTrimSavePath,
		ppm_target& target)
	: joystick_ppm(loadConfig(strConfigPath), target),
	m_strTrimSavePath(strTrimSavePath)
{
	this->loadTrims();

	// TEST
	//this->m_mapSubtrims[2] = 0.01f;
	//this->saveTrims();
}


joystick::configuration joystick::loadConfig(std::string strConfigPath)
{
	joystick::configuration conf;
	
	std::ifstream t(strConfigPath);
	std::string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	auto ini = libini::parse(str);
	
	std::cout << "Loading configuration ..." << std::endl;
	
	for(auto kvp: ini)
	{
		auto section_name = kvp.first;
		auto& section_values = kvp.second;
		int id;
		bool release = false;
		bool subtrim = false;
		
		if (is_prefix(section_name, "axis", id))
		{
			// an axis has the following settings
			// ppm_channel_id;
			// weight;
			// offset;
			// expo;
			// numeric values are in PERCENT!
			int ppm_channel_id = std::stoi(section_values["ppm_channel_id"]);
			float factor = getPercentNumAsFactor(section_values["weight"]);
			float offset = getPercentNumAsFactor(section_values["offset"]);
			float expo = 1.0f - getPercentNumAsFactor(section_values["expo"]);
			//int mixing_id = std::stoi(section_values["mixing_id"]);
			
			std::cout << "Adding mapping:" << std::endl
				<< "Axis ID: " << id << std::endl
				<< "PPM Channel ID: " << ppm_channel_id << std::endl
				<< "Factor: " << factor << std::endl
				<< "Offset: " << offset << std::endl
				<< "Expo (as factor): " << expo << std::endl
				<< std::endl;
			
			conf.AxisMappings.emplace(id, 
				axis_mapping_target(ppm_channel_id, factor, expo, offset));
		}
		else if (is_prefix(section_name, "action_button", id) ||
			(release = is_prefix(section_name, "action_releasebutton", id)))
		{
			// action button has the following values:
			// ppm_channel_id
			// value (percent)
			int ppm_channel_id = std::stoi(section_values["ppm_channel_id"]);
			float value = getPercentNumAsFactor(section_values["value"]);
			
			std::cout << "Adding button " << (release ? "RELEASE" : "press") << " action" << std::endl
				<< "Button ID: " << id << std::endl
				<< "PPM Channel ID: " << ppm_channel_id << std::endl
				<< "Value: " << value << std::endl
				<< std::endl;
			
			auto& map = (release ? conf.ButtonReleaseActions : conf.ButtonPressActions);
			map.emplace(id,
				button_action(ButtonActionType::SetValue, ppm_channel_id, value));
		}
		else if (is_prefix(section_name, "trim_button", id) ||
			(subtrim = is_prefix(section_name, "subtrim_button", id)))
		{
			// trim button has the following values:
			// axis_id
			// step (percent)
			
			// subtrim button has the following values: 
			// ppm_channel_id
			// step (percent)
			
			
			int axis_id = std::stoi(section_values[(subtrim ? "ppm_channel_id" : "axis_id")]);
			float value = getPercentNumAsFactor(section_values["step"]);

			std::cout << "Adding " << (subtrim ? "subtrim" : "trim") << " button" << std::endl
				<< "Button ID: " << id << std::endl
				<< (subtrim ? "PPM Channel" : "Axis") << " ID:" << axis_id << std::endl
				<< "Step: " << value << std::endl
				<< std::endl;
	
			conf.ButtonPressActions.emplace(id,
				button_action(subtrim ? ButtonActionType::AddSubTrim : ButtonActionType::AddTrim,
					axis_id, value));
		}
		else if (is_prefix(section_name, "buttonize_axis", id))
		{
			int low_id = std::stoi(section_values["low_button_id"]);
			int high_id = std::stoi(section_values["high_button_id"]);
			
			std::cout << "Adding axis buttonization" << std::endl
				<< "Axis ID: " << id << std::endl
				<< "Low value button ID: " << low_id << std::endl
				<< "High value button ID: " << high_id << std::endl
				<< std::endl;
				
			conf.Buttonizations.emplace(id,
				buttonize_axis(low_id, high_id));
		}
	}
	
	std::cout << "Loading configuration done." << std::endl;
	
	return conf;
}

void joystick::loadTrims()
{
	std::ifstream t(this->m_strTrimSavePath);
	std::string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
	auto ini = libini::parse(str);
	
	std::cout << "Loading user trims ..." << std::endl;
	
	for(auto kvp : ini["trims"])
	{
		auto name = kvp.first;
		int id;
		auto value = getNum(kvp.second);
		if (is_prefix(name, "trim_axis", id))
		{
			std::cout << "Loading axis trim for axis #" << id << ": " << value << std::endl;
			
			this->loadUserTrim(id, value);
		}
		else if (is_prefix(name, "subtrim_channel", id))
		{
			std::cout << "Loading channel subtrim for channel #" << id << ": " << value << std::endl;
			
			this->loadUserSubtrim(id, value);
		}
	}
	
	std::cout << "Loading user trims done." << std::endl;
	
	t.close();
}

void joystick::saveTrimValue(unsigned char axis_number, float trim)
{
	this->saveTrims();
}

void joystick::saveSubtrimValue(unsigned char axis_number, float trim)
{
	this->saveTrims();
}

void joystick::saveTrims()
{
	std::ofstream ss("user_trims.ini", std::ios::out);
	
	ss << "# IMPORTANT This file is autogenerated and will be overwritten on each trim change" << std::endl;
	ss << "[trims]" << std::endl;
	
	ss << "# Axis trims" << std::endl;
	
	for (auto kvp : this->m_config.AxisMappings)
	{
		ss << "trim_axis" << (int)kvp.first << "=" << kvp.second.user_trim << std::endl;
	}
	ss << std::endl << "# Channel subtrims" << std::endl;
	for (auto kvp : this->m_mapSubtrims)
	{
		ss << "subtrim_channel" << (int)kvp.first << "=" << kvp.second << std::endl;
	}
	
	ss.close();
}

