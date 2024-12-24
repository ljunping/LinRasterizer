//
// Created by Lin on 2024/12/5.
//

#ifndef COMMONMACRO_H
#define COMMONMACRO_H
#include <memory>
#define SHARE_PTR std::shared_ptr
#define UNIQUE_PTR std::unique_ptr
#define WEAK_PTR std::weak_ptr


#define DEFINE_UNIFORM(FORM_TYPE)\
    private:std::unordered_map<int, FORM_TYPE> FORM_TYPE##_uniform;\
    public:FORM_TYPE get_##FORM_TYPE##_uniform(int uniform_name)\
    {\
        auto _iter=FORM_TYPE##_uniform.find(uniform_name);\
        return _iter!=FORM_TYPE##_uniform.end()?_iter->second:FORM_TYPE{};\
    }\
    public:void set_##FORM_TYPE##_uniform(int uniform_name,FORM_TYPE value)\
    {\
        FORM_TYPE##_uniform[uniform_name] = value;\
    }
#endif //COMMONMACRO_H
