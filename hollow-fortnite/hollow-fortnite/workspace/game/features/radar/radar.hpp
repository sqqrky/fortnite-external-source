#pragma once
#include <impl/includes.hpp>

namespace m_class {
	class m_radar_query {
	private:
		std::mutex m_mutex;
	public:
		// do logic here
		static void tick( ); 
	};
}