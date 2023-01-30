#pragma once

#include <math.h>
#include <vector>
#include <string>

#include "Core/Types.hpp"

namespace Maths
{

    static const float VEC_COLLINEAR_PRECISION = 0.001f;
    static const float VEC_HIGH_VALUE = 1e38f;

    class Vec2;

    class IVec2
    {
    public:
        int x, y;
        inline IVec2() : x(0), y(0) {}
        inline IVec2(const IVec2& in) : x(in.x), y(in.y) {}
        inline IVec2(const Vec2& in);
        inline IVec2(const int& a, const int& b) : x(a), y(b) {}
		
		void print() const;
        const std::string toString() const;

        inline IVec2 operator+(const IVec2& a) const;

        // Return a new vector wich is the substraction of 'a' and 'b'
        inline IVec2 operator-(const IVec2& a) const;
		
		inline IVec2 operator-() const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline IVec2 operator*(const IVec2& a) const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline IVec2 operator*(const float& a) const;

        // Return the result of the aritmetic division of 'a' and 'b', will return 'a' * VEC2D_HIGH_VALUE if 'b' is equal 0;
        inline IVec2 operator/(const float& b) const;

        inline bool operator==(const IVec2& b) const;
    };

    class Vec2
    {
    public:
        float x;
        float y;

        // Return a new empty Vec2
        inline Vec2() : x(0), y(0) {}

        // Return a new Vec2 initialised with 'a' and 'b'
        inline Vec2(float a, float b) : x(a), y(b) {}

        // Return a new Vec2 equivalent to Vec(1,0) rotated by angle (in radians)
        inline Vec2(float angle) : Vec2(cosf(angle), sinf(angle)) {}

        // Return a new Vec2 initialised with 'in'
        inline Vec2(const Vec2& in) : x(in.x), y(in.y) {}
        inline Vec2(const IVec2& in) : x((float)in.x), y((float)in.y) {}
		
		void print() const;
        const std::string toString() const;

        // Return the length squared of 'in'
        inline float lengthSquared() const;

        //Return the distance between 'in' an 'a'
        inline float getDistanceFromPoint(Vec2 a) const;

        // Return the lenght of the given Vector
        inline float getLength() const;

        // Return a new vector wich is the sum of 'a' and 'b'
        inline Vec2 operator+(const Vec2& a) const;

        // Return a new vector wich is the substraction of 'a' and 'b'
        inline Vec2 operator-(const Vec2& a) const;

        inline Vec2 operator-() const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline Vec2 operator*(const Vec2& a) const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline Vec2 operator*(const float& a) const;

        // Return the result of the aritmetic division of 'a' and 'b', will return 'a' * VEC2D_HIGH_VALUE if 'b' is equal 0;
        inline Vec2 operator/(const float& b) const;

        inline Vec2 operator/(const Vec2& b) const;

        inline bool operator==(const Vec2& b) const;

        inline const float& operator[](const size_t a) const;

        inline float& operator[](const size_t a);

        // Return tue if 'a' and 'b' are collinears (Precision defined by VEC2D_COLLINEAR_PRECISION)
        inline bool isCollinearWith(Vec2 a) const;

        // Return the dot product of 'a' and 'b'
        inline float dotProduct(Vec2 a) const;

        // Return the z component of the cross product of 'a' and 'b'
        inline float crossProduct(Vec2 a) const;

        // Return a vector with the same direction that 'in', but with length 1
        inline Vec2 unitVector() const;

        // Return a vector of length 'in' and with an opposite direction
        inline Vec2 negate() const;

        // Return the normal vector of 'in'
        inline Vec2 getNormal() const;

        // return true if 'a' converted to int is equivalent to 'in' converted to int
        inline bool isIntEquivalent(Vec2 a) const;

        // Get the angle defined by this vector, in radians
        inline float GetAngle() const;
		
		inline bool isNearlyEqual(Vec2 a, float prec = 1e-5f);

    };

    class Vec3;

