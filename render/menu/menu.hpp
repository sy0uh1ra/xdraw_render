#pragma once

#include "../xdraw/xui/xui.hpp"
#include "../rendering/fonts.hpp"

namespace menu {

	struct state
	{
		float window_x{ 220.0f };
		float window_y{ 160.0f };
		float window_w{ 420.0f };
		float window_h{ 260.0f };
		float anim_t{};
	};

	void render( state& s, rendering::fonts& f );

}
