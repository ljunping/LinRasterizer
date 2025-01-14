//
// Created by Lin on 2024/12/4.
//

#ifndef MATERIAL_H
#define MATERIAL_H

#include "L_math.h"
#include "Color.h"
#include "CommonMacro.h"
#include "Geometry.h"
#include "Object.h"
#include "Resource.h"
#include "TrianglePrimitive.h"

struct TrianglePrimitive;
class Texture;
struct RayCasterResult;
class FragShader;
class Pdf
{
public:
    virtual ~Pdf();
    virtual float value(const Vec3& direction) const =0;
    virtual Vec3 generate() const =0;
};

class SpherePdf : public Pdf
{
public:
    float value(const L_MATH::Vec<float, 3>& direction) const override;
    L_MATH::Vec<float, 3> generate() const override;
};

class CosinePdf: public Pdf
{
    Vec3 normal;
public:
    explicit CosinePdf(const Vec3& normal);
    float value(const L_MATH::Vec<float, 3>& direction) const override;
    L_MATH::Vec<float, 3> generate() const override;
};
class PhongPdf:public Pdf
{
    int n;
    Vec3 reflect;
public:
    PhongPdf(const Vec3& reflect, int n);
    float value(const L_MATH::Vec<float, 3>& direction) const override;
    L_MATH::Vec<float, 3> generate() const override;
};

class MixPdf : public Pdf
{
    std::vector<SHARE_PTR<Pdf>> pdfs;
    std::vector<float> weights;
    std::vector<float> weights_sum;
    bool avage_weight = false;
public:
    MixPdf() = default;
    explicit MixPdf(std::vector<SHARE_PTR<Pdf>>& pdfs);
    explicit MixPdf(std::vector<SHARE_PTR<Pdf>>& pdfs, std::vector<float>& weight);
    float value(const Vec3& direction) const override;
    L_MATH::Vec<float, 3> generate() const override;
};

class HittablePdf: public Pdf
{
    Geometry& geometry;
    Vec3 origin;
public:
    HittablePdf(Geometry& geometry, const Vec3& origin);
    float value(const L_MATH::Vec<float, 3>& direction) const override;
    L_MATH::Vec<float, 3> generate() const override;
};

struct ScatterResult
{
    SHARE_PTR<Pdf> pdf;
    bool skip_pdf;
    Ray skip_pdf_ray;
};


class Material : public Resource
{
    INIT_TYPE(Material, Resource)
    DEFINE_UNIFORM(bool)
    DEFINE_UNIFORM(float)
    DEFINE_UNIFORM(int)
    DEFINE_UNIFORM(Color)
    DEFINE_UNIFORM(Vec4)
    DEFINE_UNIFORM(Vec3)
    DEFINE_UNIFORM(Vec2)
    DEFINE_UNIFORM(Mat44)
    DEFINE_UNIFORM(Mat33)
public:
    void on_create() override;
    virtual float emit(const Ray& ray_int, const RayCasterResult& hit);
    virtual bool scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result);
    virtual float scatter_pdf(const Ray& ray_int, const RayCasterResult& hit, const ScatterResult& scatter_result);
    virtual float f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l,
                    const L_MATH::Vec<float, 3>& v);
};


class LambertianMaterial:public Material
{
    INIT_TYPE(LambertianMaterial, Material)
protected:
public:
    bool scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result) override;
    float scatter_pdf(const Ray& ray_int, const RayCasterResult& hit, const ScatterResult& scatter_result) override;
    float f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l,
                            const L_MATH::Vec<float, 3>& v) override;
};

class BliPhongMaterial:public Material
{
    INIT_TYPE(BliPhongMaterial, Material)
protected:
    explicit BliPhongMaterial(SHARE_PTR<Texture>& diffue_texture,SHARE_PTR<Texture>& specular_texture);
public:
    int n;
    float kd, ks;
    BliPhongMaterial(int n, float kd, float ks);
    bool scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result) override;
    float scatter_pdf(const Ray& ray_int, const RayCasterResult& hit, const ScatterResult& scatter_result) override;
    float f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l,
            const L_MATH::Vec<float, 3>& v) override;
};

class MetalMaterial:public Material
{
    INIT_TYPE(MetalMaterial, Material)
    float fuzz;
protected:
    explicit MetalMaterial(float fuzz);
public:
    bool scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result) override;
};

class DiffuseLightMaterial : public Material
{
    INIT_TYPE(DiffuseLightMaterial, Material)
    float c_light;
protected:
    explicit DiffuseLightMaterial(float c_light);
public:
    float emit(const Ray& ray_int, const RayCasterResult& hit) override;
};
#endif //MATERIAL_H
