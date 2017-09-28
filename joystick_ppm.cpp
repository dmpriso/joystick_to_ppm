#include "joystick_ppm.h"
#include <cmath>
#include <iostream>

ppm_mixer::ppm_mixer(ppm_target& target)
	: m_target(target)
{}

void ppm_mixer::set_mixing_value(unsigned char ppm_channel_id,
		float value,
		int mixing_id,
		bool no_write)
{
	this->m_map[ppm_channel_id][mixing_id] = value;

	float fSum = 0.0f;
	for (auto v : this->m_map[ppm_channel_id])
	{
		fSum += v.second;
	}
	
	if (!no_write)
		this->m_target.setChannelValue(ppm_channel_id, 
			fSum);
}


int joystick_ppm::axis_mapping_target::s_mixingid_counter = -2;


joystick_ppm::joystick_ppm(const configuration& config, ppm_target& target)
	: m_config(config),
	m_mixer(target)
{
}

void joystick_ppm::onAxisUpdated(unsigned char number)
{
	// check if there is a mapping
	auto item = this->m_config.AxisMappings.find(number);
	auto value = this->get_axis(number);
	if (item != this->m_config.AxisMappings.end())
	{
		// calc mapping
		auto& mapping = item->second;
		
		// apply trim
		value += mapping.user_trim;
		// apply factor
		value *= mapping.factor;
		// apply expo. this is from https://www.rcgroups.com/forums/showpost.php?p=38339227&postcount=6
		value = ((1 - mapping.expo) * std::pow(value, 3.0f)) + (mapping.expo * value);

		// apply offset
		value += mapping.offset;

		// set value
		this->m_mixer.set_mixing_value(mapping.ppm_channel_id, 
			value,
			mapping.mixing_id);
	}
	
	// check if this axis is to be buttonized
	auto item2 = this->m_config.Buttonizations.find(number);
	if (item2 != this->m_config.Buttonizations.end())
	{
		// check if low or high
		auto& virt = item2->second;
		this->updateVirtualButton(virt.low_button_id, (value < -0.5));
		this->updateVirtualButton(virt.high_button_id, (value > 0.5));
	}
	
}

void joystick_ppm::updateVirtualButton(unsigned char number, bool value)
{
	bool bUpdate = true;
	auto item = this->m_mapVirtualButtons.find(number);
	if (item != this->m_mapVirtualButtons.end())
	{
		bUpdate = item->second != value;
	}
	this->m_mapVirtualButtons[number] = value;
	
	if (bUpdate)
	{
		this->put_button(number, value);
	}
}

void joystick_ppm::onButtonUpdated(unsigned char number)
{
	// get value first
	auto value = this->get_button(number);
	
	// check if there is a mapping
	auto& map = (value ? this->m_config.ButtonPressActions : this->m_config.ButtonReleaseActions);
	auto item = map.find(number);
	if (item != map.end())
	{
		auto& action = item->second;
		
		if (action.type == ButtonActionType::SetValue)
		{
			// directly setting value is easy-cheesy
			this->m_mixer.set_mixing_value(action.axis_or_channel_id, action.value);
		}
		else if (action.type == ButtonActionType::AddTrim)
		{
			// we are going to modify trim
			auto axis = this->m_config.AxisMappings.find(action.axis_or_channel_id);
			if (axis != this->m_config.AxisMappings.end())
			{
				auto& mapping = axis->second;
				
				if (action.type == ButtonActionType::AddTrim)
					mapping.user_trim += action.value;
				
				// re-calc the axis and send to the PPM stream
				this->onAxisUpdated(action.axis_or_channel_id);
				
				// save config
				this->saveTrimValue(action.axis_or_channel_id, mapping.user_trim);
			}
		}
		else if (action.type == ButtonActionType::AddSubTrim)
		{
			// save subtrim in our map
			auto& subtrim = this->m_mapSubtrims[action.axis_or_channel_id];
			subtrim += action.value;
			
			// write to PPM stream. mixing id "-1" == subtrim
			this->m_mixer.set_mixing_value(action.axis_or_channel_id, subtrim, -1);
			
			// save config
			this->saveSubtrimValue(action.axis_or_channel_id, subtrim);
		}
	}
}

void joystick_ppm::loadUserTrim(unsigned char axis_number, float trim)
{
	auto axis = this->m_config.AxisMappings.find(axis_number);
	if (axis != this->m_config.AxisMappings.end())
	{
		auto& mapping = axis->second;
		mapping.user_trim = trim;
	}
}

void joystick_ppm::loadUserSubtrim(unsigned char ppm_channel_id, float subtrim)
{
	this->m_mapSubtrims[ppm_channel_id] = subtrim;
	
	// also save in mixer
	this->m_mixer.set_mixing_value(ppm_channel_id, subtrim, -1, true);
}


void joystick_ppm::saveTrimValue(unsigned char axis_number, float trim) {}
void joystick_ppm::saveSubtrimValue(unsigned char axis_number, float trim) {}