    class IVec3
    {
    public:
        int x, y, z;
        inline IVec3() : x(0), y(0), z(0) {}
        inline IVec3(const IVec3& in) : x(in.x), y(in.y), z(in.z) {}
        inline IVec3(const Vec3& in);
        inline IVec3(const int& a, const int& b, const int& c) : x(a), y(b), z(c) {}

        void print() const;
        const std::string toString() const;

        inline IVec3 operator+(const IVec3& a) const;

        // Return a new vector wich is the substraction of 'a' and 'b'
        inline IVec3 operator-(const IVec3& a) const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline IVec3 operator*(const IVec3& a) const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline IVec3 operator*(const float& a) const;

        // Return the result of the aritmetic division of 'a' and 'b', will return 'a' * VEC2D_HIGH_VALUE if 'b' is equal 0;
        inline IVec3 operator/(const float& b) const;

        inline bool operator==(const IVec3& b) const;
		
		inline const int& operator[](const size_t a) const;

        inline int& operator[](const size_t a);
    };

    class Vec3
    {
    public:
        float x;
        float y;
        float z;

        // Return a new empty Vec3
        inline Vec3() : x(0), y(0), z(0) {}

        inline Vec3(float content) : x(content), y(content), z(content) {}

        // Return a new Vec3 initialised with 'a' and 'b'
        inline Vec3(float a, float b, float c) : x(a), y(b), z(c) {}

        // Return a new Vec3 initialised with 'in'
        inline Vec3(const Vec3& in) : x(in.x), y(in.y), z(in.z) {}

        inline Vec3(const IVec3& in) : x((float)in.x), y((float)in.y), z((float)in.z) {}

        // Print the Vec3
        void print() const;
		const std::string toString() const;

        // Return the length squared of 'in'
        inline float lengthSquared() const;

        // Return the lenght of the given Vector
        inline float getLength() const;

        // Return a new vector wich is the sum of 'a' and 'b'
        inline Vec3 operator+(const Vec3& a) const;

        // Return a new vector wich is the substraction of 'a' and 'b'
        inline Vec3 operator-(const Vec3& a) const;

        inline Vec3 operator-() const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline Vec3 operator*(const Vec3& a) const;

        // Return the result of the aritmetic multiplication of 'a' and 'b'
        inline Vec3 operator*(const float& a) const;

        // Return the result of the aritmetic division of 'a' and 'b', will return 'a' * VEC_HIGH_VALUE if 'b' is equal 0;
        inline Vec3 operator/(const float& b) const;

        inline bool operator==(const Vec3& b) const;

        inline const float& operator[](const size_t a) const;

        inline float& operator[](const size_t a);

        // Return tue if 'a' and 'b' are collinears (Precision defined by VEC_COLLINEAR_PRECISION)
        inline bool isCollinearWith(Vec3 a) const;

        // Return the dot product of 'a' and 'b'
        inline float dotProduct(Vec3 a) const;

        // Return the z component of the cross product of 'a' and 'b'
        inline Vec3 crossProduct(Vec3 a) const;

        // Return a vector with the same direction that 'in', but with length 1
        inline Vec3 unitVector() const;

        // Return a vector of length 'in' and with an opposite direction
        inline Vec3 negate() const;

        // return true if 'a' converted to int is equivalent to 'in' converted to int
        inline bool isIntEquivalent(Vec3 a) const;

        inline bool isNearlyEqual(Vec3 a, float prec = 1e-5f);
    };

    class Vec4;

    class Color4
    {
    public:
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;

        inline Color4() : r(0), g(0), b(0), a(0) {}
        inline Color4(const float* in);
        inline Color4(const Vec4& in);
        inline Color4(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0xff) : r(red), g(green), b(blue), a(alpha) {}
        inline Color4(unsigned int rgba) : r((rgba & 0xff000000) >> 24), g((rgba & 0x00ff0000) >> 16), b((rgba & 0x0000ff00) >> 8), a(rgba & 0x000000ff) {}

        inline Color4 operator*(const float& a) const;
        inline Color4 operator+(const Color4& a) const;
    };

    class Vec4
    {
    public:
        float x;
        float y;
        float z;
        float w;

        // Return a new empty Vec4
        inline Vec4() : x(0), y(0), z(0), w(0) {}

