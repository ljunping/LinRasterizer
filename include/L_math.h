//
// Created by Lin on 2024/11/12.
//

#ifndef L_MATH_H
#define L_MATH_H
#pragma once
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include "debug.h"

//MACRO DEFINE
#define PI 3.1415926535897932384626433832795
#define PI_OVER_2 1.5707963267948966192313216916398
#define PI_OVER_4 0.78539816339744830961566084581988
#define Vec4 L_MATH::Vec<float,4>
#define Vec3 L_MATH::Vec<float,3>
#define Vec2 L_MATH::Vec<float,2>


#define IVec3 L_MATH::Vec<int,3>

#define Mat44 L_MATH::Mat<float,4,4>
#define Mat33 L_MATH::Mat<float,3,3>
#define Mat22 L_MATH::Mat<float,2,2>

#define DEFINE_VAR_OPERATE_FLOAT_POINT(p1,p2,count,var,operate)\
float * var[count];\
for(int i=0;i<count;i++){\
    var[i]=p1[i] operate p2[i];\
}

#define OPERATE_FLOAT_POINT(p1,p2,count,operate)\
for(int i=0;i<count;i++){\
    p1[i]=p1[i] operate p2[i];\
}

#define EPSILON 1.0e-6
#define COMMA ,
#define SPACE
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define FIRST_ARG(arg1, ...) arg1
#define SECOND_ARG(arg1,arg2,...) arg2
#define CONCATENATE_2(a, b, delimiter) a delimiter b
#define CONCATENATE_3(a, b, c, delimiter) a delimiter b delimiter c
#define CONCATENATE_4(a, b, c, d, delimiter) a delimiter b delimiter c delimiter d
#define CONCATENATE(delimiter, ...) GET_MACRO(__VA_ARGS__, CONCATENATE_4, CONCATENATE_3, CONCATENATE_2)(__VA_ARGS__, delimiter)

#define INT_HIGHEST_BIT_INDEX(n) __builtin_ctz(n)
#define LONG_HIGHEST_BIT_INDEX(n) __builtin_ctzl(n)
#define LONG_LONG_HIGHEST_BIT_INDEX(n) __builtin_ctzll(n)
//从1开始计数
#define FIRST_1_BIT_COUNT(n) __builtin_ffs(n)


//临时宏,退出文件去除
#define FUNC_PARAM_DECLARE_ERROR(...)
#define FUNC_PARAM_DECLARE_2(a, b)  ,a b
#define FUNC_PARAM_DECLARE_4(a, b, c, d) ,a b,c d
#define FUNC_PARAM_DECLARE(...) GET_MACRO(__VA_ARGS__, FUNC_PARAM_DECLARE_4,FUNC_PARAM_DECLARE_ERROR, FUNC_PARAM_DECLARE_2,FUNC_PARAM_DECLARE_ERROR,FUNC_PARAM_DECLARE_ERROR)(__VA_ARGS__)
#define FUNC_PARAM_USING_ERROR(...)
#define FUNC_PARAM_USING_2(a, b) ,b
#define FUNC_PARAM_USING_4(a ,b, c, d) ,b,d
#define FUNC_PARAM_USING(...) GET_MACRO(__VA_ARGS__, FUNC_PARAM_USING_4,FUNC_PARAM_USING_ERROR, FUNC_PARAM_USING_2,FUNC_PARAM_USING_ERROR,FUNC_PARAM_USING_ERROR)(__VA_ARGS__)
#define DECLARE_MAT_OPT(OPT,ROW,COL) \
    Mat<T, ROW, COL> operator OPT(const Mat<T, ROW, COL>& other) const;\
    void operator OPT##=(const Mat<T, ROW, COL>& other);
#define DECLARE_MAT_OPT_SINGLE(OPT,ROW,COL) \
    Mat<T, ROW, COL> operator OPT(const T& other) const;\
    void operator OPT##=(const T& other);
#define DECLARE_VEC_OPT(OPT,COL) \
    Vec<T, COL> operator OPT(const Vec<T, COL>& other) const;\
    Vec<T, COL> operator OPT(const T& other) const;\
    void operator OPT##=(const T& other);\
    void operator OPT##=(const Vec<T, COL>& other);
