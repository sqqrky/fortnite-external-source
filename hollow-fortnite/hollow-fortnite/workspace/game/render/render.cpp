#include <workspace/game/render/render.hpp>
#include <workspace/unreal/core/math/math.hpp>
#include <workspace/game/features/world/world.hpp>
#include <dependencies/framework/framework.hpp>
#include <workspace/game/features/visuals/visuals.hpp>

// i wouldn't reccomend continuing to use this,
// find a screen recorder/clipping tool and hijack the window
// though this is fine for the time being.
void m_class::m_render_query::hijack_window( ) {
	m_hwnd = FindWindowA ( encrypt ( "Chrome_WidgetWin_1" ), encrypt( "Discord Overlay" ) );

	if ( !m_hwnd )
		return; 
	
	DwmExtendFrameIntoClientArea ( m_hwnd, &m_window_margin ); 
	SetLayeredWindowAttributes ( m_hwnd, RGB ( 0, 0, 0 ) , 255, LWA_ALPHA );

	UpdateWindow ( m_hwnd ); 
	ShowWindow ( m_hwnd, SW_SHOW );
}

// doubt you'll ever have to considering the window is default 1920x1080 if your a real person
void m_class::m_render_query::refresh_window( ) {
	SetWindowPos( m_hwnd, NULL, NULL, NULL, primitives::screen_width, primitives::screen_height, 0 );
}

void m_class::m_render_query::create_device() {
    DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
    swap_chain_description.BufferCount = 2;
    swap_chain_description.BufferDesc.Width = 0;
    swap_chain_description.BufferDesc.Height = 0;
    swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_description.BufferDesc.RefreshRate.Numerator = 240;
    swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_description.OutputWindow = m_hwnd;
    swap_chain_description.SampleDesc.Count = 1;
    swap_chain_description.SampleDesc.Quality = 0;
    swap_chain_description.Windowed = TRUE;
    swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL d3d_feature_array[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    D3D_FEATURE_LEVEL d3d_feature_lvl;
    HRESULT swap_chain_result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        d3d_feature_array,
        2,
        D3D11_SDK_VERSION,
        &swap_chain_description,
        &m_device_swap_chain,
        &m_device,
        &d3d_feature_lvl,
        &m_device_ctx
    );

    if (FAILED(swap_chain_result)) {
        cleanup_device();
        return;
    }

    ID3D11Texture2D* buffer = nullptr;
    swap_chain_result = m_device_swap_chain->GetBuffer(0, IID_PPV_ARGS(&buffer));
    if (FAILED(swap_chain_result) || !buffer) {
        cleanup_device();
        return;
    }

    swap_chain_result = m_device->CreateRenderTargetView(buffer, nullptr, &m_device_render_target);
    buffer->Release();

    if (FAILED(swap_chain_result)) {
        cleanup_device();
        return;
    }

    if (!ImGui::GetCurrentContext()) {
        ImGui::CreateContext();
    }

    if (!ImGui_ImplWin32_Init(m_hwnd) || !ImGui_ImplDX11_Init(m_device, m_device_ctx)) {
        cleanup_device();
        return;
    }

    ImGuiIO& im_io = ImGui::GetIO(); (void)im_io;
}

