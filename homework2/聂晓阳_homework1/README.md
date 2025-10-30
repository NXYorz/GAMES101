### get martix rotate with z

```cpp
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << std::cos(rotation_angle / 180.0 * MY_PI), -std::sin(rotation_angle / 180.0 * MY_PI), 0 ,0 ,
    std::sin(rotation_angle / 180.0 * MY_PI), std::cos(rotation_angle / 180.0 * MY_PI) , 0, 0,
    0, 0, 1, 0, 0, 0, 0, 1;
    model = translate * model;

    return model;
}
```

### get projection matrix

```cpp
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1.0 / (std::tan(eye_fov / 360.0 * MY_PI)*aspect_ratio) , 0,0,0, 0, 1.0 / std::tan(eye_fov / 360.0 * MY_PI),0,0,
    0,0,(zNear + zFar) / (zNear - zFar), -2 * zNear * zFar / (zNear - zFar),
    0,0,1,0;
    projection = translate * projection;
    return projection;
}
```

### get rotate matrix with any axis

Get the martix by calculate the Rotation Formula. 

```cpp
Eigen::Matrix4f get_rotation(Vector3f axis , float rotation_angle)
{
    Eigen::Matrix3f martix = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f translate;
    translate << 0, -axis[2] , axis[1], axis[2] , 0, axis[0] , -axis[1] , axis[0] , 0;
    martix = std::cos(rotation_angle / 180.0 * MY_PI) * martix + (1.0 - std::cos(rotation_angle / 180.0 * MY_PI)) * axis * axis.transpose() + std::sin(rotation_angle / 180.0 * MY_PI) * translate;
    Eigen::Matrix4f ans = Eigen::Matrix4f::Identity();
    ans.block(0,0,3,3) = martix;
    return ans;
}
```