#define DEFINE_MAT_OPT(OPT) \
    template <typename T, int ROW, int COL>\
    Mat<T, ROW, COL> Mat<T, ROW, COL>::operator OPT(const Mat<T, ROW, COL>& other) const\
    {\
        Mat<T, ROW, COL> result;\
        for (int i = 0; i < ROW; ++i)\
        {\
            for (int j = 0; j < COL; ++j)\
            {\
                result[i][j]=data[i][j] OPT other[i][j];\
            }\
        }\
        return result;\
    }\
    template <typename T, int ROW, int COL>\
    void Mat<T, ROW, COL>::operator OPT##=(const Mat<T, ROW, COL>& other)\
    {\
        for (int i = 0; i < ROW; ++i)\
        {\
            for (int j = 0; j < COL; ++j)\
            {\
                data[i][j] OPT##= other[i][j];\
            }\
        }\
    }
#define DEFINE_MAT_OPT_SINGLE(OPT) \
    template <typename T, int ROW, int COL>\
    Mat<T, ROW, COL> Mat<T, ROW, COL>::operator OPT(const T& other) const\
    {\
        Mat<T, ROW, COL> result;\
        for (int i = 0; i < ROW; ++i)\
        {\
            for (int j = 0; j < COL; ++j)\
            {\
                result[i][j]=data[i][j] OPT other;\
            }\
        }\
        return result;\
    }\
    template <typename T, int ROW, int COL>\
    void Mat<T, ROW, COL>::operator OPT##=(const T& other)\
    {\
        for (int i = 0; i < ROW; ++i)\
        {\
            for (int j = 0; j < COL; ++j)\
            {\
                data[i][j] OPT##= other;\
            }\
        }\
    }
#define DEFINE_VEC_OPT(OPT) \
    template <typename T, int COL>\
    Vec<T, COL> Vec<T, COL>::operator OPT(const Vec<T, COL>& other) const\
    {\
        Vec<T, COL> result;\
        if constexpr (COL == 2)\
        {\
            result.data[0] = data[0] OPT other.data[0];\
            result.data[1] = data[1] OPT other.data[1];\
        }\
        if constexpr (COL == 3)\
        {\
            result.data[0] = data[0] OPT other.data[0];\
            result.data[1] = data[1] OPT other.data[1];\
            result.data[2] = data[2] OPT other.data[2];\
        }\
        else\
        {\
            for (int i = 0; i < COL; ++i)\
            {\
                result.data[i] = data[i] OPT other.data[i];\
            }\
        }\
        return result;\
    }\
    template <typename T, int COL>\
    Vec<T, COL> Vec<T, COL>::operator OPT(const T& other) const\
    {\
        Vec<T, COL> result;\
        if constexpr (COL == 2)\
        {\
            result.data[0] = data[0] OPT other;\
            result.data[1] = data[1] OPT other;\
        }\
        if constexpr (COL == 3)\
        {\
            result.data[0] = data[0] OPT other;\
            result.data[1] = data[1] OPT other;\
            result.data[2] = data[2] OPT other;\
        }\
        else\
        {\
            for (int i = 0; i < COL; ++i)\
            {\
                result.data[i] = data[i] OPT other;\
            }\
        }\
        return result;\
    }\
    template <typename T, int COL>\
    void Vec<T, COL>::operator OPT##=(const T& other)\
    {\
        if constexpr (COL == 2)\
        {\
            data[0] OPT##= other;\
            data[1] OPT##= other;\
            return;\
        }\
        if constexpr (COL == 3)\
        {\
            data[0] OPT##= other;\
            data[1] OPT##= other;\
            data[2] OPT##= other;\
            return;\
        }\
        for (int i = 0; i < COL; ++i)\
        {\
            data[i] OPT##= other;\
        }\
    }\
    template <typename T, int COL>\
    void Vec<T, COL>::operator OPT##=(const Vec<T, COL>& other)\
    {\
        if constexpr (COL == 2)\
        {\
            data[0] OPT##= other[0];\
            data[1] OPT##= other[1];\
            return;\
        }\
        if constexpr (COL == 3)\
        {\
            data[0] OPT##= other[0];\
            data[1] OPT##= other[1];\
            data[2] OPT##= other[2];\
            return;\
        }\
        for (int i = 0; i < COL; ++i)\
        {\
            data[i] OPT##= other[i];\
        }\
    }
