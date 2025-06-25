#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/core/math/math.hpp>

namespace uengine {
	template < class type >
	class tarray
	{
	public:
		tarray( ) : data  (), count ( ), maximum_count( ) {}
		tarray ( type * data, std::uint32_t count, std::uint32_t max_count ) :
			data ( data ), count ( count ), maximum_count( max_count ) {
		}

		__forceinline type get ( std::uintptr_t idx )
		{
			return kernel -> read < type > (
				std::bit_cast < std::uintptr_t > ( this->data ) + ( idx * sizeof ( type ) )
			);
		}

		__forceinline std::vector < type >get_iteration ( )
		{
			if ( this->count > this->maximum_count )
				return { };

			try {
				std::vector < type > buffer ( this->count );

				kernel -> read_memory (
					reinterpret_cast < std::uintptr_t > ( this->data ),
					buffer . data (),
					sizeof ( type ) * this->count
				);

				return buffer;
			}
			catch ( std::bad_alloc & bad ) {
				return { };
			}
		}

		__forceinline std::uintptr_t get_address( )
		{
			return reinterpret_cast < std::uintptr_t >( this->data );
		}

		__forceinline std::uint32_t size( ) const
		{
			return this->count;
		};

		__forceinline std::uint32_t max_size( ) const
		{
			return this->maximum_count;
		};

		__forceinline bool is_valid_index( std::int32_t index ) const
		{
			return index >= 0 && index < static_cast<int>( count );
		};

		__forceinline bool is_valid ( ) const
		{
			return this -> data != nullptr;
		};

		type * data;
		std::uint32_t count;
		std::uint32_t maximum_count;
	};

	struct ftransform {
		primitives::fplane rotation;
		primitives::fvector translation;
		std::uint8_t padding_1 [ 8 ];
		primitives::fvector scale;
		std::uint8_t padding_2 [ 8 ];

		inline auto to_matrix( ) const -> D3DMATRIX {
			primitives::fvector scale_3d(
				( scale.x == 0.0 ) ? 1.0 : scale.x,
				( scale.y == 0.0 ) ? 1.0 : scale.y,
				( scale.z == 0.0 ) ? 1.0 : scale.z
			);

			double x2 = rotation.x + rotation.x;
			double y2 = rotation.y + rotation.y;
			double z2 = rotation.z + rotation.z;

			double xx2 = rotation.x * x2;
			double yy2 = rotation.y * y2;
			double zz2 = rotation.z * z2;
			double yz2 = rotation.y * z2;
			double wx2 = rotation.w * x2;
			double xy2 = rotation.x * y2;
			double wz2 = rotation.w * z2;
			double xz2 = rotation.x * z2;
			double wy2 = rotation.w * y2;

			D3DMATRIX matrix;

			matrix._41 = translation.x;
			matrix._42 = translation.y;
			matrix._43 = translation.z;

			matrix._11 = (1.0 - (yy2 + zz2)) * scale_3d.x;
			matrix._22 = (1.0 - (xx2 + zz2)) * scale_3d.y;
			matrix._33 = (1.0 - (xx2 + yy2)) * scale_3d.z;

			matrix._32 = (yz2 - wx2) * scale_3d.z;
			matrix._23 = (yz2 + wx2) * scale_3d.y;

			matrix._21 = (xy2 - wz2) * scale_3d.y;
			matrix._12 = (xy2 + wz2) * scale_3d.x;

			matrix._31 = (xz2 + wy2) * scale_3d.z;
			matrix._13 = (xz2 - wy2) * scale_3d.x;

			matrix._14 = 0.0f;
			matrix._24 = 0.0f;
			matrix._34 = 0.0f;
			matrix._44 = 1.0f;

			return matrix;
		}
	};

