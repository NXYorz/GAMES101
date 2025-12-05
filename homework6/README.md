
I've finished task ...

#### $\texttt{IntersectP}$

```cpp
inline bool Bounds3::IntersectP(const Ray& ray, const Vector3f& invDir,
                                const std::array<int, 3>& dirIsNeg) const
{
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic
    // TODO test if ray bound intersects
    float tMin_x = (pMin.x - ray.origin.x) * invDir.x;
    float tMax_x = (pMax.x - ray.origin.x) * invDir.x;

    float tMin_y = (pMin.y - ray.origin.y) * invDir.y;
    float tMax_y = (pMax.y - ray.origin.y) * invDir.y;

    float tMin_z = (pMin.z - ray.origin.z) * invDir.z;
    float tMax_z = (pMax.z - ray.origin.z) * invDir.z;

    if(dirIsNeg[0]) std::swap(tMin_x , tMax_x);
    if(dirIsNeg[1]) std::swap(tMin_y , tMax_y);
    if(dirIsNeg[2]) std::swap(tMin_z , tMax_z);

    float t_enter = std::max(tMin_x , std::max(tMin_y , tMin_z));
    float t_exit = std::min(tMax_x , std::min(tMax_y , tMax_z));

    return (t_enter <= t_exit && t_exit >= 0);
    
}
```

#### $\texttt{getIntersection}$
```cpp
Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    Intersection res;

    std::array<int , 3> dirIsNeg;
    dirIsNeg[0] = (ray.direction.x < 0) ? 1 : 0;
    dirIsNeg[1] = (ray.direction.y < 0) ? 1 : 0;
    dirIsNeg[2] = (ray.direction.z < 0) ? 1 : 0;

    if(!node->bounds.IntersectP(ray , ray.direction_inv , dirIsNeg)) return res;

    if(node->left == nullptr && node->right == nullptr) {
        return node->object->getIntersection(ray);
    }

    Intersection hitleft = getIntersection(node->left , ray);
    Intersection hitright = getIntersection(node->right , ray);

    return (hitleft.distance < hitright.distance) ? hitleft : hitright;
}
```