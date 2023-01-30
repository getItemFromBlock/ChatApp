#include <cstdio>

#include "Maths.hpp"

#include <assert.h>
#include <corecrt_math_defines.h>

namespace Maths
{

    // -----------------------   IVec2    -----------------------

    inline IVec2::IVec2(const Vec2& in) : x((int)in.x), y((int)in.y) {}

    inline IVec2 IVec2::operator+(const IVec2& a) const
    {
        IVec2 res = IVec2(a.x + this->x, a.y + this->y);
        return res;
    }

    inline IVec2 IVec2::operator-(const IVec2& a) const
    {
        return IVec2(x - a.x, y - a.y);
    }

    inline IVec2 IVec2::operator-() const
    {
        return IVec2(-x, -y);
    }

    inline IVec2 IVec2::operator*(const IVec2& a) const
    {
        IVec2 res = IVec2(this->x * a.x, this->y * a.y);
        return res;
    }

    inline IVec2 IVec2::operator*(const float& a) const
    {
        IVec2 res = IVec2(this->x * (int)a, this->y * (int)a);
        return res;
    }

    inline IVec2 IVec2::operator/(const float& a) const
    {
        if ((int)a == 0)
            return IVec2(_CRT_INT_MAX, _CRT_INT_MAX);
        IVec2 res = IVec2(x / (int)a, y / (int)a);
        return res;
    }

    inline bool IVec2::operator==(const IVec2& b) const
    {
        return (x == b.x && y == b.y);
    }

    // -----------------------   Vec2    -----------------------

    inline float Vec2::lengthSquared() const
    {
        return (x * x + y * y);
    }

    inline float Vec2::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec2 Vec2::operator+(const Vec2& a) const
    {
        Vec2 res = Vec2(a.x + this->x, a.y + this->y);
        return res;
    }

    inline Vec2 Vec2::operator-(const Vec2& a) const
    {
        Vec2 res = Vec2(this->x - a.x, this->y - a.y);
        return res;
    }

    inline Vec2 Vec2::operator-() const
    {
        return this->negate();
    }

    inline Vec2 Vec2::operator*(const Vec2& a) const
    {
        Vec2 res = Vec2(this->x * a.x, this->y * a.y);
        return res;
    }

    inline Vec2 Vec2::operator*(const float& a) const
    {
        Vec2 res = Vec2(this->x * a, this->y * a);
        return res;
    }

    inline Vec2 Vec2::operator/(const float& a) const
    {
        if (a == 0.0)
            return operator*(VEC_HIGH_VALUE);
        Vec2 res = operator*(1 / a);
        return res;
    }

    inline Vec2 Vec2::operator/(const Vec2& other) const
    {
        if (other.x == 0.0f || other.y == 0.0f)
            return operator*(VEC_HIGH_VALUE);
        return Vec2(x/other.x, y/other.y);
    }

    inline bool Vec2::operator==(const Vec2& b) const
    {
        return (x == b.x && y == b.y);
    }

    inline float& Vec2::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec2::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline bool Vec2::isCollinearWith(Vec2 a) const
    {
        float res = a.x * y - a.y * x;
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline float Vec2::dotProduct(Vec2 a) const
    {
        return (a.x * x + a.y * y);
    }

    inline float Vec2::crossProduct(Vec2 a) const
    {
        return (x * a.y - y * a.x);
    }

    inline Vec2 Vec2::unitVector() const
    {
        return operator/(getLength());
    }

    inline Vec2 Vec2::negate() const
    {
        return operator*(-1);
    }

    inline Vec2 Vec2::getNormal() const
    {
        return Vec2(-y, x);
    }

    inline bool Vec2::isIntEquivalent(Vec2 a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y);
    }

    inline float Vec2::getDistanceFromPoint(Vec2 a) const
    {
        float i = a.x - x;
        float j = a.y - y;
        return sqrtf(i * i + j * j);
    }

    inline float Maths::Vec2::GetAngle() const
    {
        return atan2f(y,x);
    }

