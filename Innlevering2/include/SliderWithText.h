/********************************************************************
    created:    20:3:2013   18:28
    filename:   SliderWithText.h
    author:     Kristian Skarseth
    
    purpose:    A simple GUI slider that can be modified with mouse input.
				Represents a value between [0, 1], or another defined value.
*********************************************************************/
#ifndef SliderWithText_h__
#define SliderWithText_h__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>
#include <gl/glew.h>

#include <string>

#include "GUI_Util.h"
#include "GUITexture.h"

namespace gui
{
	class SliderWithText
	{
	public:
		/**
		* SliderWithText constructor
		*
		* @param name_label_path full name and path, including extension, to the texture holding
		*					     the label name for this slider
		* @param gui_program the shader program used to draw GUI objects, used to set 
		*/
		SliderWithText(const std::string& name_label_path, glm::vec2 position, 
							glm::vec2 scale = glm::vec2(0.6,0.6));

		~SliderWithText();

		/**
		* Draws the slider to screen.
		*
		* @param gui_program the shader program used to draw GUI objects
		*/
		void Draw();

		/**
		* Sets the Sliders state to updating. Mouse input after this function
		* is called will affect the slider knobs position if Update is called.
		*
		* @param mouse_pos position of the mouse in SDL event.motion coordinates
		* @return true if interaction with the particular slider was started. False otherwise.
		*/
		bool BeginInteraction(glm::vec2& mouse_pos);
	
		/**
		* Sets the sliders state to not updating. Mouse input after this function
		* is called will not affect the slider knobs position.
		*
		* @param mouse_pos position of the mouse in SDL event.motion coordinates
		*/
		void EndInteraction(glm::vec2& mouse_pos);

		/**
		* Updates the position of the slider based on the mouse position.
		* If BeginInteraction has not been called since startup or previous
		* EndInteraction call, the position will not be updated.
		*
		* @param delta_time time since last game update
		* @param mouse_pos position of the mouse in SDL event.motion coordinates
		*/
		void Update(float delta_time, glm::vec2& mouse_pos);

		/**
		* Returns the value the slider represents, based on the position of the slider knob.
		* The value is clamped between [0, 1]
		*/
		float get_slider_value();

		/**
		* Sets the range the sliders value should be clamped to.
		* If this function is not set, the clamping range is [0, 1]
		*/
		void SetClampRange(float min, float max);

	private:
		GUITexture slider;
		GUITexture slider_knob;
		GUITexture label;
	
		bool interacting;
		glm::vec2 previous_mouse_pos;

		// The value the slider represents, based on the position of the slider knob.
		// The value is by default clamped between [0, 1]
		float slider_value;
		float clamp_min, clamp_max;
		void UpdateSliderValue();

	};
};


#endif // SliderWithText_h__