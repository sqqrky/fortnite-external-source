#pragma once
#include <impl/includes.hpp>
namespace m_class {
    class m_render_query {
    private:
        // this is for our window
        static inline HWND m_hwnd;
        static inline ID3D11Device* m_device;
        static inline ID3D11DeviceContext* m_device_ctx;
        static inline IDXGISwapChain* m_device_swap_chain;
        static inline ID3D11RenderTargetView* m_device_render_target;
        static inline MARGINS m_window_margin = { -1 };

        // this is for the fortnite window 
        static inline HWND m_fortnite_window;
        static inline RECT m_fortnite_rect;
        static inline int m_pos_x;
        static inline int m_pos_y;
        static inline int m_window_width ;
        static inline int m_window_height;
        static inline int m_render_width;
        static inline int m_render_height;
    public:
        static void hijack_window ( );
        static void refresh_window ( );
        static void create_device ( );
        static void cleanup_device ( );
        static void render ( );
        static void tick ( );
    };
}