#define DEFINE_VEC_FUNCTOR_2(FUNCTOR)\
    template <typename T, int Col>\
    Vec<T, Col> FUNCTOR(const Vec<T, Col>& vec1,const Vec<T, Col>& vec2)\
    {\
        Vec<T, Col> res;\
        if constexpr (Col == 2)\
        {\
            res[0]= ::FUNCTOR(vec1[0],vec2[0]);\
            res[1]= ::FUNCTOR(vec1[1],vec2[1]);\
            return res;\
        }\
        if constexpr (Col == 3)\
        {\
            res[0]= ::FUNCTOR(vec1[0],vec2[0]);\
            res[1]= ::FUNCTOR(vec1[1],vec2[1]);\
            res[2]= ::FUNCTOR(vec1[2],vec2[2]);\
            return res;\
        }\
        for (int j = 0; j < Col; ++j)\
        {\
            res[j] = ::FUNCTOR(vec1[j],vec2[j]);\
        }\
        return res;\
    }
#define DEFINE_VEC_FUNCTOR_BASE_PARAM(FUNCTOR,...)\
    template <typename T, int Col>\
    Vec<T, Col> FUNCTOR(const Vec<T, Col>& vec1 FUNC_PARAM_DECLARE(__VA_ARGS__))\
    {\
        Vec<T, Col> res;\
        if constexpr (Col == 2)\
        {\
            res[0]= ::FUNCTOR(vec1[0] FUNC_PARAM_USING(__VA_ARGS__));\
            res[1]= ::FUNCTOR(vec1[1] FUNC_PARAM_USING(__VA_ARGS__));\
            return res;\
        }\
        if constexpr (Col == 3)\
        {\
            res[0]= ::FUNCTOR(vec1[0] FUNC_PARAM_USING(__VA_ARGS__));\
            res[1]= ::FUNCTOR(vec1[1] FUNC_PARAM_USING(__VA_ARGS__));\
            res[2]= ::FUNCTOR(vec1[2] FUNC_PARAM_USING(__VA_ARGS__));\
            return res;\
        }\
        for (int j = 0; j < Col; ++j)\
        {\
            res[j] = ::FUNCTOR(vec1[j] FUNC_PARAM_USING(__VA_ARGS__));\
        }\
        return res;\
    }


#define DEFINE_MAT_FUNCTOR_2(FUNCTOR,...)\
    template <typename T, int Row, int Col>\
    Mat<T, Row, Col> FUNCTOR(const Mat<T, Row, Col>& mat1,const Mat<T, Row, Col>& mat2)\
    {\
        Mat<T, Row, Col> result;\
        for (int i = 0; i < Row; ++i)\
        {\
            for (int j = 0; j < Col; ++j)\
            {\
                result[i][j] = ::FUNCTOR(mat1[i][j],mat2[i][j]);\
            }\
        }\
        return result;\
    }

#define DEFINE_MAT_FUNCTOR_BASE_PARAM(FUNCTOR,...)\
    template <typename T, int Row, int Col>\
    Mat<T, Row, Col> FUNCTOR(const Mat<T, Row, Col>& mat1 FUNC_PARAM_DECLARE(__VA_ARGS__))\
    {\
        Mat<T, Row, Col> result;\
        for (int i = 0; i < Row; ++i)\
        {\
            for (int j = 0; j < Col; ++j)\
            {\
                result[i][j] = ::FUNCTOR(mat1[i][j] FUNC_PARAM_USING(__VA_ARGS__));\
            }\
        }\
        return result;\
    }


namespace L_MATH
{
    template <typename T, int Col>
    class Vec;
    extern Vec<float, 3> FORWARD;
    extern Vec<float, 3> UP;
    extern Vec<float, 3> LEFT;

    template <typename T, int Row, int Col>
    class Mat;

    template <typename T, int Col>
    class Vec
    {
    private:
    public:
        T data[Col];

        Vec(): data{}
        {
        };

        // Vec(const Vec<T, Col>& other) = default;
        //
        // Vec<T, Col>& operator =(const Vec<T, Col>& other) = default;
        //
        // Vec<T, Col>& operator =(const Vec<T, Col>&& other)
        // {
        //     for (int i = 0; i < Col; ++i)
        //     {
        //         data[i] = other.data[i];
        //     }
        //     return *this;
        // }
        //
        // Vec(Vec<T, Col>&& other) noexcept
        // {
        //     for (int i = 0; i < Col; ++i)
        //     {
        //         data[i] = other.data[i];
        //     }
        // }

