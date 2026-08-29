#pragma once

#include "../xdraw/xdraw.hpp"

#include <array>
#include <cstdint>
#include <span>

namespace rendering {

	enum class size : std::uint8_t
	{
		smallest,
		medium,
		large,
		count
	};

	struct family_t
	{
		std::array<xdraw::font*, static_cast< std::size_t >( size::count )> sizes{};

		[[nodiscard]] xdraw::font* operator[]( size s ) const { return this->sizes[ static_cast< std::size_t >( s ) ]; }
	};

	struct fonts
	{
		family_t inter{};
		xdraw::font* verdana{};

		void initialize( );
		void load_family( family_t& family, std::span<const std::byte> data, const std::array<float, static_cast< std::size_t >( size::count )>& sizes );
	};

}
