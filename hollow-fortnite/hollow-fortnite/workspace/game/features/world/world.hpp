#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/core/sdk.hpp>
#include <workspace/game/entity/enumerations/enumerations.hpp>

namespace m_class {
    class m_world_query
    {
    private:
        static bool is_loot_entity( const std::string & name ) {
            return name.find( "FortPickupAthena" ) != std::string::npos ||
                name.find( "Fort_Pickup_Creative_C" ) != std::string::npos ||
                name.find( "FortPickup" ) != std::string::npos ||
                name.find( "Tiered_Chest" ) != std::string::npos ||
                name.find( "AlwaysSpawn_NormalChest" ) != std::string::npos ||
                name.find( "AlwaysSpawn_RareChest" ) != std::string::npos ||
                name.find( "Tiered_Ammo" ) != std::string::npos ||
                name.find( "B_LinedNotebookSuppyDrop_Ocean_C" ) != std::string::npos ||
                name.find( "B_LinedNotebookSuppyDrop_Lake_C" ) != std::string::npos ||
                name.find( "Cooler_Container_C" ) != std::string::npos ||
                name.find( "AthenaSupplyDrop_Llama_C" ) != std::string::npos;
        }

        enum loot_type {
            entity_loot,
            entity_chest,
            entity_ammo,
            entity_supply_drop,
            entity_llama,
            entity_container
        };

        static bool cache_names( const std::string & name ) {
            if ( !is_loot_entity( name ) ) return false;
            loot_type type = get_entity_type( name );

            return true;
        }

        static loot_type get_entity_type( const std::string & name ) {
            if ( name.find( "Tiered_Chest" ) != std::string::npos || name.find( "AlwaysSpawn_NormalChest" ) != std::string::npos || name.find( "AlwaysSpawn_RareChest" ) != std::string::npos ) {
                return entity_chest;
            }
            else if ( name.find( "Tiered_Ammo" ) != std::string::npos ) {
                return entity_ammo;
            }
            else if ( name.find( "B_LinedNotebookSuppyDrop_Ocean_C" ) != std::string::npos || name.find( "B_LinedNotebookSuppyDrop_Lake_C" ) != std::string::npos ) {
                return entity_supply_drop;
            }
            else if ( name.find( "AthenaSupplyDrop_Llama_C" ) != std::string::npos ) {
                return entity_llama;
            }
            else if ( name.find( "Cooler_Container_C" ) != std::string::npos ) {
                return entity_container;
            }
            return entity_loot;
        }

        static inline uintptr_t get_root( sdk::a_fort_player_pawn* actor ) {
            return kernel->read< std::uintptr_t >(std::uintptr_t( actor ) + 0x1b0);
        }

        static inline primitives::fvector get_location( sdk::a_fort_player_pawn* actor ) {
            std::uintptr_t root = get_root( actor );
            
            return kernel->read< primitives::fvector >( root + 0x138 );
        }

        static std::string read_ftext_string( std::uintptr_t data_ptr, int length ) {
            if ( length <= 0 || length > 100 ) {
                return encrypt("");
            }

            std::unique_ptr< wchar_t[ ] > buffer( new wchar_t[ length ] );
            kernel->read_memory( data_ptr, ( PVOID ) buffer.get( ), length * sizeof( wchar_t ) );

            std::wstring string( buffer.get( ) );
            return std::string( string.begin( ), string.end( ) );
        }

        static std::string get_pickup_name( sdk::a_fort_pawn* pawn ) {
            auto pickup_entry = pawn->pickup_entry( );
            if (!pickup_entry) return encrypt( "No Pickup" );

            auto item_name = pickup_entry->name( );
       
            auto f_data = item_name->f_data( );
          
            int f_length = item_name->f_length( );
            if ( f_length <= 0 || f_length > 100 ) 
                return encrypt( "Invalid Length" );

            return read_ftext_string( f_data, f_length );
        }