        // Return a new Vec4 initialised with 'a', 'b', 'c' and 'd'
        inline Vec4(float a, float b, float c, float d = 1) : x(a), y(b), z(c), w(d) {}

        // Return a new Vec4 initialised with 'in'
        inline Vec4(const Vec3& in, float wIn = 1.0f) : x(in.x), y(in.y), z(in.z), w(wIn) {}

        // Return a new Vec4 initialised with 'in'
        inline Vec4(const Vec4& in) : x(in.x), y(in.y), z(in.z), w(in.w) {}

        inline Vec4(const Color4& in) : x(in.r / 255.0f), y(in.g / 255.0f), z(in.b / 255.0f), w(in.a / 255.0f) {}


        // Print the Vec4
        void print() const;
		const std::string toString() const;

        // Return the Vec3 of Vec4
        inline Vec3 getVector() const;

        // Return the length squared
        inline float lengthSquared() const;

        // Return the length
        inline float getLength() const;

        // Divide each components by w, or set to VEC_HIGH_VALUE if w equals 0
        inline Vec4 homogenize() const;

        inline Vec4 operator+(const Vec4& a) const;

        inline Vec4 operator-(const Vec4& a) const;

        inline Vec4 operator-() const;

        inline Vec4 operator*(const Vec4& a) const;

        inline Vec4 operator*(const float& a) const;

        inline Vec4 operator/(const float& b) const;

        inline bool operator==(const Vec4& b) const;

        inline float& operator[](const size_t a);
        inline const float& operator[](const size_t a) const;

        // Return tue if 'a' and 'b' are collinears (Precision defined by VEC_COLLINEAR_PRECISION)
        inline bool isCollinearWith(Vec4 a) const;

        inline float dotProduct(Vec4 a) const;

        // Return the z component of the cross product of 'a' and 'b'
        inline Vec4 crossProduct(Vec4 a) const;

        // Return a vector with the same direction that 'in', but with length 1
        inline Vec4 unitVector() const;

        // Return a vector of length 'in' and with an opposite direction
        inline Vec4 negate() const;

        // return true if 'a' converted to int is equivalent to 'in' converted to int
        inline bool isIntEquivalent(Vec4 a) const;
		
		inline bool isNearlyEqual(Vec4 a, float prec = 1e-5f);
    };

    class Mat3;

    class Mat4
    {
    public:
        /* data of the matrix : content[y][x]
         * Matrix is indexed with:
         *
         * 00 | 04 | 08 | 12
         * 01 | 05 | 09 | 13
         * 02 | 06 | 10 | 14
         * 03 | 07 | 11 | 15
         *
        */
        float content[16] = { 0 };

        Mat4() {}

        Mat4(float diagonal);

        Mat4(const Mat4& in);

        Mat4(const Mat3& in);

        Mat4(const float* data);

        Mat4 operator*(const Mat4& a) const;

        Vec4 operator*(const Vec4& a) const;

        static Mat4 Identity();

        static Mat4 CreateTransformMatrix(const Vec3& position, const Vec3& rotation, const Vec3& scale);

        static Mat4 CreateTransformMatrix(const Vec3& position, const Vec3& rotation);

        static Mat4 CreateTranslationMatrix(const Vec3& translation);

        static Mat4 CreateScaleMatrix(const Vec3& scale);

        static Mat4 CreateRotationMatrix(Vec3 angles);

        static Mat4 CreateXRotationMatrix(float angle);

        static Mat4 CreateYRotationMatrix(float angle);

        static Mat4 CreateZRotationMatrix(float angle);

        static Mat4 CreatePerspectiveProjectionMatrix(float near, float far, float fov);

        Vec3 GetPositionFromTranslation() const;

        Vec3 GetRotationFromTranslation(const Vec3& scale) const;

        Vec3 GetScaleFromTranslation() const;

        Mat4 transposeMatrix();

        inline float& operator[](const size_t a);
		
		inline const float& operator[](const size_t a) const;

        inline float& at(const unsigned char x, const unsigned char y);

        void printMatrix(bool raw = false);
		const std::string toString() const;

