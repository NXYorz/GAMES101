### $\texttt{insideTriangle()}$ 实现

要判断点 $P$ 在不在三角形 $ABC$ 内，只需要判断 $S_{PAB} + S_{PBC} + S_{PAC}$ 是否等于 $S_{ABC}$，面积通过向量叉乘计算。

```cpp
static bool insideTriangle(int x, int y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Eigen::Vector3f vp(x, y , 1.0);
    Eigen::Vector3f vpa = _v[0] - vp;
    Eigen::Vector3f vpb = _v[1] - vp;
    Eigen::Vector3f vpc = _v[2] - vp;

    double s1 = 0.5 * std::abs((vpa.x() * vpb.y() - vpa.y() * vpb.x()));
    double s2 = 0.5 * std::abs((vpb.x() * vpc.y() - vpb.y() * vpc.x()));
    double s3 = 0.5 * std::abs((vpa.x() * vpc.y() - vpa.y() * vpc.x()));
    double S_ = s1 + s2 + s3;
    Eigen::Vector3f vac = _v[2] - _v[0];
    Eigen::Vector3f vab = _v[1] - _v[0];
    double S = 0.5 * std::abs((vab.x() * vac.y() - vab.y() * vac.x()));
    if(std::abs(S - S_) > 0.01) return false;
    return true;
}
```

### $\texttt{rasterize-triangle}$ 实现

执行三角形栅格化算法，首先判断 $\texttt{bounding box}$ 的边界，通过注释里给的求解插值深度值，与深度缓冲区中的相应值比较，如果当前节点更靠近相机，更近缓冲区与颜色。

```cpp
//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
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
                // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                depth_buf[get_index(x , y)] = z_interpolated;
                set_pixel(Eigen::Vector3f{(float)x , (float)y , 1.0} , t.getColor());
            }
        }
}
```

### $\texttt{Update main.cpp}$

上次作业三角形倒着显示，增加透视矩阵使其正显示。


### 使用 $\texttt{super-sampling}$ 处理抗锯齿

待补