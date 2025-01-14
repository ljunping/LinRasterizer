//
// Created by Lin on 2024/12/4.
//

#include "Material.h"

#include "BVHTree.h"
#include "Mesh.h"
#include "Texture.h"
#include "TrianglePrimitive.h"


float Material::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v)
{
    return 1 / PI;
}

float LambertianMaterial::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l, const L_MATH::Vec<float, 3>& v)
{
    return 1 / PI;
}

float BliPhongMaterial::f(const L_MATH::Vec<float, 3>& normal, const L_MATH::Vec<float, 3>& l,
                          const L_MATH::Vec<float, 3>& v)
{
    return ks * pow(dot(((l + v) / 2).normalize(), normal), n) + kd / PI;
}


Pdf::~Pdf()
{
}

float SpherePdf::value(const L_MATH::Vec<float, 3>& direction) const
{
    return 1.0 / (4 * PI);
}

L_MATH::Vec<float, 3> SpherePdf::generate() const
{
    return L_MATH::random_unit_vector();
}

CosinePdf::CosinePdf(const L_MATH::Vec<float, 3>& normal): normal(normal.normalize())
{
}


float CosinePdf::value(const L_MATH::Vec<float, 3>& direction) const
{
    auto cose = L_MATH::dot(normal, direction.normalize());
    return cose < 0 ? 0 : cose / PI;
}

L_MATH::Vec<float, 3> CosinePdf::generate() const
{
    auto rotate = L_MATH::rotate(L_MATH::FORWARD, normal);
    return L_MATH::pos3_dot_mat33(L_MATH::random_cosine_direction(), rotate);;
}

PhongPdf::PhongPdf(const L_MATH::Vec<float, 3>& reflect, int n): reflect(reflect), n(n)
{
}


float PhongPdf::value(const L_MATH::Vec<float, 3>& direction) const
{
    auto cose = dot(direction, reflect);
    return cose < 0 ? 0 : pow(cose, n) * (n + 1) / (2 * PI);
}

L_MATH::Vec<float, 3> PhongPdf::generate() const
{
    float r1 = L_MATH::random();
    float r2 = L_MATH::random();

    float cos_theta = std::pow(r1, 1.0 / (n + 1));
    float sin_theta = sqrt(1 - cos_theta * cos_theta);
    float phi = 2 * M_PI * r2;

    float x = sin_theta * std::cos(phi);
    float y = sin_theta * std::sin(phi);
    float z = cos_theta;
    Vec3 sample(x, y, z);
    auto rotate0 = L_MATH::rotate(L_MATH::FORWARD, reflect);
    L_MATH::pos3_l_dot_mat33(sample, rotate0);
    return sample;
}

MixPdf::MixPdf(std::vector<std::shared_ptr<Pdf>>& pdfs): pdfs(pdfs)
{
    avage_weight = true;
}

MixPdf::MixPdf(std::vector<std::shared_ptr<Pdf>>& pdfs, std::vector<float>& weight): pdfs(pdfs), weights(weight)
{
    weights_sum.assign(weights.size(), weights[0]);
    for (int i = 1; i < weights.size(); ++i)
    {
        weights_sum[i] = weights_sum[i - 1] + weights[i];
    }
    avage_weight = false;
}


float MixPdf::value(const L_MATH::Vec<float, 3>& direction) const
{
    if (avage_weight)
    {
        float value=0;
        for (auto& pdf : pdfs)
        {
            value += pdf->value(direction);
        }
        return value / pdfs.size();
    }
    float sum = weights_sum[weights_sum.size() - 1];
    float value=0;
    for (int i = 0; i < weights.size(); ++i)
    {
        value += weights[i] / sum * (pdfs[i]->value(direction));
    }
    return value;
}

L_MATH::Vec<float, 3> MixPdf::generate() const
{
    if (avage_weight)
    {
        int index = L_MATH::rand_int(0, pdfs.size()-1);
        return pdfs[index]->generate();
    }
    float thr = L_MATH::random() * weights_sum[weights_sum.size() - 1];
    for (int i = 0; i < weights.size() - 1; ++i)
    {
        if (weights_sum[i] >= thr)
        {
            return pdfs[i]->generate();
        }
    }
    return pdfs[pdfs.size() - 1]->generate();
}