        explicit Vec(T flatv)
        {
            if constexpr (Col == 3)
            {
                data[0] = flatv;
                data[1] = flatv;
                data[2] = flatv;
                return;
            }
            for (int i = 0; i < Col; ++i)
            {
                data[i] = flatv;
            }
        }


        explicit Vec(std::initializer_list<T> list)
        {
            if (list.size() != Col) return;
            if constexpr (Col == 3)
            {
                data[0] = list.begin()[0];
                data[1] = list.begin()[1];
                data[2] = list.begin()[2];
                return;
            }
            for (int i = 0; i < Col; ++i)
            {
                data[i] = list.begin()[i];
            }
        }

        explicit Vec(Vec<T, Col - 1> vec1, T t)
        {
            for (int i = 0; i < Col - 1; ++i)
            {
                data[i] = vec1[i];
            }
            data[Col - 1] = t;
        }

        template <int C>
        explicit Vec(const Vec<T, C>& src)
        {
            int i = 0;
            for (i = 0; i < Col && i < C; ++i)
            {
                data[i] = src[i];
            }
            for (; i < Col; ++i)
            {
                data[i] = 0;
            }
        }

        explicit Vec(const Mat<T, Col, 1>& mat);
        const static Vec<T, Col> ZERO;
        const static Vec<T, Col> ONE;

        const T& operator[](int index) const
        {
            return data[index];
        }

        T& operator[](int index)
        {
            return data[index];
        }

        template <int C>
        Mat<T, 1, C> operator*(const Mat<T, Col, C>& other) const
        {
            Mat<T, 1, C> result;
            for (int i = 0; i < C; ++i)
            {
                for (int j = 0; j < Col; ++j)
                {
                    result[0][i] += data[j] * other[j][i];
                }
            }
            return result;
        }

        T dot(const Vec<T, Col>& other) const;
        T magnitude() const;
        T sqrt_magnitude() const;

        Vec<T, Col> normalize() const;

        DECLARE_VEC_OPT(+, Col)
        DECLARE_VEC_OPT(-, Col)
        DECLARE_VEC_OPT(*, Col)
        DECLARE_VEC_OPT(/, Col)
    };

    template <typename T, int Col>
    class Vec;

    template <typename T, int Row, int Col>
    class Mat
    {
        using Mat_T_Row_Col = Mat<T, Row, Col>;
        Vec<T, Row> data[Col];

        void clear(T t)
        {
            for (int i = 0; i < Col; ++i)
            {
                for (int j = 0; j < Row; j++)
                {
                    data[i][j] = t;
                }
            }
        }

    public:
        Mat()
        {
            clear(0);
        }

        explicit Mat(T t)
        {
            clear(t);
        }
        explicit Mat(Vec<T,Row> diagV)
        {
            clear(0);
            for (int i = 0; i < Row; ++i)
            {
                data[i][i] = diagV[i];
            }
        }
        explicit Mat(std::initializer_list<Vec<T, Row>> list)
        {
            if (list.size() != Row)
            {
                return;
            }
            //先填充列
            for (int i = 0; i < Col; ++i)
            {
                data[i] = list.begin()[i];
            }
        }

        explicit Mat(std::initializer_list<T> list)
        {
            if (list.size() != Col * Row)
            {
                return;
            }
            for (int i = 0; i < Col; ++i)
            {
                for (int j = 0; j < Row; j++)
                {
                    data[i][j] = list.begin()[i * Row + j];
                }
            }
        }

        template <int R, int C>
        void copy_from(int si, int sj, const Mat<T, R, C>& src)
        {
            for (int i = 0; i < C && i + si < Col; ++i)
            {
                for (int j = 0; j < R && j + sj < Row; j++)
                {
                    data[i + si][j + sj] = src[i][j];
                }
            }
        }

        template <int R, int C>
        void copy_to(int si, int sj, Mat<T, R, C>& dst) const
        {
            for (int i = 0; i < C && i + si < Col; ++i)
            {
                for (int j = 0; j < R && j + sj < Row; j++)
                {
                    dst[i][j] = data[i + si][j + sj];
                }
            }
        }


