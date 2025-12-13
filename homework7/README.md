
I've finished task ...

---------

#### $\texttt{Path Tracing}$
```cpp
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
```

------------

#### $\texttt{在 Render 中使用多线程优化}$
```cpp
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int spp = 16;

    int num_threads = std::thread::hardware_concurrency(); 
    std::vector<std::thread> threads;

    std::mutex mutex_progress;
    int lines_complete = 0;

    std::cout << "SPP: " << spp << " Threads: " << num_threads << "\n";

    auto render_rows = [&](int start_row, int end_row) {
        for (int j = start_row; j < end_row; ++j) {
            for (int i = 0; i < scene.width; ++i) {
                float x = (2 * (i + 0.5) / (float)scene.width - 1) * imageAspectRatio * scale;
                float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
                Vector3f dir = normalize(Vector3f(-x, y, 1));

                for (int k = 0; k < spp; k++) {
                    int index = j * scene.width + i;
                    framebuffer[index] += scene.castRay(Ray(eye_pos, dir), 0) / spp;
                }
            }
            std::lock_guard<std::mutex> lock(mutex_progress);
            lines_complete++;
            UpdateProgress(lines_complete / (float)scene.height);
        }
    };

    int rows_per_thread = scene.height / num_threads;
    for (int t = 0; t < num_threads; ++t) {
        int start = t * rows_per_thread;
        int end = (t == num_threads - 1) ? scene.height : (start + rows_per_thread);
        threads.emplace_back(render_rows, start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    UpdateProgress(1.f);

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
```