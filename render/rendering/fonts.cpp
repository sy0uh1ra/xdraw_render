#include <pch/pch.hpp>

#include "fonts.hpp"
#include "../resources/fonts.hpp"

namespace rendering {

	void fonts::initialize( )
	{
		this->load_family( this->inter, std::as_bytes( std::span{ resources::fonts::inter } ), { 12.0f, 15.0f, 20.0f } );

		this->verdana = xdraw::load_system_font( "Verdana", 12.0f, 400, xdraw::font_quality::cleartype_natural );
	}

	void fonts::load_family( family_t& family, std::span<const std::byte> data, const std::array<float, static_cast< std::size_t >( size::count )>& sizes )
	{
		for ( auto i = 0ull; i < sizes.size( ); ++i )
		{
			family.sizes[ i ] = xdraw::load_font( data, sizes[ i ] );
		}
	}

}
