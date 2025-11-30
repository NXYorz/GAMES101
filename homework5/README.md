# 说明

结果保存在 ./Code/image 文件夹下面
实现了 **光线生成** 与 **光线与三角形相交**


---------

### $\texttt{Render()}$

```cpp
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = std::tan(deg2rad(scene.fov * 0.5f));
    float imageAspectRatio = scene.width / (float)scene.height;

    // Use this variable as the eye position to start your rays.
    Vector3f eye_pos(0);
    int m = 0;
    for (int j = 0; j < scene.height; ++j)
    {
        for (int i = 0; i < scene.width; ++i)
        {
            // generate primary ray direction
            float x;
            float y;
            // TODO: Find the x and y positions of the current pixel to get the direction
            // vector that passes through it.
            x = 2 * (((float)i + 0.5) / scene.width) - 1;
            y = -1 * (2 * (((float)j + 0.5) / scene.height) - 1);
            // Also, don't forget to multiply both of them with the variable *scale*, and
            // x (horizontal) variable with the *imageAspectRatio*            
            x *= scale;
            y *= scale;
            x *= imageAspectRatio;
            Vector3f dir = Vector3f(x, y, -1); // Don't forget to normalize this direction!
            framebuffer[m++] = castRay(eye_pos, dir, scene, 0);
        }
        UpdateProgress(j / (float)scene.height);
    }
```


----------

### $\texttt{rayTriangleIntersect()}$

```cpp
bool rayTriangleIntersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Vector3f& orig,
                          const Vector3f& dir, float& tnear, float& u, float& v)
{
    // TODO: Implement this function that tests whether the triangle
    // that's specified bt v0, v1 and v2 intersects with the ray (whose
    // origin is *orig* and direction is *dir*)
    // Also don't forget to update tnear, u and v.
    Vector3f E1 = v1 - v0 , E2 = v2 - v0 , S = orig - v0;
    Vector3f S1 = crossProduct(dir , E2) , S2 = crossProduct(S , E1);
    tnear = dotProduct(S2 , E2) / dotProduct(S1 , E1);
    u = dotProduct(S1 , S) / dotProduct(S1 , E1);
    v = dotProduct(S2 , dir) / dotProduct(S1 , E1);
    return tnear > 0 && u > 0 && v > 0 && (1 - u - v) > 0;
}
```