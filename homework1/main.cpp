#include<bits/stdc++.h>

using namespace std;


void transformPoint(double x, double y, double& xOut, double& yOut) {
 
    double P_h[3] = {x, y, 1.0};

   
    double R[3][3] = {
        {cos(M_PI / 4), -sin(M_PI / 4), 0.0},
        {sin(M_PI / 4),  cos(M_PI / 4), 0.0},
        {0.0,            0.0,           1.0}
    };

 
    double T[3][3] = {
        {1.0, 0.0, 1.0},
        {0.0, 1.0, 2.0},
        {0.0, 0.0, 1.0}
    };

    
    double R_P_h[3] = {0.0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            R_P_h[i] += R[i][j] * P_h[j];
        }
    }

   
    double T_R_P_h[3] = {0.0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            T_R_P_h[i] += T[i][j] * R_P_h[j];
        }
    }

 
    xOut = T_R_P_h[0];
    yOut = T_R_P_h[1];
}

int main() {
 
    double x = 2.0, y = 1.0;
    double xTrans, yTrans;

   
    transformPoint(x, y, xTrans, yTrans);


    cout << "start location = (" << x << ", " << y << ")" << endl;
    cout << "transfomed location = (" << fixed << setprecision(4) 
         << xTrans << ", " << yTrans << ")" << endl;

    return 0;
}