        static Mat<T, Row, Col> dialog()
        {
            Mat<T, Row, Col> result;
            for (int i = 0; i < Row; ++i)
            {
                result[i][i] = 1;
            }
            return result;
        }

        Mat<T, Col, Row> transpose() const
        {
            Mat<T, Col, Row> result;
            for (int i = 0; i < Row; ++i)
            {
                for (int j = 0; j < Col; ++j)
                {
                    result[i][j] = data[j][i];
                }
            }
            return result;
        }

        static const Mat<T, Row, Col> IDENTITY;
        static const Mat<T, Row, Col> ZERO;
        static const Mat<T, Row, Col> ONE;


        Vec<T, Row>& operator[](int index)
        {
            return data[index];
        }

        const Vec<T, Row>& operator[](int index) const
        {
            return data[index];
        }

        template <int C>
        Mat<T, Row, C> operator *(const Mat<T, Col, C>& other) const
        {
            Mat<T, Row, C> result(0);
            for (int i = 0; i < Row; ++i)
            {
                for (int j = 0; j < C; ++j)
                {
                    for (int k = 0; k < Col; ++k)
                    {
                        result[j][i] += data[k][i] * other[j][k];
                    }
                }
            }
            return result;
        }

        Mat<T, Row, 1> operator *(const Vec<T, Col>& other) const
        {
            Mat<T, Row, 1> result(0);
            for (int i = 0; i < Row; ++i)
            {
                for (int k = 0; k < Col; ++k)
                {
                    result[0][i] += data[k][i] * other[k];
                }
            }
            return result;
        }

        DECLARE_MAT_OPT(+, Row, Col)
        DECLARE_MAT_OPT(-, Row, Col)
        DECLARE_MAT_OPT_SINGLE(+, Row, Col)
        DECLARE_MAT_OPT_SINGLE(-, Row, Col)
        DECLARE_MAT_OPT_SINGLE(*, Row, Col)
        DECLARE_MAT_OPT_SINGLE(/, Row, Col)
    };


    inline Vec<float, 3> FORWARD({0, 0, 1});
    inline Vec<float, 3> UP({0, 1, 0});
    inline Vec<float, 3> LEFT({1, 0, 0});


    DEFINE_MAT_FUNCTOR_2(fmin)
    DEFINE_MAT_FUNCTOR_2(fmax)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(sin)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(cos)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(tan)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(abs)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(pow, float, t)
    DEFINE_MAT_FUNCTOR_BASE_PARAM(log, float, t)


    DEFINE_VEC_FUNCTOR_BASE_PARAM(sin)
    DEFINE_VEC_FUNCTOR_BASE_PARAM(cos)
    DEFINE_VEC_FUNCTOR_BASE_PARAM(tan)
    DEFINE_VEC_FUNCTOR_BASE_PARAM(abs)
    DEFINE_VEC_FUNCTOR_BASE_PARAM(pow, float, t)
    DEFINE_VEC_FUNCTOR_BASE_PARAM(log, float, t)
    DEFINE_VEC_FUNCTOR_2(fmin)
    DEFINE_VEC_FUNCTOR_2(fmax)


    template <typename T>
    T cross(const Vec<T, 2>& a, const Vec<T, 2>& b)
    {
        return a[0] * b[1] - b[0] * a[1];
    }

    template <typename T>
    Vec<T, 3> cross(const Vec<T, 3>& a, const Vec<T, 3>& b)
    {
        return Vec<T, 3>({
            a.data[1] * b.data[2] - a.data[2] * b.data[1], a.data[2] * b.data[0] - a.data[0] * b.data[2],
            a.data[0] * b.data[1] - a.data[1] * b.data[0]
        });
    }

    template <typename T>
    void cross_opt_3(const Vec<T, 3>& a, const Vec<T, 3>& b, T& result)
    {
        result = a.data[0] * b.data[1] - a.data[1] * b.data[0];
    }

    template <typename T>
    T dot(const Vec<T, 2>& a, const Vec<T, 2>& b)
    {
        return a[0] * b[0] + a[1] * b[1];
    }

    template <typename T>
    T dot(const Vec<T, 3>& a, const Vec<T, 3>& b)
    {
        return a.data[0] * b.data[0] + a.data[1] * b.data[1] + a.data[2] * b.data[2];
    }

