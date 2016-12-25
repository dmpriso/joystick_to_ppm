#ifndef JOYSTICK_PPM_H
#define JOYSTICK_PPM_H

#include "joystick_state.h"
#include "ppm_stream.h"

class ppm_mixer
{
public:
	ppm_mixer(ppm_target& target);
	
public:
	void set_mixing_value(unsigned char ppm_channel_id,
		float value,
		int mixing_id = 0,
		bool no_write = false);
	
private:
	ppm_target& m_target;
	
	std::map<unsigned char, std::map<int, float>> m_map;
	
};

class joystick_ppm : public joystick_state
{
public:
	struct axis_mapping_target
	{
		inline axis_mapping_target(unsigned char ppm_channel_id, 
			float factor, 
			float expo = 1.0f,
			float offset = 0.0f,
			float trim = 0.0f,
			int mixing_id = 0)
			: ppm_channel_id(ppm_channel_id),
			factor(factor),
			expo(expo),
			offset(offset),
			mixing_id(mixing_id > 0 ? mixing_id : --s_mixingid_counter)
		{}
		
		unsigned char ppm_channel_id;
		float factor;
		float expo;
		float offset;
		float user_trim = 0.0f;
		int mixing_id;
		
		static int s_mixingid_counter;
	};
	
	enum class ButtonActionType
	{
		SetValue = 0,	// axis_or_channel_id is a PPM channel ID
		AddTrim = 1,	// axis_or_channel_id is an axis id
		AddSubTrim = 2	// axis_or_channel_id is an axis id
	};
	
	struct button_action
	{
		inline button_action(ButtonActionType type,
			unsigned char axis_or_channel_id,
			float value)
			: type(type),
			axis_or_channel_id(axis_or_channel_id),
			value(value)
		{}

		ButtonActionType type;
		// depending on type, this is the axis number or the PPM channel ID
		unsigned char axis_or_channel_id;
		float value;
	};
	
	struct configuration
	{
		std::map<unsigned char, axis_mapping_target> AxisMappings;
		std::map<unsigned char, button_action> ButtonPressActions;
		std::map<unsigned char, button_action> ButtonReleaseActions;
	};

public:
	joystick_ppm(const configuration& config, ppm_target& target);
	
protected:
	void loadUserTrim(unsigned char axis_number, float trim);
	void loadUserSubtrim(unsigned char ppm_channel_id, float subtrim);
	
private:
	void writePPM(unsigned char channel);
	
private:
	void onAxisUpdated(unsigned char number) override;
	void onButtonUpdated(unsigned char number) override;
	
private:
	// should save trim & subtrim value to the configuration
	virtual void saveTrimValue(unsigned char axis_number, float trim);
	virtual void saveSubtrimValue(unsigned char ppm_channel_id, float subtrim);
	
protected:
	configuration m_config;
	std::map<unsigned char, float> m_mapSubtrims;

private:
	ppm_mixer m_mixer;
};


#endif


