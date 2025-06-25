#pragma once
#include <impl/includes.hpp>

namespace m_class {
	class m_visuals_query {
	private:
		static std::mutex m_mutex; 
	public:

		static void tick( ); 
	};
}