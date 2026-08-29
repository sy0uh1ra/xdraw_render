#include <pch/pch.hpp>

#include "menu.hpp"

namespace menu {

	void render( state& s, rendering::fonts& f )
	{
		s.anim_t += xdraw::delta_time( );

		if ( xui::begin_window( "example", s.window_x, s.window_y, s.window_w, s.window_h, true, 320.0f, 220.0f ) )
		{
			auto& dl = xui::draw::current( );

			const auto [ title_w, title_h ] = xdraw::measure_text( "xdraw test window", f.inter[ rendering::size::large ] );
			const auto title_rect = xui::layout::item( title_w, title_h );

			dl.text( title_rect.x, title_rect.y, "xdraw test window", tokens::col_text, f.inter[ rendering::size::large ] );

			xui::text( "moving rounded rect below", tokens::col_text_dim );
			xui::layout::spacing( 8.0f );

			const auto [ avail_w, avail_h ] = xui::layout::avail( );
			const auto area = xui::layout::item( avail_w, 120.0f );

			dl.rect_filled( area.x, area.y, area.w, area.h, tokens::col_card, xdraw::corner_radius{ 10.0f } );
			dl.push_clip( area.x, area.y, area.w, area.h );

			constexpr auto rect_w{ 60.0f };
			constexpr auto rect_h{ 60.0f };
			const auto range = area.w - rect_w;
			const auto t = std::sinf( s.anim_t * 2.0f ) * 0.5f + 0.5f;
			const auto x = area.x + t * range;
			const auto y = area.y + area.h * 0.5f - rect_h * 0.5f;

			dl.rect_filled( x, y, rect_w, rect_h, tokens::col_accent, xdraw::corner_radius{ 14.0f } );

			dl.pop_clip( );
			xui::layout::spacing( 10.0f );

			static xui::setting demo_toggle{ false, {}, "demo_toggle", "example" };
			xui::checkbox( "toggle something", demo_toggle );

			xui::end_window( );
		}
	}

}
