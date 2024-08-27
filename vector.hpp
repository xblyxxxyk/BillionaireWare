#pragma once
#include <numbers>
#include <cmath>
#include <Windows.h>

struct view_matrix_t
{
	float* operator[](int index)
	{
		return matrix[index];
	}

	float matrix[4][4];
};
#pragma once

#include <cmath>
#include <numbers>

struct vec4
{
	float w, x, y, z;
};

struct vec3
{
	float x, y, z;
	vec3 operator+(vec3 other)
	{
		return { this->x + other.x, this->y + other.y, this->z + other.z };
	}
	vec3 operator-(vec3 other)
	{
		return { this->x - other.x, this->y - other.y, this->z - other.z };
	}
	vec3 operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}
	vec3 operator-(const vec3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}
	vec3 RelativeAngle(const vec3& src, const vec3& dst) {
		// Calculate the delta (difference) vector
		vec3 delta;
		delta.x = dst.x - src.x;
		delta.y = dst.y - src.y;
		delta.z = dst.z - src.z;

		// Calculate angles
		vec3 angles;
		angles.x = std::atan2(-delta.z, std::hypot(delta.x, delta.y)) * (180.0f / std::numbers::pi_v<float>);
		angles.y = std::atan2(delta.y, delta.x) * (180.0f / std::numbers::pi_v<float>);
		angles.z = 0.0f;  // Typically, roll isn't needed in aimbots

		// Normalize yaw angle if needed
		if (angles.y < 0.0f)
			angles.y += 360.0f;

		return angles;
	}



};

struct vec2
{
	float x, y;
};
class Vector
{
public:
	constexpr Vector(
		const float x = 0.f,
		const float y = 0.f,
		const float z = 0.f) noexcept :
		x(x), y(y), z(z) { }
	
	constexpr const Vector& operator-(const Vector& other) const noexcept;
	constexpr const Vector& operator+(const Vector& other) const noexcept;
	constexpr const Vector& operator/(const float factor) const noexcept;
	constexpr const Vector& operator*(const float factor) const noexcept;

	// 3d -> 2d, explanations already exist.
	const static bool world_to_screen(view_matrix_t view_matrix, Vector& in, Vector& out);

	const bool IsZero();

	float x, y, z;
};