	inline D3DMATRIX to_multiplication(D3DMATRIX matrix1, D3DMATRIX matrix2)
	{
		D3DMATRIX result;
		result._11 = matrix1._11 * matrix2._11 + matrix1._12 * matrix2._21 + matrix1._13 * matrix2._31 + matrix1._14 * matrix2._41;
		result._12 = matrix1._11 * matrix2._12 + matrix1._12 * matrix2._22 + matrix1._13 * matrix2._32 + matrix1._14 * matrix2._42;
		result._13 = matrix1._11 * matrix2._13 + matrix1._12 * matrix2._23 + matrix1._13 * matrix2._33 + matrix1._14 * matrix2._43;
		result._14 = matrix1._11 * matrix2._14 + matrix1._12 * matrix2._24 + matrix1._13 * matrix2._34 + matrix1._14 * matrix2._44;
		result._21 = matrix1._21 * matrix2._11 + matrix1._22 * matrix2._21 + matrix1._23 * matrix2._31 + matrix1._24 * matrix2._41;
		result._22 = matrix1._21 * matrix2._12 + matrix1._22 * matrix2._22 + matrix1._23 * matrix2._32 + matrix1._24 * matrix2._42;
		result._23 = matrix1._21 * matrix2._13 + matrix1._22 * matrix2._23 + matrix1._23 * matrix2._33 + matrix1._24 * matrix2._43;
		result._24 = matrix1._21 * matrix2._14 + matrix1._22 * matrix2._24 + matrix1._23 * matrix2._34 + matrix1._24 * matrix2._44;
		result._31 = matrix1._31 * matrix2._11 + matrix1._32 * matrix2._21 + matrix1._33 * matrix2._31 + matrix1._34 * matrix2._41;
		result._32 = matrix1._31 * matrix2._12 + matrix1._32 * matrix2._22 + matrix1._33 * matrix2._32 + matrix1._34 * matrix2._42;
		result._33 = matrix1._31 * matrix2._13 + matrix1._32 * matrix2._23 + matrix1._33 * matrix2._33 + matrix1._34 * matrix2._43;
		result._34 = matrix1._31 * matrix2._14 + matrix1._32 * matrix2._24 + matrix1._33 * matrix2._34 + matrix1._34 * matrix2._44;
		result._41 = matrix1._41 * matrix2._11 + matrix1._42 * matrix2._21 + matrix1._43 * matrix2._31 + matrix1._44 * matrix2._41;
		result._42 = matrix1._41 * matrix2._12 + matrix1._42 * matrix2._22 + matrix1._43 * matrix2._32 + matrix1._44 * matrix2._42;
		result._43 = matrix1._41 * matrix2._13 + matrix1._42 * matrix2._23 + matrix1._43 * matrix2._33 + matrix1._44 * matrix2._43;
		result._44 = matrix1._41 * matrix2._14 + matrix1._42 * matrix2._24 + matrix1._43 * matrix2._34 + matrix1._44 * matrix2._44;

		return result;
	}

	inline static auto to_rotation_matrix(primitives::frotator rotation) -> D3DMATRIX {
		auto rad_pitch = (rotation.pitch * float(std::numbers::pi) / 180.f);
		auto rad_yaw = (rotation.yaw * float(std::numbers::pi) / 180.f);
		auto rad_roll = (rotation.roll * float(std::numbers::pi) / 180.f);

		auto sp = sinf(rad_pitch);
		auto cp = cosf(rad_pitch);
		auto sy = sinf(rad_yaw);
		auto cy = cosf(rad_yaw);
		auto sr = sinf(rad_roll);
		auto cr = cosf(rad_roll);

		D3DMATRIX matrix;
		matrix.m[0][0] = cp * cy;
		matrix.m[0][1] = cp * sy;
		matrix.m[0][2] = sp;
		matrix.m[0][3] = 0.f;

		matrix.m[1][0] = sr * sp * cy - cr * sy;
		matrix.m[1][1] = sr * sp * sy + cr * cy;
		matrix.m[1][2] = -sr * cp;
		matrix.m[1][3] = 0.f;

		matrix.m[2][0] = -(cr * sp * cy + sr * sy);
		matrix.m[2][1] = cy * sr - cr * sp * sy;
		matrix.m[2][2] = cr * cp;
		matrix.m[2][3] = 0.f;

		matrix.m[3][0] = 0.0f;
		matrix.m[3][1] = 0.0f;
		matrix.m[3][2] = 0.0f;
		matrix.m[3][3] = 1.f;

		return matrix;
	}

	struct alignas (16) matrix_elements {
		double m11, m12, m13, m14;
		double m21, m22, m23, m24;
		double m31, m32, m33, m34;
		double m41, m42, m43, m44;

		matrix_elements() : m11(0), m12(0), m13(0), m14(0),
			m21(0), m22(0), m23(0), m24(0),
			m31(0), m32(0), m33(0), m34(0),
			m41(0), m42(0), m43(0), m44(0) {
		}
	};

	struct alignas (16) dbl_matrix {
		union {
			matrix_elements elements;
			double m[4][4];
		};

		dbl_matrix() : elements() {}

		double& operator () (std::size_t row, std::size_t col) { return m[row][col]; }
		const double& operator () (std::size_t row, std::size_t col) const { return m[row][col]; }
	};

	struct alignas (16) fmatrix : public dbl_matrix {
		primitives::fplane x_plane;
		primitives::fplane y_plane;
		primitives::fplane z_plane;
		primitives::fplane w_plane;

		fmatrix() : dbl_matrix(), x_plane(), y_plane(), z_plane(), w_plane() {}
	};

	// had to inline these to make intellisense shut the fuck up
	inline primitives::fvector location { };
	inline primitives::frotator rotation { };
	inline float field_of_view;

	void update_matrix( );

	primitives::fvector2d world_to_screen( primitives::fvector location );
}