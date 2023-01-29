#include "Maths/Maths.hpp"

#include <cstdio>

namespace Maths
{
    // -----------------------   IVec2    -----------------------

    void IVec2::print() const
    {
        printf("(%d, %d)\n", x, y);
    }

    const std::string IVec2::toString() const
    {
        return "( " + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    // -----------------------   Vec2    -----------------------

    void Vec2::print() const
    {
        printf("(%.2f, %.2f)\n", x, y);
    }

    const std::string Vec2::toString() const
    {
        return "( " + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    // -----------------------   IVec3    -----------------------

    void IVec3::print() const
    {
        printf("(%d, %d, %d)\n", x, y, z);
    }

    const std::string IVec3::toString() const
    {
        return "( " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    // -----------------------   Vec3    -----------------------

    void Vec3::print() const
    {
        printf("(%.2f, %.2f, %.2f)\n", x, y, z);
    }

    const std::string Vec3::toString() const
    {
        return "( " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    // -----------------------   Vec4    -----------------------

    void Vec4::print() const
    {
        printf("(%.2f, %.2f, %.2f, %.2f)\n", x, y, z, w);
    }

    const std::string Vec4::toString() const
    {
        return "( " + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ", " + std::to_string(w) + ")";
    }

    // -----------------------   Mat4    -----------------------

    Mat4::Mat4(float diagonal)
    {
        for (size_t i = 0; i < 4; i++) content[i*5] = diagonal;
    }

    Mat4::Mat4(const Mat4& in)
    {
        for (size_t j = 0; j < 16; j++)
        {
            content[j] = in.content[j];
        }
    }

    Mat4::Mat4(const Mat3& in)
    {
        for (size_t j = 0; j < 9; j++)
        {
            content[j + (j/3)] = in.content[j];
        }
        content[15] = 1.0f;
    }

    Mat4::Mat4(const float* data)
    {
        for (size_t j = 0; j < 4; j++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                content[j*4+i] = data[j+i*4];
            }
        }
    }

    Mat4 Mat4::operator*(const Mat4& in) const
    {
        Mat4 out;
        for (size_t j = 0; j < 4; j++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                float res = 0;
                for (size_t k = 0; k < 4; k++)
                    res += content[j+k*4] * in.content[k+i*4];

                out.content[j+i*4] = res;
            }
        }
        return out;
    }

    Vec4 Mat4::operator*(const Vec4& in) const
    {
        Vec4 out;
        for (size_t i = 0; i < 4; i++)
        {
            float res = 0;
            for (size_t k = 0; k < 4; k++) res += content[i+k*4] * in[k];
            out[i] = res;
        }
        return out;
    }

    Mat4 Mat4::CreateXRotationMatrix(float angle)
    {
        Mat4 out = Mat4(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(1, 1) = cosA;
        out.at(2, 1) = -sinA;
        out.at(1, 2) = sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat4 Mat4::CreateYRotationMatrix(float angle)
    {
        Mat4 out = Mat4(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(2, 0) = sinA;
        out.at(0, 2) = -sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat4 Mat4::CreateZRotationMatrix(float angle)
    {
        Mat4 out = Mat4(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(1, 0) = -sinA;
        out.at(0, 1) = sinA;
        out.at(1, 1) = cosA;
        return out;
    }

    Mat4 Mat4::CreateScaleMatrix(const Vec3& scale)
    {
        Mat4 out;
        for (int i = 0; i < 3; i++) out.at(i, i) = scale[i];
        out.content[15] = 1;
        return out;
    }

    Mat4 Mat4::CreateTranslationMatrix(const Vec3& translation)
    {
        Mat4 out = Mat4(1);
        for (int i = 0; i < 3; i++) out.at(3, i) = translation[i];
        return out;
    }

    Mat4 Mat4::CreateTransformMatrix(const Vec3& position, const Vec3& rotation, const Vec3& scale)
    {
        return CreateTranslationMatrix(position) * CreateRotationMatrix(rotation) * CreateScaleMatrix(scale);
    }

    Mat4 Mat4::CreateTransformMatrix(const Vec3& position, const Vec3& rotation)
    {
        return CreateTranslationMatrix(position) * CreateRotationMatrix(rotation);
    }

    Mat4 Maths::Mat4::CreateRotationMatrix(Vec3 angles)
    {
        return CreateYRotationMatrix(angles.y) * CreateXRotationMatrix(angles.x) * CreateZRotationMatrix(angles.z);
    }

    Mat4 Mat4::CreatePerspectiveProjectionMatrix(float near, float far, float fov)
    {
        float s = 1.0f / tanf(Util::toRadians(fov / 2.0f));
        float param1 = -far / (far - near);
        float param2 = param1 * near;
        Mat4 out;
        out.at(0, 0) = s;
        out.at(1, 1) = s;
        out.at(2, 2) = param1;
        out.at(3, 2) = -1;
        out.at(2, 3) = param2;
        return out;
    }

    Mat4 Mat4::transposeMatrix()
    {
        float x[16] = { 0 };
        for (int j = 0; j < 4; j++)
        {
            for (int i = 0; i < 4; i++)
            {
                x[i + j * 4] = content[i+j*4];
            }
        }

        return Mat4{ x };
    }

    void Mat4::printMatrix(bool raw)
    {
        if (raw)
        {
            printf("[ ");
            for (int j = 0; j < 16; j++)
            {
                printf("%.2f",content[j]);
                if (j != 15) printf(", ");
            }
            printf("]\n");
        }
        else
        {
            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    printf("% 8.2f ", content[j + i * 4]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }

    const std::string Mat4::toString() const
    {
        std::string res = "( ";
        for (int i = 0; i < 16; i++)
        {
            res += std::to_string(content[(i * 4) % 16 + i / 4]);
            if (i != 15) res.append(", ");
        }
        res.append(")");
        return res;
    }

    Mat4 Mat4::Identity()
    {
        return Mat4(1);
    }

    Mat4 Mat4::getCofactor(int p, int q, int n) const
    {
        Mat4 mat;
        int i = 0, j = 0;
        // Looping for each element of the matrix
        for (int row = 0; row < n; row++)
        {
            for (int col = 0; col < n; col++)
            {
                //  Copying into temporary matrix only those element
                //  which are not in given row and column
                if (row != p && col != q)
                {
                    mat.content[i+j*4] = content[row+col*4];
                    j++;

                    // Row is filled, so increase row index and
                    // reset col index
                    if (j == n - 1)
                    {
                        j = 0;
                        i++;
                    }
                }
            }
        }
        return mat;
    }

    float Mat4::getDeterminant(float n) const
    {
        Mat4 a;
        float D = 0; // Initialize result

        //  Base case : if matrix contains single element
        if (n == 1)
            return content[0];

        char sign = 1;  // To store sign multiplier

         // Iterate for each element of first row
        for (int f = 0; f < n; f++)
        {
            // Getting Cofactor of matrix[0][f]
            a = getCofactor(0, f, (int)n);
            D += sign * content[f*4] * a.getDeterminant(n - 1);

            // terms are to be added with alternate sign
            sign = -sign;
        }

        return D;
    }

    Mat4 Mat4::CreateInverseMatrix() const
    {
        // Find determinant of matrix
        Mat4 inverse;
        float det = getDeterminant(4);
        if (det == 0)
        {
            printf("Singular matrix, can't find its inverse\n");
            return Mat4();
        }

        // Find adjoint
        Mat4 adj = CreateAdjMatrix();

        // Find Inverse using formula "inverse(A) = adj(A)/det(A)"
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                inverse.content[i+j*4] = adj.content[i+j*4] / det;

        return inverse;
    }

    Mat4 Mat4::CreateAdjMatrix() const
    {
        // temp is used to store cofactors of matrix
        Mat4 temp;
        Mat4 adj;
        char sign = 1;

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                // Get cofactor of matrix[i][j]
                temp = getCofactor(i, j, 4);

                // sign of adj positive if sum of row
                // and column indexes is even.
                sign = ((i + j) % 2 == 0) ? 1 : -1;

                // Interchanging rows and columns to get the
                // transpose of the cofactor matrix
                adj.content[j+i*4] = (sign) * (temp.getDeterminant(3));
            }
        }
        return adj;
    }

    Mat3::Mat3(float diagonal)
    {
        for (size_t i = 0; i < 3; i++) content[i * 4] = diagonal;
    }

    Mat3::Mat3(const Mat3& in)
    {
        for (size_t j = 0; j < 9; j++)
        {
            content[j] = in.content[j];
        }
    }

    Mat3::Mat3(const Mat4& in)
    {
        size_t index = 0;
        for (size_t j = 0; j < 11; j++)
        {
            if (j == 3 || j == 7) continue;
            content[index] = in.content[j];
            index++;
        }
    }

    Mat3::Mat3(const float* data)
    {
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t i = 0; i < 3; i++)
            {
                content[j * 3 + i] = data[j + i * 3];
            }
        }
    }

    Mat3 Mat3::operator*(const Mat3& in)
    {
        Mat3 out;
        for (size_t j = 0; j < 3; j++)
        {
            for (size_t i = 0; i < 3; i++)
            {
                float res = 0;
                for (size_t k = 0; k < 3; k++)
                    res += content[j + k * 3] * in.content[k + i * 3];

                out.content[j + i * 3] = res;
            }
        }
        return out;
    }

    Vec3 Mat3::operator*(const Vec3& in)
    {
        Vec3 out;
        for (size_t i = 0; i < 3; i++)
        {
            float res = 0;
            for (size_t k = 0; k < 3; k++) res += content[i + k * 3] * in[k];
            out[i] = res;
        }
        return out;
    }

    Mat3 Mat3::CreateXRotationMatrix(float angle)
    {
        Mat3 out = Mat3(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(1, 1) = cosA;
        out.at(2, 1) = -sinA;
        out.at(1, 2) = sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat3 Mat3::CreateYRotationMatrix(float angle)
    {
        Mat3 out = Mat3(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(2, 0) = sinA;
        out.at(0, 2) = -sinA;
        out.at(2, 2) = cosA;
        return out;
    }

    Mat3 Mat3::CreateZRotationMatrix(float angle)
    {
        Mat3 out = Mat3(1);
        float radA = Util::toRadians(angle);
        float cosA = cosf(radA);
        float sinA = sinf(radA);
        out.at(0, 0) = cosA;
        out.at(1, 0) = -sinA;
        out.at(0, 1) = sinA;
        out.at(1, 1) = cosA;
        return out;
    }

    Mat3 Mat3::CreateScaleMatrix(const Vec3& scale)
    {
        Mat3 out;
        for (int i = 0; i < 3; i++) out.at(i, i) = scale[i];
        return out;
    }

    Mat3 Mat3::transposeMatrix()
    {
        float x[9] = { 0 };
        for (int j = 0; j < 3; j++)
        {
            for (int i = 0; i < 3; i++)
            {
                x[i + j * 3] = content[i + j * 3];
            }
        }

        return Mat3{ x };
    }

    void Mat3::printMatrix(bool raw)
    {
        if (raw)
        {
            printf("[ ");
            for (int j = 0; j < 9; j++)
            {
                printf("%.2f", content[j]);
                if (j != 8) printf(", ");
            }
            printf("]\n");
        }
        else
        {
            for (int j = 0; j < 3; j++)
            {
                for (int i = 0; i < 3; i++)
                {
                    printf("% 8.2f ", content[j + i * 3]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }

    Mat3 Mat3::Identity()
    {
        return Mat3(1);
    }

    Mat3 Mat3::getCofactor(int p, int q, int n)
    {
        Mat3 mat;
        int i = 0, j = 0;
        // Looping for each element of the matrix
        for (int row = 0; row < n; row++)
        {
            for (int col = 0; col < n; col++)
            {
                //  Copying into temporary matrix only those element
                //  which are not in given row and column
                if (row != p && col != q)
                {
                    mat.content[i + j * 3] = content[row + col * 3];
                    j++;

                    // Row is filled, so increase row index and
                    // reset col index
                    if (j == n - 1)
                    {
                        j = 0;
                        i++;
                    }
                }
            }
        }
        return mat;
    }

    float Mat3::getDeterminant(float n)
    {
        Mat3 a;
        float D = 0; // Initialize result

        //  Base case : if matrix contains single element
        if (n == 1)
            return content[0];

        char sign = 1;  // To store sign multiplier

         // Iterate for each element of first row
        for (int f = 0; f < n; f++)
        {
            // Getting Cofactor of matrix[0][f]
            a = getCofactor(0, f, (int)n);
            D += sign * content[f * 3] * a.getDeterminant(n - 1);

            // terms are to be added with alternate sign
            sign = -sign;
        }

        return D;
    }

    Mat3 Mat3::CreateInverseMatrix()
    {
        // Find determinant of matrix
        Mat3 inverse;
        float det = getDeterminant(3);
        if (det == 0)
        {
            printf("Singular matrix, can't find its inverse\n");
            return 1;
        }

        // Find adjoint
        Mat3 adj = CreateAdjMatrix();

        // Find Inverse using formula "inverse(A) = adj(A)/det(A)"
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                inverse.content[i + j * 3] = adj.content[i + j * 3] / det;

        return inverse;
    }

    Mat3 Mat3::CreateAdjMatrix()
    {
        // temp is used to store cofactors of matrix
        Mat3 temp;
        Mat3 adj;
        char sign = 1;

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                // Get cofactor of matrix[i][j]
                temp = getCofactor(i, j, 3);

                // sign of adj positive if sum of row
                // and column indexes is even.
                sign = ((i + j) % 2 == 0) ? 1 : -1;

                // Interchanging rows and columns to get the
                // transpose of the cofactor matrix
                adj.content[j + i * 3] = (sign) * (temp.getDeterminant(2));
            }
        }
        return adj;
    }

    Vec3 Maths::Mat4::GetPositionFromTranslation() const
    {
        return Vec3(content[12], content[13], content[14]);
    }

    Vec3 Maths::Mat4::GetRotationFromTranslation(const Vec3& scale) const
    {
        /*
        * 00 | 04 | 08
        * 01 | 05 | 09
        * 02 | 06 | 10
        */
        float thetaX;
        float thetaY;
        float thetaZ;
        if (Util::minF(fabsf(scale.x), Util::minF(fabsf(scale.y), fabsf(scale.z))) < 0.0001f) return Vec3();
        float a = content[9] / scale.z;
        if (a < 0.9999f)
        {
            if (a > -0.9999f)
            {
                thetaX = asinf(-a);
                thetaY = atan2f(content[8] / scale.z, content[10] / scale.z);
                thetaZ = atan2f(content[1] / scale.x, content[5] / scale.y);
            }
            else
            {
                thetaX = (float)M_PI_2;
                thetaY = -atan2f(-content[4] / scale.y, content[0] / scale.x);
                thetaZ = 0;
            }
        }
        else
        {
            thetaX = -(float)M_PI_2;
            thetaY = atan2f(-content[4] / scale.y, content[0] / scale.x);
            thetaZ = 0;
        }
        return Vec3(Util::toDegrees(thetaX), Util::toDegrees(thetaY), Util::toDegrees(thetaZ));
    }

    Vec3 Maths::Mat4::GetScaleFromTranslation() const
    {
        Vec3 x = Vec3(content[0], content[1], content[2]);
        Vec3 y = Vec3(content[4], content[5], content[6]);
        Vec3 z = Vec3(content[8], content[9], content[10]);
        return Vec3(x.getLength(), y.getLength(), z.getLength());
    }

    void Maths::Util::GenerateSphere(int x, int y, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        float DtY = 180.0f / y;
        float DtX = 360.0f / x;
        for (int b = 1; b < y+1; b++)
        {
            for (int a = 0; a < x; a++)
            {
                if (b != y)
                {
                    PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(0, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(1, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(1, 0));
                }
                if (b == 1) continue;
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(0, 0));
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(0, 1));
                PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(1, 0));
            }
        }
    }

    void Maths::Util::GenerateCube(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        float sign = 1.0f;
        Vec3 V[4];
        Vec3 N;
        Vec2 UV[4];
        for (char i = 0; i < 6; i++)
        {
            if (i == 3) sign = -sign;
            float A = 1.0f;
            float B = 1.0f;
            for (char j = 0; j < 4; j++)
            {
                V[j][i%3] = sign/2;
                V[j][(i + 1 + (i < 3)) % 3] = A/2;
                V[j][(i + 2 - (i < 3)) % 3] = B/2;
                N[i % 3] = sign;
                N[(i + 1) % 3] = 0;
                N[(i + 2) % 3] = 0;
                UV[j][i % 2] = A < 0.0f ? 0.0f : 1.0f;
                UV[j][(i + 1) % 2] = B * sign < 0.0f ? 0.0f : 1.0f;
                A = -A;
                std::swap(A,B);
            }
            for (char j = 0; j < 2; j++)
            {
                PosOut->push_back(V[0]);
                PosOut->push_back(V[1+j]);
                PosOut->push_back(V[2+j]);
                for (char k = 0; k < 3; k++) NormOut->push_back(N);
                UVOut->push_back(UV[0]);
                UVOut->push_back(UV[1+j]);
                UVOut->push_back(UV[2+j]);
            }
        }
    }

    void Util::GenerateDome(int x, int y, bool reversed, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        float DtY = 180.0f / y;
        float DtX = 360.0f / x;
        for (int b = (reversed ? 1 : y/2 + 1); b < (reversed ? y / 2 : y) + 1; b++)
        {
            for (int a = 0; a < x; a++)
            {
                if (b != y)
                {
                    PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(0, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * b - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(1, 1));
                    PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                    NormOut->push_back(PosOut->back());
                    UVOut->push_back(Vec2(1, 0));
                }
                if (b == 1) continue;
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(0, 0));
                PosOut->push_back(GetSphericalCoord(DtX * a, DtY * b - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(0, 1));
                PosOut->push_back(GetSphericalCoord(DtX * (a + 1), DtY * (b - 1) - 90));
                NormOut->push_back(PosOut->back());
                UVOut->push_back(Vec2(1, 0));
            }
        }
    }

    void Util::GenerateCylinder(int x, int y, std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        float DtY = 2.0f / y;
        float DtX = 360.0f / x;
        for (int b = 0; b < y; b++)
        {
            for (int a = 0; a < x; a++)
            {
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() + Vec3(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2(0, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() +Vec3(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2(1, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() + Vec3(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2(1, 0));
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() +Vec3(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2(0, 0));
                NormOut->push_back(GetSphericalCoord(DtX * a, 0));
                PosOut->push_back(NormOut->back() + Vec3(0, DtY * (b + 1) - 1, 0));
                UVOut->push_back(Vec2(0, 1));
                NormOut->push_back(GetSphericalCoord(DtX * (a + 1), 0));
                PosOut->push_back(NormOut->back() + Vec3(0, DtY * b - 1, 0));
                UVOut->push_back(Vec2(1, 0));
            }
        }
    }

    void Util::GeneratePlane(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        PosOut->push_back(Vec3(-1, 1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(0, 0));
        PosOut->push_back(Vec3(-1, -1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(0, 1));
        PosOut->push_back(Vec3(1, 1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(1, 0));
        PosOut->push_back(Vec3(1, 1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(1, 0));
        PosOut->push_back(Vec3(-1, -1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(0, 1));
        PosOut->push_back(Vec3(1, -1, 0));
        NormOut->push_back(Vec3(0, 0, 1));
        UVOut->push_back(Vec2(1, 1));
    }

    void Util::GenerateSkyPlane(std::vector<Vec3>* PosOut, std::vector<Vec3>* NormOut, std::vector<Vec2>* UVOut)
    {
        PosOut->push_back(Vec3(-1, 1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(0, 0));

        PosOut->push_back(Vec3(1, -1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(1, 1));

        PosOut->push_back(Vec3(1, 1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(1, 0));

        PosOut->push_back(Vec3(-1, 1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(0, 0));

        PosOut->push_back(Vec3(-1, -1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(0, 1));

        PosOut->push_back(Vec3(1, -1, 1.0f));
        NormOut->push_back(Vec3(0, 0, -1));
        UVOut->push_back(Vec2(1, 1));
    }

    Vec3 Maths::Util::GetSphericalCoord(float longitude, float latitude)
    {
        longitude = toRadians(longitude);
        latitude = toRadians(latitude);
        return Vec3(cosf(longitude)*cosf(latitude),sinf(latitude), sinf(longitude) * cosf(latitude));
    }
}