    inline bool Vec2::isNearlyEqual(Vec2 a, float prec)
    {
        return (fabsf(x-a.x) < prec) && (fabsf(y - a.y) < prec);
    }

    // -----------------------   IVec3    -----------------------

    inline IVec3::IVec3(const Vec3& in) : x((int)in.x), y((int)in.y), z((int)in.z) {}

    inline IVec3 IVec3::operator+(const IVec3& a) const
    {
        return IVec3(a.x + this->x, a.y + this->y, a.z + this->z);
    }

    inline IVec3 IVec3::operator-(const IVec3& a) const
    {
        return IVec3(x - a.x, y - a.y, z - a.z);
    }

    inline IVec3 IVec3::operator*(const IVec3& a) const
    {
        IVec3 res = IVec3(this->x * a.x, this->y * a.y, this->z * a.z);
        return res;
    }

    inline IVec3 IVec3::operator*(const float& a) const
    {
        IVec3 res = IVec3(this->x * (int)a, this->y * (int)a, this->z * (int)a);
        return res;
    }

    inline IVec3 IVec3::operator/(const float& a) const
    {
        if ((int)a == 0)
            return IVec3(_CRT_INT_MAX, _CRT_INT_MAX, _CRT_INT_MAX);
        IVec3 res = IVec3(x / (int)a, y / (int)a, z / (int)a);
        return res;
    }

    inline bool IVec3::operator==(const IVec3& b) const
    {
        return (x == b.x && y == b.y && z == b.z);
    }

    inline const int& IVec3::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline int& IVec3::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    // -----------------------   Vec3    -----------------------

    inline float Vec3::lengthSquared() const
    {
        return (x * x + y * y + z * z);
    }

    inline float Vec3::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec3 Vec3::operator+(const Vec3& a) const
    {
        Vec3 res = Vec3(a.x + this->x, a.y + this->y, a.z + this->z);
        return res;
    }

    inline Vec3 Vec3::operator-(const Vec3& a) const
    {
        Vec3 res = Vec3(this->x - a.x, this->y - a.y, this->z - a.z);
        return res;
    }

    inline Vec3 Vec3::operator-() const
    {
        return this->negate();
    }

    inline Vec3 Vec3::operator*(const Vec3& a) const
    {
        Vec3 res = Vec3(this->x * a.x, this->y * a.y, this->z * a.z);
        return res;
    }

    inline Vec3 Vec3::operator*(const float& a) const
    {
        Vec3 res = Vec3(this->x * a, this->y * a, this->z * a);
        return res;
    }

    inline Vec3 Vec3::operator/(const float& a) const
    {
        if (a == 0.0f)
            return operator*(VEC_HIGH_VALUE);
        Vec3 res = operator*(1 / a);
        return res;
    }

    inline bool Vec3::operator==(const Vec3& b) const
    {
        return (x == b.x && y == b.y && z == b.z);
    }

    inline float& Vec3::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec3::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline float Vec3::dotProduct(Vec3 a) const
    {
        return (a.x * x + a.y * y + a.z * z);
    }

    inline bool Vec3::isCollinearWith(Vec3 a) const
    {
        float res = this->dotProduct(a);
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline Vec3 Vec3::crossProduct(Vec3 a) const
    {
        return Vec3((y * a.z) - (z * a.y), (z * a.x) - (x * a.z), (x * a.y) - (y * a.x));
    }

    inline Vec3 Vec3::unitVector() const
    {
        return operator/(getLength());
    }

    inline Vec3 Vec3::negate() const
    {
        return operator*(-1);
    }

    inline bool Vec3::isIntEquivalent(Vec3 a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y && (int)z == (int)a.z);
    }

    inline bool Vec3::isNearlyEqual(Vec3 a, float prec)
    {
        return (fabsf(x - a.x) < prec) && (fabsf(y - a.y) < prec) && (fabsf(z - a.z) < prec);
    }

    // -----------------------   Vec4    -----------------------

    inline Vec3 Vec4::getVector() const
    {
        return Vec3(x, y, z);
    }