HittablePdf::HittablePdf(Geometry& geometry, const L_MATH::Vec<float, 3>& origin): geometry(geometry), origin(origin)
{
}



float HittablePdf::value(const L_MATH::Vec<float, 3>& direction) const
{
    return geometry.pdf(origin, direction);
}

L_MATH::Vec<float, 3> HittablePdf::generate() const
{
    return geometry.random(origin);
}


void Material::on_create()
{
    this->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    this->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
    this->set_Vec4_uniform(MATERIAL_ENV_LIGHT, {1, 1, 1, 1});
    this->set_Vec4_uniform(MATERIAL_COLOR1, {1, 0, 0, 0.5});
    this->set_Vec3_uniform(MATERIAL_LIGHT_KD, {1, 1, 1});
    this->set_Vec3_uniform(MATERIAL_LIGHT_KS, {0.7, 0.7, 0.7});
    this->set_Vec3_uniform(MATERIAL_LIGHT_KE, {0.3, 0.3, 0.3});
    this->set_float_uniform(MATERIAL_NORMAL_BUMP_FACTOR, 1.0f);
}

float Material::emit(const Ray& ray_int, const RayCasterResult& hit)
{
    return 0;
}

bool Material::scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result)
{
    return false;
}

float Material::scatter_pdf(const Ray& ray_int, const RayCasterResult& hit, const ScatterResult& scatter_result)
{
    return 0;
}


bool LambertianMaterial::scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result)
{
    auto hit_point = ray_int.get_point(hit.t);
    scatter_result.pdf = std::make_shared<CosinePdf>(hit.geometry->get_normal(hit_point));
    scatter_result.skip_pdf = false;
    return true;
}

float LambertianMaterial::scatter_pdf(const Ray& ray_int, const RayCasterResult& hit,
    const ScatterResult& scatter_result)
{
    return scatter_result.pdf->value(ray_int.direction);
}

BliPhongMaterial::BliPhongMaterial(int n, float kd, float ks): Material(), n(n), kd(kd), ks(ks)
{

}



bool BliPhongMaterial::scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result)
{
    auto hit_point = ray_int.get_point(hit.t);
    auto normal = hit.geometry->get_normal(hit_point);
    auto reflect = L_MATH::reflect(ray_int.direction, normal);
    auto phong_pdf = std::make_shared<PhongPdf>(reflect, n);
    auto cose_pdf = std::make_shared<CosinePdf>(normal);
    std::vector<SHARE_PTR<Pdf>> pdfs = {cose_pdf, phong_pdf};
    std::vector<float> weights = {kd, ks};
    auto mix_pdf = std::make_shared<MixPdf>(pdfs, weights);
    scatter_result.pdf = mix_pdf;
    return true;
}

float BliPhongMaterial::scatter_pdf(const Ray& ray_int, const RayCasterResult& hit, const ScatterResult& scatter_result)
{
    return scatter_result.pdf->value(ray_int.direction) * (kd + ks);
}


MetalMaterial::MetalMaterial(float fuzz): fuzz(fuzz)
{
}


bool MetalMaterial::scatter(const Ray& ray_int, const RayCasterResult& hit, ScatterResult& scatter_result)
{
    auto hit_point = ray_int.get_point(hit.t);
    auto normal = hit.geometry->get_normal(hit_point);
    auto reflect = L_MATH::reflect(ray_int.direction, normal);
    auto scatter_dir = (reflect + L_MATH::random_unit_vector() * fuzz).normalize();
    scatter_result.skip_pdf = true;
    scatter_result.skip_pdf_ray = {hit_point, scatter_dir};
    return true;
}


DiffuseLightMaterial::DiffuseLightMaterial(float c_light): c_light(c_light)
{
}


float DiffuseLightMaterial::emit(const Ray& ray_int, const RayCasterResult& hit)
{
    auto hit_point = ray_int.get_point(hit.t);
    auto normal = hit.geometry->get_normal(hit_point);
    if (dot(normal, ray_int.direction) > 0)
    {
        return 0;
    }
    return c_light;
}

