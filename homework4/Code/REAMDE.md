 
### $\texttt{rasterize triangle}$

```cpp
void rst::rasterizer::rasterize_triangle(const Triangle& t, const std::array<Eigen::Vector3f, 3>& view_pos) 
{
    // TODO: From your HW3, get the triangle rasterization code.
    // TODO: Inside your rasterization loop:
    //    * v[i].w() is the vertex view space depth value z.
    //    * Z is interpolated view space depth for the current pixel
    //    * zp is depth between zNear and zFar, used for z-buffer

    // float Z = 1.0 / (alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    // float zp = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    // zp *= Z;

    // TODO: Interpolate the attributes:
    auto v = t.toVector4();
    float min_x = std::floor(std::min(v[0][0], std::min(v[1][0], v[2][0])));
    float min_y = std::floor(std::min(v[0][1], std::min(v[1][1], v[2][1])));
    float max_x = std::ceil(std::max(v[0][0], std::max(v[1][0], v[2][0])));
    float max_y = std::ceil(std::max(v[0][1], std::max(v[1][1], v[2][1])));
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    for(int i = min_x; i <= max_x; i++) 
        for(int j = min_y; j <= max_y; j++) {
            if(!insideTriangle(i , j , t.v)) continue;
            // If so, use the following code to get the interpolated z value.
            int x = i , y = j;
            auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
            float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            z_interpolated *= w_reciprocal;
            if(z_interpolated < depth_buf[get_index(x , y)]) {

                auto interpolated_color = alpha * t.color[0] + beta * t.color[1] + gamma * t.color[2];
                auto interpolated_normal = alpha * t.normal[0] + beta * t.normal[1] + gamma * t.normal[2];
                auto interpolated_texcoords = alpha * t.tex_coords[0] + beta * t.tex_coords[1] + gamma * t.tex_coords[2];
                auto interpolated_shadingcoords = alpha * view_pos[0] + beta * view_pos[1] + gamma * view_pos[2];

                fragment_shader_payload payload( interpolated_color, interpolated_normal.normalized(), interpolated_texcoords, texture ? &*texture : nullptr);
                payload.view_pos = interpolated_shadingcoords;
                //Use: Instead of passing the triangle's color directly to the frame buffer, pass the color to the shaders first to get the final color;
                auto pixel_color = fragment_shader(payload);
                depth_buf[get_index(x , y)] = z_interpolated;
                set_pixel(Eigen::Vector2i{(float)x , (float)y} , pixel_color);
            }
        }
}
```

### $\texttt{texture fragment shader}$
```cpp
Eigen::Vector3f texture_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f return_color = {0, 0, 0};
    if (payload.texture)
    {
        // TODO: Get the texture value at the texture coordinates of the current fragment
        return_color = payload.texture->getColor(payload.tex_coords.x(), payload.tex_coords.y());

    }
    Eigen::Vector3f texture_color;
    texture_color << return_color.x(), return_color.y(), return_color.z();

    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = texture_color / 255.f;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = texture_color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_dir = (light.position - point).normalized();
        Eigen::Vector3f view_dir = (eye_pos - point).normalized();
        Eigen::Vector3f half_dir = (light_dir + view_dir).normalized();

        Eigen::Vector3f La = ka.cwiseProduct(amb_light_intensity);

        float r2 = (light.position - point).dot(light.position - point);
        Eigen::Vector3f I_r2 = light.intensity / r2;

        Eigen::Vector3f Ld = kd.cwiseProduct(I_r2);
        Ld *= std::max(0.0f, normal.normalized().dot(light_dir));

        Eigen::Vector3f Ls = ks.cwiseProduct(I_r2);
        Ls *= std::pow(std::max(0.0f, normal.normalized().dot(half_dir)), p);

        result_color += (La + Ld + Ls);

    }

    return result_color * 255.f;
}
```

### $\texttt{phong fragment shader}$

```cpp
Eigen::Vector3f phong_fragment_shader(const fragment_shader_payload& payload)
{
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color;
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    Eigen::Vector3f result_color = {0, 0, 0};
    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.

        Eigen::Vector3f light_dir = (light.position - point).normalized();
        Eigen::Vector3f view_dir = (eye_pos - point).normalized();
        Eigen::Vector3f half_dir = (light_dir + view_dir).normalized();

        Eigen::Vector3f La = ka.cwiseProduct(amb_light_intensity);

        float r2 = (light.position - point).dot(light.position - point);
        Eigen::Vector3f I_r2 = light.intensity / r2;

        Eigen::Vector3f Ld = kd.cwiseProduct(I_r2);
        Ld *= std::max(0.0f, normal.normalized().dot(light_dir));

        Eigen::Vector3f Ls = ks.cwiseProduct(I_r2);
        Ls *= std::pow(std::max(0.0f, normal.normalized().dot(half_dir)), p);

        result_color += (La + Ld + Ls);
        
    }

    return result_color * 255.f;
}
```