    template <typename T>
    T dot(const Vec<T, 4>& a, const Vec<T, 4>& b)
    {
        return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
    }

    template <typename T, int COL>
    T dot(const Vec<T, COL>& a, const Vec<T, COL>& b)
    {
        T result = 0;
        for (int i = 0; i < COL; ++i)
        {
            result += a[i] * b[i];
        }
        return result;
    }

    template <typename T, int COL>
    T sum(const Vec<T, COL>& a)
    {
        T result = 0;
        for (int i = 0; i < COL; ++i)
        {
            result += a[i];
        }
        return result;
    }


    DEFINE_VEC_OPT(+)
    DEFINE_VEC_OPT(-)
    DEFINE_VEC_OPT(*)
    DEFINE_VEC_OPT(/)
    DEFINE_MAT_OPT(+)
    DEFINE_MAT_OPT(-)
    DEFINE_MAT_OPT_SINGLE(+)
    DEFINE_MAT_OPT_SINGLE(-)
    DEFINE_MAT_OPT_SINGLE(*)
    DEFINE_MAT_OPT_SINGLE(/)


    template <typename T, int Col>
    Vec<T, Col>::Vec(const L_MATH::Mat<T, Col, 1>& mat)
    {
        for (int i = 0; i < Col; ++i)
        {
            data[i] = mat[0][i];
        }
    }

    template <typename T, int Col>
    T Vec<T, Col>::dot(const Vec<T, Col>& other) const
    {
        return L_MATH::dot(*this, other);
    }

    template <typename T, int Col>
    T Vec<T, Col>::magnitude() const
    {
        return dot(*this);
    }

    template <typename T, int Col>
    T Vec<T, Col>::sqrt_magnitude() const
    {
        return sqrt(dot(*this));
    }

    template <typename T, int Col>
    Vec<T, Col> Vec<T, Col>::normalize() const
    {
        auto var = *this / this->sqrt_magnitude();
        return var;
    }


    template <typename T, int Row, int Col>
    const Mat<T, Row, Col> Mat<T, Row, Col>::IDENTITY = Mat::dialog();
    template <typename T, int Row, int Col>
    const Mat<T, Row, Col> Mat<T, Row, Col>::ZERO(0);

    template <typename T, int Col>
    const Vec<T, Col> Vec<T, Col>::ONE(1);
    template <typename T, int Col>
    const Vec<T, Col> Vec<T, Col>::ZERO(0);

    inline float deg2rad(float deg)
    {
        return deg * PI / 180;
    }

    inline Mat<float, 4, 4> rotate(const Vec<float, 3>& axis, float deg)
    {
        float x = deg2rad(deg);
        auto a = axis.normalize();
        auto b = FORWARD;
        if (::abs(a[2] - 1) < EPSILON)
        {
            b = UP;
        }
        b = cross(a, b);
        b = b.normalize();
        auto c = cross(a, b);
        Mat<float, 3, 3> axis_rot = Mat<float, 3, 3>({a, b, c});
        Mat<float, 3, 3> rot_mat_base({1, 0, 0, 0, ::cos(x), ::sin(x), 0, -::sin(x), ::cos(x)});
        auto rot_mat33 = axis_rot * rot_mat_base * axis_rot.transpose();
        Mat<float, 4, 4> rot_mat44 = Mat<float, 4, 4>::IDENTITY;
        rot_mat44.copy_from(0, 0, rot_mat33);
        return rot_mat44;
    }

    inline Mat<float, 4, 4> rotate(const Vec<float, 3>& from, const Vec<float, 3>& to)
    {
        auto left = from.normalize();
        auto _to = to.normalize();
        auto up_sn = cross(left, _to);
        auto up = up_sn.normalize();
        float sn = up_sn.magnitude();
        auto forward = cross(left, up);
        if (sn < EPSILON)
        {
            return Mat<float, 4, 4>::IDENTITY;
        }
        float cs = dot(left, _to);
        Mat<float, 3, 3> axis_rot = Mat<float, 3, 3>({left, up, forward});
        Mat<float, 3, 3> rot_mat_base({1, 0, 0, 0, cs, sn, 0, sn, cs});
        auto rot_mat_33 = axis_rot * rot_mat_base * axis_rot.transpose();
        Mat<float, 4, 4> rot_mat_44 = Mat<float, 4, 4>::IDENTITY;
        rot_mat_44.copy_from(0, 0, rot_mat_33);
        return rot_mat_44;
    }