    inline Vec4 Vec4::homogenize() const
    {
        return Vec4(getVector() / w);
    }

    inline Vec4 Vec4::unitVector() const
    {
        Vec4 homogenized = homogenize();
        return homogenized / homogenized.getVector().getLength();
    }

    inline float Vec4::lengthSquared() const
    {
        return homogenize().getVector().lengthSquared();
    }

    inline float Vec4::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Vec4 Vec4::operator+(const Vec4& a) const
    {
        return Vec4(x + a.x, y + a.y, z + a.z, w + a.w);
    }

    inline Vec4 Vec4::operator-(const Vec4& a) const
    {
        return Vec4(x - a.x, y - a.y, z - a.z, w - a.w);
    }

    inline Vec4 Vec4::operator-() const
    {
        return this->negate();
    }

    inline Vec4 Vec4::operator*(const Vec4& a) const
    {
        return Vec4(x * a.x, y * a.y, z * a.z, w * a.w);
    }

    inline Vec4 Vec4::operator*(const float& a) const
    {
        return Vec4(x * a, y * a, z * a, w * a);
    }

    inline Vec4 Vec4::operator/(const float& b) const
    {
        if (b == 0.0f)
            return operator*(VEC_HIGH_VALUE);
        Vec4 res = operator*(1 / b);
        return res;
    }

    inline bool Vec4::operator==(const Vec4& b) const
    {
        return (x == b.x && y == b.y && z == b.z && w == b.w);
    }

    inline float& Vec4::operator[](const size_t a)
    {
        return *((&x) + a);
    }

    inline const float& Vec4::operator[](const size_t a) const
    {
        return *((&x) + a);
    }

    inline bool Vec4::isCollinearWith(Vec4 a) const
    {
        float res = dotProduct(a);
        return (res < VEC_COLLINEAR_PRECISION);
    }

    inline float Vec4::dotProduct(Vec4 a) const
    {
        return (a.x * x + a.y * y + a.z * z);
    }

    inline Vec4 Vec4::crossProduct(Vec4 a) const
    {
        return Vec4((y * a.z) - (z * a.y), (z * a.x) - (x * a.z), (x * a.y) - (y * a.x), 1.0f);
    }

    inline Vec4 Vec4::negate() const
    {
        return operator*(-1);
    }

    inline bool Vec4::isIntEquivalent(Vec4 a) const
    {
        return ((int)x == (int)a.x && (int)y == a.y && (int)z == (int)a.z && (int)w == (int)a.w);
    }

    inline bool Vec4::isNearlyEqual(Vec4 a, float prec)
    {
        return (fabsf(x - a.x) < prec) && (fabsf(y - a.y) < prec) && (fabsf(z - a.z) < prec) && (fabsf(w - a.w) < prec);
    }

    // -----------------------   Color4    -----------------------

    inline Color4::Color4(const float* in)
    {
        r = (unsigned char)(in[0] * 255);
        g = (unsigned char)(in[1] * 255);
        b = (unsigned char)(in[2] * 255);
        a = (unsigned char)(in[3] * 255);
    }

    inline Color4::Color4(const Vec4& in)
    {
        r = (unsigned char)(Util::clamp(in[0], 0.0f, 1.0f) * 255);
        g = (unsigned char)(Util::clamp(in[1], 0.0f, 1.0f) * 255);
        b = (unsigned char)(Util::clamp(in[2], 0.0f, 1.0f) * 255);
        a = (unsigned char)(Util::clamp(in[3], 0.0f, 1.0f) * 255);
    }

    inline Color4 Color4::operator*(const float& in) const
    {
        return Color4(r * (int)in, g * (int)in, b * (int)in, a);
    }

    inline Color4 Color4::operator+(const Color4& in) const
    {
        return Color4(r + in.r, g + in.g, b + in.b, a);
    }

    // -----------------------   Mat4    -----------------------

    inline float& Mat4::operator[](const size_t in)
    {
        assert(in < 16);
        return content[in];
    }

