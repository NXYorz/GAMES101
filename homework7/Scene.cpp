//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Vector3f L_dir = Vector3f(0,0,0);
    Intersection L_dir_inter;
    float pdf_light = 0;
    sampleLight(L_dir_inter,pdf_light);

    Intersection inter = intersect(ray);
    if(!inter.happened)
        return Vector3f();
    if(inter.m->hasEmission())
    return inter.m->getEmission();

    Vector3f p = inter.coords , x = L_dir_inter.coords , wo = ray.direction;
    Vector3f ws = (x - p).normalized();

    Ray p2light(p , ws);
    Intersection p2light_inter = intersect(p2light);
    if(std::abs(p2light_inter.distance - (x - p).norm()) < 0.005) {
        Vector3f f_r = inter.m->eval(wo , ws , inter.normal);
        Vector3f emit = L_dir_inter.emit;
        L_dir = emit * f_r * dotProduct(ws , inter.normal) * dotProduct(-ws , L_dir_inter.normal) / dotProduct(x - p , x - p) / pdf_light;
    }

    Vector3f L_indir = Vector3f(0,0,0);

    if(get_random_float() > RussianRoulette)
        return L_dir;

    Vector3f wi = (inter.m->sample(wo , inter.normal)).normalized();
    Ray _ray(p , wi);
    Intersection _inter = intersect(_ray);
    if(_inter.happened && !_inter.m->hasEmission()) {
        Vector3f tmp = _inter.m->eval(wo , wi , inter.normal);
        L_indir = castRay(_ray , depth + 1) * tmp * dotProduct(wi , inter.normal) / inter.m->pdf(wo , wi , inter.normal) / RussianRoulette;
    }

    return L_indir + L_dir;
}