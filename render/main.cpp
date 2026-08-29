#include <pch/pch.hpp>

#include "xdraw/xdraw.hpp"
#include "xdraw/xui/xui.hpp"
#include "rendering/fonts.hpp"
#include "menu/menu.hpp"

using Microsoft::WRL::ComPtr;

namespace app {

	namespace detail {

		struct globals
		{
			HWND hwnd{};
			ComPtr<ID3D11Device> device{};
			ComPtr<ID3D11DeviceContext> context{};
			ComPtr<IDXGISwapChain> swapchain{};
			ComPtr<ID3D11RenderTargetView> rtv{};
			int width{ 1280 };
			int height{ 800 };
		};

		static globals g{};

		static bool create_render_target()
		{
			ComPtr<ID3D11Texture2D> backbuffer{};
			if (FAILED(g.swapchain->GetBuffer(0, IID_PPV_ARGS(&backbuffer))))
			{
				return false;
			}

			return SUCCEEDED(g.device->CreateRenderTargetView(backbuffer.Get(), nullptr, &g.rtv));
		}

		static void bind_render_target()
		{
			ID3D11RenderTargetView* views[]{ g.rtv.Get() };
			g.context->OMSetRenderTargets(1, views, nullptr);

			D3D11_VIEWPORT vp{};
			vp.Width = static_cast<float>(g.width);
			vp.Height = static_cast<float>(g.height);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;

			g.context->RSSetViewports(1, &vp);
		}

		static void draw_fps(rendering::fonts& fonts)
		{
			char buf[32]{};
			std::snprintf(buf, sizeof(buf), "FPS: %.0f", xdraw::framerate());

			const auto font = fonts.inter[rendering::size::medium];
			const auto [text_w, text_h] = xdraw::measure_text(buf, font);

			const auto [vp_w, vp_h] = xdraw::viewport_size();
			constexpr auto margin{ 12.0f };

			const auto x = static_cast<float>(vp_w) - text_w - margin;
			const auto y = margin;

			xdraw::get(xdraw::layer::top).text(x, y, buf, xdraw::color{ 25, 25, 30, 255 }, font);
		}

		static void resize(int width, int height)
		{
			if (width <= 0 || height <= 0 || !g.swapchain)
			{
				return;
			}

			g.width = width;
			g.height = height;

			g.context->OMSetRenderTargets(0, nullptr, nullptr);
			g.rtv.Reset();

			g.swapchain->ResizeBuffers(0, static_cast<UINT>(width), static_cast<UINT>(height), DXGI_FORMAT_UNKNOWN, 0);

			create_render_target();
			bind_render_target();
		}

		static bool create_device()
		{
			DXGI_SWAP_CHAIN_DESC desc{};
			desc.BufferDesc.Width = static_cast<UINT>(g.width);
			desc.BufferDesc.Height = static_cast<UINT>(g.height);
			desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.BufferDesc.RefreshRate.Numerator = 60;
			desc.BufferDesc.RefreshRate.Denominator = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			desc.BufferCount = 1;
			desc.OutputWindow = g.hwnd;
			desc.Windowed = TRUE;
			desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			UINT flags{};
#ifdef _DEBUG
			flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

			const D3D_FEATURE_LEVEL levels[]{ D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
			D3D_FEATURE_LEVEL out_level{};

			const auto hr = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				flags,
				levels,
				static_cast<UINT>(std::size(levels)),
				D3D11_SDK_VERSION,
				&desc,
				&g.swapchain,
				&g.device,
				&out_level,
				&g.context);

			if (FAILED(hr))
			{
				return false;
			}

			if (!create_render_target())
			{
				return false;
			}

			bind_render_target();
			return true;
		}

		static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			xui::wndproc(msg, wp, lp);

			switch (msg)
			{
			case WM_SIZE:
				if (wp != SIZE_MINIMIZED)
				{
					resize(LOWORD(lp), HIWORD(lp));
				}
				return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			}

			return DefWindowProcW(hwnd, msg, wp, lp);
		}

	}

	static bool create_window(HINSTANCE instance)
	{
		WNDCLASSEXW wc{};
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = detail::wndproc;
		wc.hInstance = instance;
		wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wc.lpszClassName = L"xdraw_app_window";

		RegisterClassExW(&wc);

		const DWORD style = WS_OVERLAPPEDWINDOW;
		RECT r{ 0, 0, detail::g.width, detail::g.height };
		AdjustWindowRect(&r, style, FALSE);

		detail::g.hwnd = CreateWindowExW(
			0,
			wc.lpszClassName,
			L"xdraw",
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			r.right - r.left,
			r.bottom - r.top,
			nullptr,
			nullptr,
			instance,
			nullptr);

		if (!detail::g.hwnd)
		{
			return false;
		}

		ShowWindow(detail::g.hwnd, SW_SHOWDEFAULT);
		UpdateWindow(detail::g.hwnd);
		return true;
	}

	static int run(HINSTANCE instance)
	{
		if (!create_window(instance))
		{
			return 1;
		}

		if (!detail::create_device())
		{
			return 1;
		}

		if (!xdraw::initialize(detail::g.device.Get(), detail::g.context.Get()))
		{
			return 1;
		}

		xui::initialize(detail::g.hwnd);

		rendering::fonts fonts{};
		fonts.initialize();

		menu::state menu_state{};

		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				continue;
			}

			detail::bind_render_target();

			const float clear_col[]{ 0.82f, 0.82f, 0.84f, 1.0f };
			detail::g.context->ClearRenderTargetView(detail::g.rtv.Get(), clear_col);

			xdraw::begin_frame();
			xui::begin();

			menu::render(menu_state, fonts);
			detail::draw_fps(fonts);

			xui::end();
			xdraw::end_frame();

			detail::g.swapchain->Present(1, 0);
		}

		return 0;
	}

}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	const auto result = app::run(instance);

	CoUninitialize();
	return result;
}