    inline const float& Mat4::operator[](const size_t in) const
    {
        assert(in < 16);
        return content[in];
    }

    inline float& Mat4::at(const unsigned char x, const unsigned char y)
    {
        assert(x < 4 && y < 4);
        return content[x*4+y];
    }

    inline float& Mat3::operator[](const size_t in)
    {
        assert(in < 9);
        return content[in];
    }

    inline const float& Mat3::operator[](const size_t in) const
    {
        assert(in < 9);
        return content[in];
    }

    inline float& Mat3::at(const unsigned char x, const unsigned char y)
    {
        assert(x < 3 && y < 3);
        return content[x * 3 + y];
    }

    // -----------------------   Quat    ------------------------

    inline Quat::Quat(const Mat3& in)
    {
        a = sqrtf(1 + in[0] + in[4] + in[8]) / 2.0f;
        v = Vec3(in[5] - in[7], in[6] - in[2], in[1] - in[3]) / (4*a);
    }

    inline Quat::Quat(const Mat4& in)
    {
        a = sqrtf(1 + in[0] + in[5] + in[10]) / 2.0f;
        v = Vec3(in[6] - in[9], in[8] - in[2], in[1] - in[4]) / (4 * a);
    }

    inline float Quat::lengthSquared() const
    {
        return a*a + v.lengthSquared();
    }

    inline float Quat::getLength() const
    {
        return sqrtf(lengthSquared());
    }

    inline Quat Maths::Quat::Conjugate() const
    {
        return Quat(-v, a);
    }

    inline Quat Maths::Quat::Inverse() const
    {
        float lq = lengthSquared();
        if (lq < 1e-5f) return *this;
        return Quat(v / lq, a / lq);
    }

    inline Quat Quat::AxisAngle(Vec3 axis, float angle)
    {
        float hAngle = angle / 2;
        return Quat(axis * sinf(hAngle), cosf(hAngle));
    }

    inline float Quat::GetAngle()
    {
        return 2 * acosf(a);
    }

    inline Vec3 Quat::GetAxis()
    {
        float factor = sqrtf(1-a*a);
        return v / factor;
    }

    inline Mat3 Quat::GetRotationMatrix3()
    {
        Mat3 result;
        float b = v.x;
        float c = v.y;
        float d = v.z;
        result.at(0, 0) = 2 * (a*a + b*b) - 1;
        result.at(1, 0) = 2 * (b*c - d*a);
        result.at(2, 0) = 2 * (b*d + c*a);
        result.at(0, 1) = 2 * (b*c + d*a);
        result.at(1, 1) = 2 * (a*a + c*c) - 1;
        result.at(2, 1) = 2 * (c*d - b*a);
        result.at(0, 2) = 2 * (b*d - c*a);
        result.at(1, 2) = 2 * (c*d + b*a);
        result.at(2, 2) = 2 * (a*a + d*d) - 1;
        return result;
    }

    inline Mat4 Quat::GetRotationMatrix4()
    {
        Mat4 result;
        float b = v.x;
        float c = v.y;
        float d = v.z;
        result.at(0, 0) = 2 * (a * a + b * b) - 1;
        result.at(1, 0) = 2 * (b * c - d * a);
        result.at(2, 0) = 2 * (b * d + c * a);
        result.at(0, 1) = 2 * (b * c + d * a);
        result.at(1, 1) = 2 * (a * a + c * c) - 1;
        result.at(2, 1) = 2 * (c * d - b * a);
        result.at(0, 2) = 2 * (b * d - c * a);
        result.at(1, 2) = 2 * (c * d + b * a);
        result.at(2, 2) = 2 * (a * a + d * d) - 1;
        return result;
    }

    inline Quat Quat::operator+(const Quat& other) const
    {
        return Quat(v + other.v, a + other.a);
    }

    inline Quat Quat::operator-(const Quat& other) const
    {
        return Quat(v - other.v, a - other.a);
    }

    inline Quat Quat::operator-() const
    {
        return Quat(-v, -a);
    }