void m_class::m_render_query::cleanup_device( ) {
    if ( m_device_ctx ) { 
        m_device_ctx->Release( ); 
        m_device_ctx = nullptr; 
    }

    if ( m_device_render_target ) { 
        m_device_render_target->Release( ); 
        m_device_render_target = nullptr; 
    }

    if ( m_device_swap_chain ) { 
        m_device_swap_chain->Release( ); 
        m_device_swap_chain = nullptr; 
    }

    if ( m_device ) { 
        m_device->Release( ); 
        m_device = nullptr; 
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void m_class::m_render_query::render ( ) {
    ImGui_ImplDX11_NewFrame( );
    ImGui_ImplWin32_NewFrame( );
    ImGui::NewFrame( );

    if ( GetAsyncKeyState ( VK_F2 ) )
    {
        refresh_window( );
    }

    m_class::m_visuals_query::tick( );

    m_class::m_world_query::render( );

    ImGui::Render( );

    m_device_ctx->ClearRenderTargetView( m_device_render_target, [ ] { static const float const_float[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f }; return const_float; } ( ) );

    m_device_ctx->OMSetRenderTargets( 1, &m_device_render_target, nullptr );

    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

    m_device_swap_chain->Present( g_vars->misc.vsync ? 1 : 0, 0 );
}

void m_class::m_render_query::tick( ) {
    RECT original_rect = { 0, 0, 0, 0 };

    MSG tag = { 0 };

    while ( true ) {
        if ( PeekMessage( &tag, m_hwnd, 0, 0, PM_REMOVE ) ) {
            TranslateMessage( &tag );
            DispatchMessage( &tag );

            if ( tag.message == WM_QUIT )
                break;
        }

        m_fortnite_window = FindWindowA ( encrypt ( "UnrealWindow" ), encrypt ( "Fortnite  " ) );
        if ( GetWindow( m_fortnite_window, 0 ) == nullptr ) {
            SendMessage( m_hwnd, WM_DESTROY, 0, 0);
            break;
        }
        HWND foreground_hwnd = GetForegroundWindow( );
        if ( foreground_hwnd == m_fortnite_window || foreground_hwnd == m_hwnd ) {
            POINT point;
            ZeroMemory( &m_fortnite_rect, sizeof( RECT ) );
            ZeroMemory( &point, sizeof(POINT));

            GetClientRect( m_fortnite_window, &m_fortnite_rect );
            ClientToScreen( m_fortnite_window, &point );
            m_fortnite_rect.left = point.x;
            m_fortnite_rect.top = point.y;

            if ( m_fortnite_rect.left != original_rect.left || m_fortnite_rect.right != original_rect.right ||
                m_fortnite_rect.top != original_rect.top || m_fortnite_rect.bottom != original_rect.bottom) {
                original_rect = m_fortnite_rect;
                m_pos_x = m_fortnite_rect.left;
                m_pos_y = m_fortnite_rect.top;
                m_window_width = m_fortnite_rect.right;
                m_window_height = m_fortnite_rect.bottom;

                m_render_width = m_fortnite_rect.right;
                m_render_height = m_fortnite_rect.bottom;

                SetWindowPos( m_hwnd, 0, point.x, point.y, m_render_width, m_render_height, SWP_NOREDRAW );
            }

            ImGuiIO& io = ImGui::GetIO();

            io.DeltaTime = 1.0f / 60.0f;

            POINT point_cursor;
            if ( !GetCursorPos ( &point_cursor) ) {
                std::cout << encrypt( "> failed to retrieve cursor position" ) << std::endl; 
                continue;
            }

            ScreenToClient( m_hwnd, &point_cursor );
            io.MousePos.x = point_cursor.x;
            io.MousePos.y = point_cursor.y;

            io.MouseDown[ 0 ] =( GetAsyncKeyState ( VK_LBUTTON ) & 0x8000 ) != 0;
            io.MouseClicked[ 0 ] = io.MouseDown[ 0 ];

            m_class::m_render_query::render ( ); 

            HWND hwnd = GetWindow( foreground_hwnd, GW_HWNDPREV );
            if ( foreground_hwnd == m_hwnd ) {
                hwnd = GetWindow( m_fortnite_window, GW_HWNDPREV );
            }
            SetWindowPos( m_hwnd, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }
        else {
            float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            m_device_ctx->ClearRenderTargetView( m_device_render_target, [ ] { static const float const_float[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f }; return const_float; } ( ) );
            m_device_ctx->OMSetRenderTargets( 1, &m_device_render_target, NULL );
            m_device_swap_chain->Present( 0, 0 );
        }
    }

    ImGui_ImplDX11_Shutdown( );
    ImGui_ImplWin32_Shutdown( );
    ImGui::DestroyContext( );

    DestroyWindow( m_hwnd );
}