    inline Mat<float, 4, 4> translate(const Vec<float, 3>& move)
    {
        Mat<float, 4, 4> result = Mat<float, 4, 4>::IDENTITY;
        result[3][0] = move[0];
        result[3][1] = move[1];
        result[3][2] = move[2];
        return result;
    }

    inline Mat<float, 4, 4> project(float near, float far, float fov, float ratio)
    {
        float h = 2 * near * ::tan(fov / 2 / 180 * PI);
        Mat<float, 4, 4> result({
            2 * near / (h * ratio), 0, 0, 0,
            0, 2 * near / h, 0, 0,
            0, 0, (near + far) / (near - far), -1,
            0, 0, 2 * near * far / (near - far), 0
        });
        return result;
    }


    inline Mat<float, 4, 4> ortho(float near, float far, float fov, float ratio)
    {
        float h = 2 * near * ::tan(fov / 2 / 180 * PI);
        Mat<float, 4, 4> result({
            2 / (h * ratio), 0, 0, 0,
            0, 2 / (ratio), 0, 0,
            0, 0, 2 / (near - far), 0,
            0, 0, (near + far) / (far - near), 1.0f
        });
        return result;
    }

    inline Mat<float, 4, 4> look_at(Vec<float, 3> look_dir, Vec<float, 3> up)
    {
        auto forward = look_dir.normalize();
        auto left = cross(up, forward);
        if (left.magnitude() < EPSILON)
        {
            return Mat<float, 4, 4>::IDENTITY;
        }
        left = left.normalize();
        up = cross(forward, left);
        auto mat_33 = Mat<float, 3, 3>({left, up, forward});
        auto mat_44 = Mat<float, 4, 4>::IDENTITY;
        mat_44.copy_from(0, 0, mat_33);
        return mat_44.transpose();
    }

    inline Mat<float, 4, 4> scale(const Vec3& scale)
    {
        auto mat = Mat<float, 4, 4>::IDENTITY * Mat<float, 4, 4>(Vec4(scale, 1));
        mat[3][3] = 1;
        return mat;
    }


    // 平面方程 plane_normal*x-c=0
    inline float intersect_plane(const L_MATH::Vec<float, 3>& point, const Vec3& dir, const Vec3& plane_normal, float c)
    {
        float t = -(L_MATH::dot(plane_normal, point) - c) / dot(dir, plane_normal);
        return t;
    }

    inline bool is_zero(float x)
    {
        return fabs(x) < EPSILON;
    }
    template<typename T,int N>
    inline void clamp(Vec<T,N>& point, const Vec<T,N>& min, const Vec<T,N>& max)
    {
        point = fmax(min, point);
        point = fmin(max, point);
    }

    inline uint32_t floor_pot(uint32_t n)
    {
#if DETECT_CC_GCC && (DETECT_ARCH_X86 || DETECT_ARCH_X86_64)
        if (n == 0)
            return 0;

        __asm__("bsr %1,%0"
               : "=r" (n)
               : "rm" (n)
               : "cc");
        return 1 << n;
#else
        n |= (n >> 1);
        n |= (n >> 2);
        n |= (n >> 4);
        n |= (n >> 8);
        n |= (n >> 16);
        return n - (n >> 1);
#endif
    }

    inline uint32_t ceil_pot(uint32_t n)
    {
        return floor_pot(n - 1) << 1;
    }

    inline uint32_t pot_count(uint32_t n)
    {
        RUNTIME_ASSERT(n!=0, "pot_count Potentially zero");
        return 31 - __builtin_clz(n);
    }

    template <class T>
    inline T linear2(T &a, T &b, float alpha)
    {
        return (T)(a * (1 - alpha) + b * alpha);
    }

    template <class T>
    inline T linear3(T &a, T &b, T &c,Vec3 &alpha)
    {
        return (T)(a * alpha[0] + b * alpha[1] + c * alpha[2]);
    }

    inline  void alpha4(float alpha_x,float alpha_y,Vec4& alpha)
    {
        alpha[0] = (1 - alpha_x) * (1 - alpha_y);
        alpha[1] = (1 - alpha_y) * alpha_x;
        alpha[2] = (1 - alpha_x) * alpha_y;
        alpha[3] = alpha_y * alpha_x;
    }