    inline Quat Quat::operator*(const Quat& other) const
    {
        return Quat(other.v * a + v * other.a + v.crossProduct(other.v), a*other.a - v.dotProduct(other.v));
    }

    inline Vec3 Quat::operator*(const Vec3& other) const
    {
        Quat tmp = operator*(Quat(other, 0.0f)) * Inverse();
        return Vec3(tmp.v);
    }

    inline Quat Quat::operator*(const float scalar) const
    {
        return Quat(v * scalar, a * scalar);
    }

    inline Vec3 Quat::GetRight() const
    {
        return operator*(Vec3(1,0,0));
    }

    inline Vec3 Quat::GetUp() const
    {
        return operator*(Vec3(0, 1, 0));
    }

    inline Vec3 Quat::GetFront() const
    {
        return operator*(Vec3(0, 0, 1));
    }

    // ----------------------- Math Utils -----------------------

    inline float Util::toRadians(float in)
    {
        return in / 180.0f * (float)M_PI;
    }

    inline float Util::toDegrees(float in)
    {
        return in * 180.0f / (float)M_PI;
    }

    inline float Util::clamp(float in, float min, float max)
    {
        if (in < min)
            in = min;
        if (in > max)
            in = max;
        return in;
    }

    inline int Util::iclamp(int in, int min, int max)
    {
        if (in < min)
            in = min;
        if (in > max)
            in = max;
        return in;
    }

    inline float Util::lerp(float a, float b, float delta)
    {
        return a + delta * (b - a);
    }

    Quat Util::slerp(const Quat& a, Quat b, float alpha)
    {
        Quat result = Quat();
        float cosHalfTheta = a.a * b.a + a.v.x * b.v.x + a.v.y * b.v.y + a.v.z * b.v.z;
        if (cosHalfTheta < 0) {
            b = -b;
            cosHalfTheta = -cosHalfTheta;
        }
        if (fabsf(cosHalfTheta) >= 1.0f) {
            result = a;
            return result;
        }
        float halfTheta = acosf(cosHalfTheta);
        float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
        if (fabsf(sinHalfTheta) < 0.001f)
        {
            result = a * 0.5f + b * 0.5f;
            return result;
        }
        float ratioA = sinf((1 - alpha) * halfTheta) / sinHalfTheta;
        float ratioB = sinf(alpha * halfTheta) / sinHalfTheta;
        result = a * ratioA + b * ratioB;
        return result;
    }

    inline float Util::mod(float in, float value)
    {
        return in - value * floorf(in / value);
    }

    inline int Util::imod(int in, int value)
    {
        int tmp = in % value;
        if (tmp < 0) tmp += value;
        return tmp;
    }

    inline float Util::minF(float a, float b)
    {
        if (a > b)
            return b;
        return a;
    }

    inline float Util::maxF(float a, float b)
    {
        if (a > b)
            return a;
        return b;
    }

    inline int Util::minI(int a, int b)
    {
        if (a > b)
            return b;
        return a;
    }

    inline int Util::maxI(int a, int b)
    {
        if (a > b)
            return a;
        return b;
    }

    inline float Util::smin(float a, float b, float delta)
    {
        float half = clamp(0.5f + 0.5f * (a - b) / delta, 0.0f, 1.0f);
        return lerp(a, b, half) - delta * half * (1.0f - half);
    }

    inline bool Util::isEqual(float a, float b, float prec)
    {
        return (a - prec < b&& a + prec > b);
    }

    static const char* digits = "0123456789ABCDEF";

    inline std::string Util::GetHex(u64 number)
    {
        std::string result = std::string();
        result.reserve(16);
        for (u8 i = 0; i < 16; i++)
        {
            u8 digit = (number >> (i * 4)) & 0xf;
            result[i] = digits[digit];
        }
        return result;
    }

    inline void Util::GetHex(char* buffer, u64 number)
    {
        for (u8 i = 0; i < 16; i++)
        {
            u8 digit = (number >> (i * 4)) & 0xf;
            buffer[i] = digits[digit];
        }
    }
}