        Mat4 CreateInverseMatrix() const;

        Mat4 CreateAdjMatrix() const;

        Mat4 getCofactor(int p, int q, int n) const;

        // Recursive function for finding determinant of matrix. n is current dimension of 'in'.
        float getDeterminant(float n) const;
    };

    class Mat3
    {
    public:
        /* data of the matrix : content[y][x]
         * Matrix is indexed with:
         *
         * 00 | 03 | 06
         * 01 | 04 | 07
         * 02 | 05 | 08
         *
        */
        float content[9] = { 0 };

        Mat3() {}

        Mat3(float diagonal);

        Mat3(const Mat3& in);

        Mat3(const Mat4& in);

        Mat3(const float* data);

        Mat3 operator*(const Mat3& a);

        Vec3 operator*(const Vec3& a);

        static Mat3 Identity();

        static Mat3 CreateScaleMatrix(const Vec3& scale);

        static Mat3 CreateXRotationMatrix(float angle);

        static Mat3 CreateYRotationMatrix(float angle);

        static Mat3 CreateZRotationMatrix(float angle);

        Mat3 transposeMatrix();

        inline float& operator[](const size_t a);

        inline const float& operator[](const size_t a) const;

        inline float& at(const unsigned char x, const unsigned char y);

        void printMatrix(bool raw = false);

        Mat3 CreateInverseMatrix();

        Mat3 CreateAdjMatrix();

        Mat3 getCofactor(int p, int q, int n);

        // Recursive function for finding determinant of matrix. n is current dimension of 'in'.
        float getDeterminant(float n);
    };
    
    class Quat
    {
    public:
        Vec3 v;
        float a;

        inline Quat() : v(), a(1) {}

        inline Quat(Vec3 vector, float real) : v(vector), a(real) {}

        inline Quat(const Mat3& in);

        inline Quat(const Mat4& in);

        // Return the length squared
        inline float lengthSquared() const;

        // Return the length
        inline float getLength() const;

        inline Quat Conjugate() const;

        inline Quat Inverse() const;

        // Makes a quaternion representing a rotation in 3d space. Angle is in radians.
        static Quat AxisAngle(Vec3 axis, float angle);

        inline float GetAngle();

        inline Vec3 GetAxis();

        inline Mat3 GetRotationMatrix3();

        inline Mat4 GetRotationMatrix4();

        inline Quat operator+(const Quat& other) const;

        inline Quat operator-(const Quat& other) const;

        inline Quat operator-() const;

        inline Quat operator*(const Quat& other) const;

        inline Vec3 operator*(const Vec3& other) const;

        inline Quat operator*(const float scalar) const;

        inline Vec3 GetRight() const;

        inline Vec3 GetUp() const;

        inline Vec3 GetFront() const;
    };

    namespace Util
    {
        // Return the given angular value in degrees converted to radians
        inline float toRadians(float in);

        // Return the given angular value in radians converted to degrees
        inline float toDegrees(float in);

        inline float clamp(float in, float min = 0.0f, float max = 1.0f);

        inline int iclamp(int in, int min, int max);

        inline float lerp(float a, float b, float delta);

        inline Quat slerp(const Quat& a, Quat b, float alpha);

        inline float mod(float in, float value);

        inline int imod(int in, int value);

        inline float minF(float a, float b);

        inline float maxF(float a, float b);

        inline int minI(int a, int b);

        inline int maxI(int a, int b);

        inline float smin(float a, float b, float delta);

        inline bool isEqual(float a, float b, float prec);

        // Returns a string with the hex representation of number
        inline std::string GetHex(u64 number);

        // Fills the given buffer with the hex representation of number
        // WARNING: buffer must be at least 16 char long
        inline void GetHex(char* buffer, u64 number);

        void GenerateSphere(int x, int y, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        void GenerateCube(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        void GenerateDome(int x, int y, bool reversed, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        void GenerateCylinder(int x, int y, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        void GeneratePlane(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        void GenerateSkyPlane(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut);

        Vec3 GetSphericalCoord(float longitude, float latitude);
    };
}

#include "maths.inl"