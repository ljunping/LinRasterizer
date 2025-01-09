//
// Created by Lin on 2024/11/12.
//

#ifndef L_MATH_H
#define L_MATH_H
#pragma once
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <stdexcept>

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

#define INT_HIGHEST_BIT_INDEX(n) (31-__builtin_clz(n))
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
    template <int COL2>\
    Vec<T, MIN_RC<COL, COL2>> operator OPT(const Vec<T, COL2>& other) const;\
    Vec<T, COL> operator OPT(const T& other) const;\
    template <int COL2>\
    void operator OPT##=(const Vec<T, COL2>& other);\
    void operator OPT##=(const T& other);
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
    template <int COL2>\
    Vec<T, MIN_RC<COL, COL2>> Vec<T, COL>:: operator OPT(const Vec<T, COL2>& other) const\
    {\
        Vec<T, MIN_RC<COL, COL2>> result;\
        if constexpr (MIN_RC<COL, COL2> == 2)\
        {\
            result.data[0] = data[0] OPT other.data[0];\
            result.data[1] = data[1] OPT other.data[1];\
        }\
        if constexpr (MIN_RC<COL, COL2> == 3)\
        {\
            result.data[0] = data[0] OPT other.data[0];\
            result.data[1] = data[1] OPT other.data[1];\
            result.data[2] = data[2] OPT other.data[2];\
        }\
        else\
        {\
            for (int i = 0; i < MIN_RC<COL, COL2>; ++i)\
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
    template <typename  T, int COL>\
    template <int COL2>\
    void Vec<T, COL>::operator OPT##=(const Vec<T, COL2>& other)\
    {\
        if constexpr (MIN_RC<COL, COL2> == 2)\
        {\
            data[0] OPT##= other[0];\
            data[1] OPT##= other[1];\
            return;\
        }\
        if constexpr (MIN_RC<COL, COL2> == 3)\
        {\
            data[0] OPT##= other[0];\
            data[1] OPT##= other[1];\
            data[2] OPT##= other[2];\
            return;\
        }\
        for (int i = 0; i < MIN_RC<COL, COL2>; ++i)\
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
    inline bool is_zero(float x)
    {
        return fabs(x) < EPSILON;
    }

    template <typename T, int Col>
    class Vec;
    extern Vec<float, 3> FORWARD;
    extern Vec<float, 3> UP;
    extern Vec<float, 3> LEFT;

    template <int Row, int Col>
    constexpr int min()
    {
        return (Row < Col) ? Row : Col;
    }

    template <int Row, int Col>
    constexpr int max()
    {
        return (Row < Col) ? Col : Row;
    }

    template <int Row, int Col>
    constexpr int MIN_RC = min<Row, Col>();

    template <int Row, int Col>
    constexpr int MAX_RC = max<Row, Col>();

    template <typename T, int Row, int Col>
    class Mat;

    template <typename T, int Col>
    class Vec
    {
    public:
        T data[MAX_RC<Col, 4>]{};
        Vec() = default;

        explicit Vec(T v1)
        {
            for (int i = 0; i < Col; ++i)
            {
                data[i] = v1;
            }
        }

        explicit Vec(T v1, T v2)
        {
            data[0] = v1;
            data[1] = v2;
        }

        explicit Vec(T v1, T v2, T v3)
        {
            data[0] = v1;
            data[1] = v2;
            data[2] = v3;
        }

        explicit Vec(T v1, T v2, T v3, T v4)
        {
            data[0] = v1;
            data[1] = v2;
            data[2] = v3;
            data[3] = v4;
        }

        template <int COL2>
        explicit operator L_MATH::Vec<T, COL2>&()
        {
            if constexpr (Col > 4 || COL2 > 4)
            {
                throw std::runtime_error("L_MATH::Vec constructor called with invalid arguments");
            }
            return reinterpret_cast<L_MATH::Vec<T, COL2>&>(*this);
        }

        template <int COL2>
        explicit operator const L_MATH::Vec<T, COL2>&()
        {
            if constexpr (Col > 4 || COL2 > 4)
            {
                throw std::runtime_error("L_MATH::Vec constructor called with invalid arguments");
            }
            return reinterpret_cast<const L_MATH::Vec<T, COL2>&>(*this);
        }

        template <int COL2>
        explicit operator const L_MATH::Vec<T, COL2>&() const
        {
            if constexpr (Col > 4 || COL2 > 4)
            {
                throw std::runtime_error("L_MATH::Vec constructor called with invalid arguments");
            }
            return reinterpret_cast<const L_MATH::Vec<T, COL2>&>(*this);
        }

        Vec(const std::initializer_list<T>& list)
        {
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

        template <int Row>
        Vec<T, Row> operator*(const Mat<T, Col, Row>& mat) const;

        template <int Row>
        Vec<T, Row> mul_transpose(const Mat<T, Row, Col>& mat) const;


        template <int Row>
        explicit Vec(const Mat<T, Row, 1>& mat);

        template <int Row>
        explicit Vec(const Vec<T, Row>& vec);

        template <int Row>
        explicit Vec(Vec<T, Row>&& vec);

        template <int Row>
        Vec<T, Col>& operator=(const Vec<T, Row>& vec);

        template <int Row>
        Vec<T, Col>& operator=(Vec<T, Row>&& vec);

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

        T dot(const Vec<T, Col>& other) const;
        T magnitude() const;
        T sqrt_magnitude() const;
        T invert_sqrt_magnitude() const;
        Vec<T, Col> normalize() const;
        void normalized();

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
        Vec<T, Row> data[MAX_RC<Col, 4>]{};

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
        Mat() = default;

        explicit Mat(T t)
        {
            clear(t);
        }

        explicit Mat(const Vec<T, Row>& diagV)
        {
            clear(0);
            for (int i = 0; i < Row; ++i)
            {
                data[i][i] = diagV[i];
            }
        }

        explicit Mat(const std::initializer_list<Vec<T, Row>>& list)
        {
            if (list.size() != Col)
            {
                return;
            }
            //先填充列
            for (int i = 0; i < Col; ++i)
            {
                data[i] = (list.begin()[i]);
            }
        }

        explicit Mat(const std::initializer_list<T>& list)
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

        template <int ROW2, int COL2>
        explicit operator L_MATH::Mat<T, ROW2, COL2>&()
        {
            if constexpr (ROW2 > 4 || COL2 > 4 || Row > 4 || Col > 4)
            {
                throw std::runtime_error("L_MATH::Mat constructor called with invalid arguments");
            }
            return reinterpret_cast<L_MATH::Mat<T, ROW2, COL2>&>(*this);
        }

        template <int ROW2, int COL2>
        explicit operator const L_MATH::Mat<T, ROW2, COL2>&() const
        {
            if constexpr (ROW2 > 4 || COL2 > 4 || Row > 4 || Col > 4)
            {
                throw std::runtime_error("L_MATH::Mat constructor called with invalid arguments");
            }
            return reinterpret_cast<const L_MATH::Mat<T, ROW2, COL2>&>(*this);
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
            Mat<T, Row, 1> result(0.f);
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

    template <typename T, int Row, int COL>
    T sum(const Mat<T, Row, COL>& a)
    {
        T result = 0;
        for (int i = 0; i < COL; ++i)
        {
            for (int k = 0; k < Row; ++k)
            {
                result += a[i][k];
            }
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
    template <int Row>
    Vec<T, Row> Vec<T, Col>::operator*(const Mat<T, Col, Row>& mat) const
    {
        Vec<T, Row> result;
        for (int i = 0; i < Row; ++i)
        {
            for (int k = 0; k < Col; ++k)
            {
                result.data[i] += data[k] * mat[i][k];
            }
        }
        return result;
    }

    template <typename T, int Col>
    template <int Row>
    Vec<T, Row> Vec<T, Col>::mul_transpose(const Mat<T, Row, Col>& mat) const
    {
        Vec<T, Row> result;
        for (int i = 0; i < Row; ++i)
        {
            for (int k = 0; k < Col; ++k)
            {
                result.data[i] += data[k] * mat[k][i];
            }
        }
        return result;
    }


    template <typename T, int Col>
    template <int Row>
    Vec<T, Col>::Vec(const Mat<T, Row, 1>& mat)
    {
        std::memcpy(this->data, mat.data[0].data, sizeof(T) * MIN_RC<Row, Col>());
    }

    template <typename T, int Col>
    template <int Row>
    Vec<T, Col>::Vec(const Vec<T, Row>& vec)
    {
        std::memcpy(this->data, vec.data, sizeof(T) * MIN_RC<Row, Col>);
    }

    template <typename T, int Col>
    template <int Row>
    Vec<T, Col>::Vec(Vec<T, Row>&& vec)
    {
        std::memcpy(this->data, vec.data, sizeof(T) * MIN_RC<Row, Col>);
    }

    template <typename T, int Col>
    template <int Row>
    Vec<T, Col>& Vec<T, Col>::operator=(const Vec<T, Row>& vec)
    {
        std::memcpy(this->data, vec.data, sizeof(T) * MIN_RC<Row, Col>);
        return *this;
    }

    template <typename T, int Col>
    template <int Row>
    Vec<T, Col>& Vec<T, Col>::operator=(Vec<T, Row>&& vec)
    {
        std::memcpy(this->data, vec.data, sizeof(T) * MIN_RC<Row, Col>);
        return *this;
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
    T Vec<T, Col>::invert_sqrt_magnitude() const
    {
        return 1 / ::sqrt(dot(*this));
    }

    template <typename T, int Col>
    Vec<T, Col> Vec<T, Col>::normalize() const
    {
        auto var = *this / this->sqrt_magnitude();
        return var;
    }

    template <typename T, int Col>
    void Vec<T, Col>::normalized()
    {
        auto sqrt_magnitude = this->sqrt_magnitude();
        *this /= sqrt_magnitude;
    }


    template <typename T, int Row, int Col>
    const Mat<T, Row, Col> Mat<T, Row, Col>::IDENTITY = Mat<T, Row, Col>::dialog();
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
        if (is_zero(deg))
        {
            return Mat<float, 4, 4>::IDENTITY;
        }
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
        Mat<float, 3, 3> rot_mat33 = axis_rot * rot_mat_base * axis_rot.transpose();
        rot_mat33[3][3] = 1;
        return static_cast<L_MATH::Mat<float, 4, 4>&>(rot_mat33);
    }

    inline Mat<float, 4, 4> rotate(const Vec<float, 3>& from, const Vec<float, 3>& to)
    {
        auto left = from.normalize();
        auto _to = to.normalize();
        auto up_sn = cross(left, _to);
        float _up_sn_l = up_sn.sqrt_magnitude();
        if (_up_sn_l < EPSILON)
        {
            return Mat<float, 4, 4>::IDENTITY;
        }
        auto up = up_sn / _up_sn_l;
        auto forward = cross(left, up);
        float cs = dot(left, _to);
        float sn = dot(forward, _to);
        Mat<float, 3, 3> axis_rot = Mat<float, 3, 3>({left, up, forward});
        Mat<float, 3, 3> rot_mat_base({cs, 0, sn, 0, 1, 0, -sn, 0, cs});
        auto rot_mat_33 = axis_rot * rot_mat_base * axis_rot.transpose();
        rot_mat_33[3][3] = 1;
        return static_cast<L_MATH::Mat<float, 4, 4>&>(rot_mat_33);
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


    inline Mat<float, 4, 4> ortho(float near, float far, float w, float h)
    {
        Mat<float, 4, 4> result({
            2 / w, 0, 0, 0,
            0, 2 / h, 0, 0,
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
        mat_33 = mat_33.transpose();
        return static_cast<Mat<float, 4, 4>&>(mat_33);
    }

    inline Mat<float, 4, 4> scale(const Vec3& __scale)
    {
        const Vec4& _scale = (const Vec4&)__scale;
        Mat<float, 4, 4> result = Mat<float, 4, 4>::IDENTITY;
        result[0][0] = _scale[0];
        result[1][1] = _scale[1];
        result[2][2] = _scale[2];
        result[3][3] = 1;
        return result;
    }


    template <typename T, int N>
    inline void clamp(Vec<T, N>& point, const Vec<T, N>& min, const Vec<T, N>& max)
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
        return 31 - __builtin_clz(n);
    }

    template <class T>
    inline T linear2(T& a, T& b, float alpha)
    {
        return (T)(a * (1 - alpha) + b * alpha);
    }

    template <class T>
    inline T linear3(T& a, T& b, T& c,Vec3& alpha)
    {
        return (T)(a * alpha[0] + b * alpha[1] + c * alpha[2]);
    }

    inline void alpha4(float alpha_x, float alpha_y,Vec4& alpha)
    {
        alpha[0] = (1 - alpha_x) * (1 - alpha_y);
        alpha[1] = (1 - alpha_y) * alpha_x;
        alpha[2] = (1 - alpha_x) * alpha_y;
        alpha[3] = alpha_y * alpha_x;
    }

    template <class T>
    inline T linear4(T& a, T& b, T& c, T& d, const Vec4& alpha)
    {
        return (T)(a * alpha[0] + b * alpha[1] + c * alpha[2] + d * alpha[3]);
    }

    inline float f2_distance(const Mat44& a, const Mat44& b)
    {
        return sum(pow(a - b, 2));
    }



    inline void pos3_l_dot_mat44(Vec3& v, const Mat44& mat)
    {
        Vec4& v4 = static_cast<L_MATH::Vec<float, 4>&>(v);
        v4[3] = 1;
        v4 = v4.mul_transpose(mat);
        v /= v4[3];
    }

    inline Vec3 pos3_dot_mat44(const Vec3& v, const Mat44& mat)
    {
        Vec4 v4 = static_cast<L_MATH::Vec<float, 4>>(v);
        v4[3] = 1;
        v4 = v4.mul_transpose(mat);
        v4 /= v4[3];
        return static_cast<Vec3&>(v4);
    }

    inline void pos3_l_dot_mat33(Vec3& v, const Mat44& mat)
    {
        v = v.mul_transpose(static_cast<const Mat33&>(mat));
    }

    inline Vec3 pos3_dot_mat33(Vec3& v, const Mat44& mat)
    {
        Vec3 _v = v;
        _v = _v.mul_transpose(static_cast<const Mat33&>(mat));
        return _v;
    }


    inline Mat44 compose_trs(const Vec3& t, const Vec3& r, const Vec3& s)
    {
        return L_MATH::translate(t) *
            L_MATH::rotate(L_MATH::FORWARD, r[2]) *
            L_MATH::rotate(L_MATH::UP, r[1]) *
            L_MATH::rotate(L_MATH::LEFT, r[0]) *
            L_MATH::scale(s);
    }

    inline float determinant(const Mat44& m)
    {
        return
            m[3][0] * m[2][1] * m[1][2] * m[0][3] - m[2][0] * m[3][1] * m[1][2] * m[0][3] -
            m[3][0] * m[1][1] * m[2][2] * m[0][3] + m[1][0] * m[3][1] * m[2][2] * m[0][3] +
            m[2][0] * m[1][1] * m[3][2] * m[0][3] - m[1][0] * m[2][1] * m[3][2] * m[0][3] -
            m[3][0] * m[2][1] * m[0][2] * m[1][3] + m[2][0] * m[3][1] * m[0][2] * m[1][3] +
            m[3][0] * m[0][1] * m[2][2] * m[1][3] - m[0][0] * m[3][1] * m[2][2] * m[1][3] -
            m[2][0] * m[0][1] * m[3][2] * m[1][3] + m[0][0] * m[2][1] * m[3][2] * m[1][3] +
            m[3][0] * m[1][1] * m[0][2] * m[2][3] - m[1][0] * m[3][1] * m[0][2] * m[2][3] -
            m[3][0] * m[0][1] * m[1][2] * m[2][3] + m[0][0] * m[3][1] * m[1][2] * m[2][3] +
            m[1][0] * m[0][1] * m[3][2] * m[2][3] - m[0][0] * m[1][1] * m[3][2] * m[2][3] -
            m[2][0] * m[1][1] * m[0][2] * m[3][3] + m[1][0] * m[2][1] * m[0][2] * m[3][3] +
            m[2][0] * m[0][1] * m[1][2] * m[3][3] - m[0][0] * m[2][1] * m[1][2] * m[3][3] -
            m[1][0] * m[0][1] * m[2][2] * m[3][3] + m[0][0] * m[1][1] * m[2][2] * m[3][3];
    }


    inline Mat44 inverse(const Mat44& m)
    {
        Mat44 inv;
        float det = determinant(m);
        if (is_zero(det))
        {
            throw std::runtime_error("Matrix is not invertible");
        }
        inv[0][0] = (m[1][1] * m[2][2] * m[3][3] + m[1][2] * m[2][3] * m[3][1] + m[1][3] * m[2][1] * m[3][2]
            - m[1][1] * m[2][3] * m[3][2] - m[1][2] * m[2][1] * m[3][3] - m[1][3] * m[2][2] * m[3][1]) / det;
        inv[0][1] = -(m[0][1] * m[2][2] * m[3][3] + m[0][2] * m[2][3] * m[3][1] + m[0][3] * m[2][1] * m[3][2]
            - m[0][1] * m[2][3] * m[3][2] - m[0][2] * m[2][1] * m[3][3] - m[0][3] * m[2][2] * m[3][1]) / det;
        inv[0][2] = (m[0][1] * m[1][2] * m[3][3] + m[0][2] * m[1][3] * m[3][1] + m[0][3] * m[1][1] * m[3][2]
            - m[0][1] * m[1][3] * m[3][2] - m[0][2] * m[1][1] * m[3][3] - m[0][3] * m[1][2] * m[3][1]) / det;
        inv[0][3] = -(m[0][1] * m[1][2] * m[2][3] + m[0][2] * m[1][3] * m[2][1] + m[0][3] * m[1][1] * m[2][2]
            - m[0][1] * m[1][3] * m[2][2] - m[0][2] * m[1][1] * m[2][3] - m[0][3] * m[1][2] * m[2][1]) / det;
        inv[1][0] = -(m[1][0] * m[2][2] * m[3][3] + m[1][2] * m[2][3] * m[3][0] + m[1][3] * m[2][0] * m[3][2]
            - m[1][0] * m[2][3] * m[3][2] - m[1][2] * m[2][0] * m[3][3] - m[1][3] * m[2][2] * m[3][0]) / det;
        inv[1][1] = (m[0][0] * m[2][2] * m[3][3] + m[0][2] * m[2][3] * m[3][0] + m[0][3] * m[2][0] * m[3][2]
            - m[0][0] * m[2][3] * m[3][2] - m[0][2] * m[2][0] * m[3][3] - m[0][3] * m[2][2] * m[3][0]) / det;
        inv[1][2] = -(m[0][0] * m[1][2] * m[3][3] + m[0][2] * m[1][3] * m[3][0] + m[0][3] * m[1][0] * m[3][2]
            - m[0][0] * m[1][3] * m[3][2] - m[0][2] * m[1][0] * m[3][3] - m[0][3] * m[1][2] * m[3][0]) / det;
        inv[1][3] = (m[0][0] * m[1][2] * m[2][3] + m[0][2] * m[1][3] * m[2][0] + m[0][3] * m[1][0] * m[2][2]
            - m[0][0] * m[1][3] * m[2][2] - m[0][2] * m[1][0] * m[2][3] - m[0][3] * m[1][2] * m[2][0]) / det;
        inv[2][0] = (m[1][0] * m[2][1] * m[3][3] + m[1][1] * m[2][3] * m[3][0] + m[1][3] * m[2][0] * m[3][1]
            - m[1][0] * m[2][3] * m[3][1] - m[1][1] * m[2][0] * m[3][3] - m[1][3] * m[2][1] * m[3][0]) / det;
        inv[2][1] = -(m[0][0] * m[2][1] * m[3][3] + m[0][1] * m[2][3] * m[3][0] + m[0][3] * m[2][0] * m[3][1]
            - m[0][0] * m[2][3] * m[3][1] - m[0][1] * m[2][0] * m[3][3] - m[0][3] * m[2][1] * m[3][0]) / det;
        inv[2][2] = (m[0][0] * m[1][1] * m[3][3] + m[0][1] * m[1][3] * m[3][0] + m[0][3] * m[1][0] * m[3][1]
            - m[0][0] * m[1][3] * m[3][1] - m[0][1] * m[1][0] * m[3][3] - m[0][3] * m[1][1] * m[3][0]) / det;
        inv[2][3] = -(m[0][0] * m[1][1] * m[2][3] + m[0][1] * m[1][3] * m[2][0] + m[0][3] * m[1][0] * m[2][1]
            - m[0][0] * m[1][3] * m[2][1] - m[0][1] * m[1][0] * m[2][3] - m[0][3] * m[1][1] * m[2][0]) / det;
        inv[3][0] = -(m[1][0] * m[2][1] * m[3][2] + m[1][1] * m[2][2] * m[3][0] + m[1][2] * m[2][0] * m[3][1]
            - m[1][0] * m[2][2] * m[3][1] - m[1][1] * m[2][0] * m[3][2] - m[1][2] * m[2][1] * m[3][0]) / det;
        inv[3][1] = (m[0][0] * m[2][1] * m[3][2] + m[0][1] * m[2][2] * m[3][0] + m[0][2] * m[2][0] * m[3][1]
            - m[0][0] * m[2][2] * m[3][1] - m[0][1] * m[2][0] * m[3][2] - m[0][2] * m[2][1] * m[3][0]) / det;
        inv[3][2] = -(m[0][0] * m[1][1] * m[3][2] + m[0][1] * m[1][2] * m[3][0] + m[0][2] * m[1][0] * m[3][1]
            - m[0][0] * m[1][2] * m[3][1] - m[0][1] * m[1][0] * m[3][2] - m[0][2] * m[1][1] * m[3][0]) / det;
        inv[3][3] = (m[0][0] * m[1][1] * m[2][2] + m[0][1] * m[1][2] * m[2][0] + m[0][2] * m[1][0] * m[2][1]
            - m[0][0] * m[1][2] * m[2][1] - m[0][1] * m[1][0] * m[2][2] - m[0][2] * m[1][1
                ] * m[2][0]) / det;
        return inv;
    }

    inline void decompose_trs(const Mat44& mat,Vec3& t,Vec3& r,Vec3& s)
    {
        t[0] = mat[3][0];
        t[1] = mat[3][1];
        t[2] = mat[3][2];

        s[0] = sqrt(mat[0][0] * mat[0][0] + mat[0][1] * mat[0][1] + mat[0][2] * mat[0][2]);
        s[1] = sqrt(mat[1][0] * mat[1][0] + mat[1][1] * mat[1][1] + mat[1][2] * mat[1][2]);
        s[2] = sqrt(mat[2][0] * mat[2][0] + mat[2][1] * mat[2][1] + mat[2][2] * mat[2][2]);

        r[1] = std::asin(-mat[0][2] / s[0]);

        if (std::cos(r[1]) > 0.0001)
        {
            // Check for gimbal lock
            r[0] = std::atan2(mat[1][2] * s[2], mat[2][2] * s[1]);
            r[2] = std::atan2(mat[0][1], mat[0][0]);
        }
        else
        {
            r[0] = std::atan2(-mat[2][1] * s[1], mat[1][1] * s[2]);
            r[2] = 0;
        }
    }

    inline Mat44 invert_trs_mat(const Mat44& trs_mat)
    {
        return inverse(trs_mat);
        // Vec3 t, r, s;
        // decompose_trs(trs_mat, t, r, s);
        // float alpha = r[0];
        // float beta = r[1];
        // float gamma = r[2];
        //
        // // Precompute sines and cosines
        // float cos_alpha = std::cos(alpha);
        // float sin_alpha = std::sin(alpha);
        // float cos_beta = std::cos(beta);
        // float sin_beta = std::sin(beta);
        // float cos_gamma = std::cos(gamma);
        // float sin_gamma = std::sin(gamma);
        //
        // // Inverse scaling
        // float inv_sx = 1.0f / s[0];
        // float inv_sy = 1.0f / s[1];
        // float inv_sz = 1.0f / s[2];
        //
        // Mat44 M_inv;
        // // Calculate inverse matrix elements (column-major order)
        // M_inv[0][0] = cos_beta * cos_gamma * inv_sx;
        // M_inv[1][0] = cos_beta * sin_gamma * inv_sx;
        // M_inv[2][0] = -sin_beta * inv_sx;
        // M_inv[3][0] = 0.0f;
        //
        // M_inv[0][1] = (sin_alpha * sin_beta * cos_gamma - cos_alpha * sin_gamma) * inv_sy;
        // M_inv[1][1] = (sin_alpha * sin_beta * sin_gamma + cos_alpha * cos_gamma) * inv_sy;
        // M_inv[2][1] = sin_alpha * cos_beta * inv_sy;
        // M_inv[3][1] = 0.0f;
        //
        // M_inv[0][2] = (cos_alpha * sin_beta * cos_gamma + sin_alpha * sin_gamma) * inv_sz;
        // M_inv[1][2] = (cos_alpha * sin_beta * sin_gamma - sin_alpha * cos_gamma) * inv_sz;
        // M_inv[2][2] = cos_alpha * cos_beta * inv_sz;
        // M_inv[3][2] = 0.0f;
        //
        // M_inv[0][3] = -(M_inv[0][0] * t[0] + M_inv[0][1] * t[1] + M_inv[0][2] * t[2]);
        // M_inv[1][3] = -(M_inv[1][0] * t[0] + M_inv[1][1] * t[1] + M_inv[1][2] * t[2]);
        // M_inv[2][3] = -(M_inv[2][0] * t[0] + M_inv[2][1] * t[1] + M_inv[2][2] * t[2]);
        // M_inv[3][3] = 1.0f;
        // return M_inv;
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