### $\texttt{displacement fragment shader}$
```cpp
Eigen::Vector3f displacement_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;

    float kh = 0.2, kn = 0.1;
    
    // TODO: Implement displacement mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Position p = p + kn * n * h(u,v)
    // Normal n = normalize(TBN * ln)

    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    Eigen::Vector3f t = Eigen::Vector3f(x * y / std::sqrt(x * x + z * z), std::sqrt(x * x + z * z), z * y / std::sqrt(x * x + z * z));
    Eigen::Vector3f b = normal.cross(t);

    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
        t.y(), b.y(), normal.y(),
        t.z(), b.z(), normal.z();

    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    float w = payload.texture->width;
    float h = payload.texture->height;

    float dU = kh * kn * (payload.texture->getColor(u + 1.0f / w, v).norm() - payload.texture->getColor(u, v).norm());
    float dV = kh * kn * (payload.texture->getColor(u, v + 1.0f / h).norm() - payload.texture->getColor(u, v).norm());

    Eigen::Vector3f ln = Eigen::Vector3f(-dU, -dV, 1.0f);

    point += (kn * normal * payload.texture->getColor(u, v).norm());

    normal = (TBN * ln).normalized();

    Eigen::Vector3f result_color = {0, 0, 0};

    for (auto& light : lights)
    {
        // TODO: For each light source in the code, calculate what the *ambient*, *diffuse*, and *specular* 
        // components are. Then, accumulate that result on the *result_color* object.
        Eigen::Vector3f light_dir = (light.position - point).normalized();
        Eigen::Vector3f view_dir = (eye_pos - point).normalized();
        Eigen::Vector3f half_dir = (light_dir + view_dir).normalized();

        Eigen::Vector3f La = ka.cwiseProduct(amb_light_intensity);

        float r2 = (light.position - point).dot(light.position - point);
        Eigen::Vector3f I_r2 = light.intensity / r2;

        Eigen::Vector3f Ld = kd.cwiseProduct(I_r2);
        Ld *= std::max(0.0f, normal.normalized().dot(light_dir));

        Eigen::Vector3f Ls = ks.cwiseProduct(I_r2);
        Ls *= std::pow(std::max(0.0f, normal.normalized().dot(half_dir)), p);

        result_color += (La + Ld + Ls);


    }

    return result_color * 255.f;
}
```

### $\texttt{bump fragment shader}$

```cpp
Eigen::Vector3f bump_fragment_shader(const fragment_shader_payload& payload)
{
    
    Eigen::Vector3f ka = Eigen::Vector3f(0.005, 0.005, 0.005);
    Eigen::Vector3f kd = payload.color;
    Eigen::Vector3f ks = Eigen::Vector3f(0.7937, 0.7937, 0.7937);

    auto l1 = light{{20, 20, 20}, {500, 500, 500}};
    auto l2 = light{{-20, 20, 0}, {500, 500, 500}};

    std::vector<light> lights = {l1, l2};
    Eigen::Vector3f amb_light_intensity{10, 10, 10};
    Eigen::Vector3f eye_pos{0, 0, 10};

    float p = 150;

    Eigen::Vector3f color = payload.color; 
    Eigen::Vector3f point = payload.view_pos;
    Eigen::Vector3f normal = payload.normal;


    float kh = 0.2, kn = 0.1;

    // TODO: Implement bump mapping here
    // Let n = normal = (x, y, z)
    // Vector t = (x*y/sqrt(x*x+z*z),sqrt(x*x+z*z),z*y/sqrt(x*x+z*z))
    // Vector b = n cross product t
    // Matrix TBN = [t b n]
    // dU = kh * kn * (h(u+1/w,v)-h(u,v))
    // dV = kh * kn * (h(u,v+1/h)-h(u,v))
    // Vector ln = (-dU, -dV, 1)
    // Normal n = normalize(TBN * ln)

    float x = normal.x();
    float y = normal.y();
    float z = normal.z();

    Eigen::Vector3f t = Eigen::Vector3f(x * y / std::sqrt(x * x + z * z), std::sqrt(x * x + z * z), z * y / std::sqrt(x * x + z * z));
    Eigen::Vector3f b = normal.cross(t);

    Eigen::Matrix3f TBN;
    TBN << t.x(), b.x(), normal.x(),
        t.y(), b.y(), normal.y(),
        t.z(), b.z(), normal.z();

    float u = payload.tex_coords.x();
    float v = payload.tex_coords.y();
    float w = payload.texture->width;
    float h = payload.texture->height;

    float dU = kh * kn * (payload.texture->getColor(u + 1.0f / w, v).norm() - payload.texture->getColor(u, v).norm());
    float dV = kh * kn * (payload.texture->getColor(u, v + 1.0f / h).norm() - payload.texture->getColor(u, v).norm());

    Eigen::Vector3f ln = Eigen::Vector3f(-dU, -dV, 1.0f);
    normal = TBN * ln;

    Eigen::Vector3f result_color = {0, 0, 0};
    result_color = normal.normalized();

    return result_color * 255.f;
}
```