    template <class T>
    inline T linear4(T &a, T &b, T &c, T &d,Vec4 &alpha)
    {
        return (T)(a * alpha[0] + b * alpha[1] + c * alpha[2] + d * alpha[3]);
    }

    inline void Sutherland_Hodgman(std::vector<Vec3>& clip_plane_normal,
                                   std::vector<float>& clip_plane_c,
                                   std::vector<Vec3>& ccw_points, std::vector<std::vector<float>>* result)
    {
        std::vector<std::vector<float>>* ccw_points_temps[2];
        std::vector<std::vector<float>> ccw_points_temp_3;
        ccw_points_temps[(clip_plane_normal.size() & 1) ^ 1] = &ccw_points_temp_3;
        ccw_points_temps[clip_plane_normal.size() & 1] = result;
        int temp_index = 0;
        for (int i = 0; i < ccw_points.size(); i++)
        {
            std::vector<float> temp_c(ccw_points.size(), 0);
            temp_c[i] = 1;
            ccw_points_temps[temp_index]->emplace_back(std::move(temp_c));
        }

        auto get_pos = [&ccw_points](std::vector<float>& alpha)
        {
            Vec3 pos;
            for (int i = 0; i < ccw_points.size(); i++)
            {
                pos += ccw_points[i] * alpha[i];
            }
            return pos;
        };

        auto calculate_alpha = [](std::vector<float>& alpha_l, std::vector<float>& alpha_r, float t,std::vector<float>& result)
        {
            for (int i = 0; i < alpha_l.size(); ++i)
            {
                result[i] = alpha_l[i] * (1 - t) + alpha_r[i] * t;
            }
        };

        for (int i = 0; i < clip_plane_normal.size(); ++i)
        {
            auto& p_normal = clip_plane_normal[i];
            float c = clip_plane_c[i];
            auto& ccw_points_temp_1 = *(ccw_points_temps[temp_index]);
            auto& ccw_points_temp_2 = *(ccw_points_temps[temp_index ^ 1]);
            ccw_points_temp_2.clear();
            for (int l = 0; l < ccw_points_temp_1.size(); ++l)
            {
                auto& alpha_l = ccw_points_temp_1[l];
                auto l_p = get_pos(alpha_l);
                bool l_inside = dot(p_normal, l_p) - c < 0;
                auto r = (l + 1) % ccw_points_temp_1.size();
                auto& alpha_r = ccw_points_temp_1[r];
                auto r_p = get_pos(alpha_r);
                // 假定平面方程<0是内部
                bool r_inside = dot(p_normal, r_p) - c < 0;
                if (!l_inside && !r_inside)
                {
                    continue;
                }
                if (l_inside)
                {
                    ccw_points_temp_2.emplace_back(alpha_l);
                }
                if (!l_inside || !r_inside)
                {
                    auto dir = r_p - l_p;
                    auto t = intersect_plane(l_p, dir, p_normal, c);
                    if (t > 0 && t < 1)
                    {
                        std::vector<float> result_l(ccw_points_temp_1.size(), 0);
                        calculate_alpha(alpha_l, alpha_r, t,result_l);
                        ccw_points_temp_2.emplace_back(std::move(result_l));
                    }
                }
            }
            temp_index ^= 1;
        }
    }
}


#undef FUNC_PARAM_USING
#undef FUNC_PARAM_DECLARE
#undef DECLARE_VEC_OPT
#undef DECLARE_MAT_OPT
#undef DECLARE_MAT_OPT_SINGLE
#undef DECLARE_VEC_OPT_SINGLE
#undef DEFINE_MAT_OPT
#undef DEFINE_VEC_OPT
#undef DEFINE_MAT_OPT_SINGLE
#undef FUNC_PARAM_USING_2
#undef FUNC_PARAM_USING_4
#undef FUNC_PARAM_USING_ERROR
#undef FUNC_PARAM_DECLARE_4
#undef FUNC_PARAM_DECLARE_2
#undef FUNC_PARAM_DECLARE_ERROR
#undef DEFINE_MAT_FUNCTOR_BASE_PARAM
#undef DEFINE_VEC_FUNCTOR_BASE_PARAM
#undef DEFINE_MAT_FUNCTOR_2
#undef DEFINE_VEC_FUNCTOR_2


#endif