        static enumerations::e_fort_rarity get_pickup_rarity( sdk::a_fort_pawn * pawn ) {
            auto pickup_entry = pawn->pickup_entry( );
            if ( !pickup_entry ) return { };

            std::uintptr_t entry_ptr = reinterpret_cast< std::uintptr_t >( pickup_entry );
      
            return kernel->read< enumerations::e_fort_rarity >( entry_ptr + 0xa2 );
        }

        static inline ImColor get_loot_color( enumerations::e_fort_rarity tier ) {
            switch ( tier ) {
            case enumerations::e_fort_rarity::common: return ImColor( 224, 224, 224, 255 );
            case enumerations::e_fort_rarity::uncommon: return ImColor( 90, 169, 0, 255 );
            case enumerations::e_fort_rarity::rare: return ImColor( 97, 155, 242, 255 );
            case enumerations::e_fort_rarity::epic: return ImColor( 189, 63, 250, 255 );
            case enumerations::e_fort_rarity::legendary: return ImColor( 202, 164, 29, 255 );
            case enumerations::e_fort_rarity::mythic: return ImColor( 210, 203, 80, 255 );
            default: return ImColor( 224, 224, 224, 255 );
            }
        }


        // since fname for fortnite is encrypted we must decrypt it into a readable string
        class f_name {
        public:
            std::int32_t comparison_index;

            f_name( ) {
                comparison_index = 0;
            }

            f_name( std::uintptr_t object ) {
                if  (object ) {
                    comparison_index = kernel->read< std::int32_t >( object + 0x8 );
                }
            }

            std::string to_string( ) const {
                return to_string( comparison_index );
            }

            static std::string to_string( std::uintptr_t object ) {
                return f_name( object ).to_string( );
            }

            static std::string to_string( std::int32_t index ) {
                std::int32_t decrypted_index = decrypt_index( index );

                std::uint64_t name_pool_chunk = kernel->read< std::uint64_t >(
                    kernel->m_base_address + ( 0x1885d640 + 8 * ( decrypted_index >> 16 ) + 16 )
                );
                name_pool_chunk += 2 * ( std::uint16_t )decrypted_index;

                std::uint16_t pool = kernel->read< std::uint16_t >( name_pool_chunk );

                if ( ( ( pool ^ 0x80 ) & 0x7fe0 ) == 0 ) {
                    decrypted_index = decrypt_index( kernel->read< std::int32_t >( name_pool_chunk + 2 ) );

                    name_pool_chunk = kernel->read< std::uint64_t> (
                        kernel->m_base_address + ( 0x1885d640 + 8 * ( decrypted_index >> 16 ) + 16 )
                    );
                    name_pool_chunk += 2 * ( std::uint16_t )decrypted_index;

                    pool = kernel->read< std::uint16_t >( name_pool_chunk );
                }

                std::int32_t length = ( ( pool ^ 0x80u ) >> 5 ) & 0x3ff;

                std::vector< char > name_buffer( length + 1 );
                kernel->read_memory( name_pool_chunk + 2, name_buffer.data( ), length );
                decrypt_fname( name_buffer.data( ), length );

                return std::string( name_buffer.data( ) );
            }

            static std::int32_t decrypt_index( std::int32_t index ) {
                if ( index ) {
                    std::int32_t decrypted_index = _rotr( index - 1, 1 ) - 0x527d388b;
                    return decrypted_index ? decrypted_index : 0xad82c774;
                }

                return 0;
            }

            static void decrypt_fname( char* buffer, std::int32_t length ) {
                if ( length ) {
                    std::vector< std::uint8_t > encrypted_buffer( buffer, buffer + length );

                    std::int32_t v = 8962 * length + 21856876;
                    for ( std::int32_t i = 0; i < length; i++ ) {
                        v = 8962 * v + 21856876;
                        buffer[ length - 1 - i ] = char( v + _rotr8( encrypted_buffer[ i ], 1 ) + 6 );
                    }
                }

                buffer[ length ] = '\0';
            }
        };

        class item {
        public:
            sdk::a_fort_player_pawn * m_pawn;
            std::string m_name, m_pickup;
            float m_distance;
        };

        static inline std::mutex m_w_mutex;
        static inline std::vector< item > m_e_world_cache;

    public:
        static void tick( );
        static void render( );
    };
}