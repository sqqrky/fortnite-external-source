#include <impl/includes.hpp>
#include <workspace/game/entity/entity.hpp>
#include <workspace/game/features/world/world.hpp>
#include <workspace/game/render/render.hpp>
#include <workspace/unreal/exception/exception.hpp>

auto main( ) -> std::int32_t {
	SetConsoleTitleA ( encrypt ( "hollow.wtf" ) );
	SetUnhandledExceptionFilter( m_class::m_exception_query::exception_filter );

	if ( !kernel->setup( ) )
		return std::getchar( );

	if ( !kernel->attach( target_process ) ) {
		printf( encrypt ( "> failed to gather information to the target process.\n" ) );
		kernel->unload( );
		return std::getchar( );
	}

	std::jthread( [ & ]( ) -> void { m_class::m_tick_query::tick( ); } ).detach( );
	std::jthread( [ & ]( ) -> void { m_class::m_world_query::tick( ); } ).detach( );

	m_class::m_render_query::hijack_window( ); 
	m_class::m_render_query::create_device( ); 
	m_class::m_render_query::tick( );

	std::cin.get( ); 
}