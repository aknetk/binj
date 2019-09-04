#if INTERFACE
class Matrix4x4 {
public:
    float Values[16];
};
#endif

#include <Udon/Math/Matrix4x4.h>

#include <Udon/Logging/Log.h>

#include <math.h>
#include <string.h>

#define EPSILON 0.000001

PUBLIC STATIC Matrix4x4* Matrix4x4::Create() {
    Matrix4x4* mat4 = new Matrix4x4;
    mat4->Values[0] = 1.0f;
    mat4->Values[1] = 0.0f;
    mat4->Values[2] = 0.0f;
    mat4->Values[3] = 0.0f;
    mat4->Values[4] = 0.0f;
    mat4->Values[5] = 1.0f;
    mat4->Values[6] = 0.0f;
    mat4->Values[7] = 0.0f;
    mat4->Values[8] = 0.0f;
    mat4->Values[9] = 0.0f;
    mat4->Values[10] = 1.0f;
    mat4->Values[11] = 0.0f;
    mat4->Values[12] = 0.0f;
    mat4->Values[13] = 0.0f;
    mat4->Values[14] = 0.0f;
    mat4->Values[15] = 1.0f;
    return mat4;
}

PUBLIC STATIC void Matrix4x4::Perspective(Matrix4x4* out, float fovy, float aspect, float near, float far) {
    float f = 1.0f / tan(fovy / 2.0f);
    float nf = 1.0f / (near - far);
    out->Values[0]  = f / aspect;
    out->Values[1]  = 0.0f;
    out->Values[2]  = 0.0f;
    out->Values[3]  = 0.0f;
    out->Values[4]  = 0.0f;
    out->Values[5]  = f;
    out->Values[6]  = 0.0f;
    out->Values[7]  = 0.0f;
    out->Values[8]  = 0.0f;
    out->Values[9]  = 0.0f;
    out->Values[10] = (far + near) * nf;
    out->Values[11] = -1.0f;
    out->Values[12] = 0.0f;
    out->Values[13] = 0.0f;
    out->Values[14] = 2.0f * far * near * nf;
    out->Values[15] = 0.0f;
}
PUBLIC STATIC void Matrix4x4::Ortho(Matrix4x4* out, float left, float right, float bottom, float top, float near, float far) {
    float lr = 1.0f / (left - right);
    float bt = 1.0f / (bottom - top);
    float nf = 1.0f / (near - far);
    out->Values[0]  = -2.0f * lr;
    out->Values[1]  = 0.0f;
    out->Values[2]  = 0.0f;
    out->Values[3]  = 0.0f;
    out->Values[4]  = 0.0f;
    out->Values[5]  = -2.0f * bt;
    out->Values[6]  = 0.0f;
    out->Values[7]  = 0.0f;
    out->Values[8]  = 0.0f;
    out->Values[9]  = 0.0f;
    out->Values[10] = 2.0f * nf;
    out->Values[11] = 0.0f;
    out->Values[12] = (left + right) * lr;
    out->Values[13] = (top + bottom) * bt;
    out->Values[14] = (far + near) * nf;
    out->Values[15] = 1.0f;
}

PUBLIC STATIC void Matrix4x4::Copy(Matrix4x4* out, Matrix4x4* a) {
    memcpy(&out->Values[0], &a->Values[0], 16 * sizeof(float));
}

PUBLIC STATIC void Matrix4x4::Translate(Matrix4x4* out, Matrix4x4* a, float x, float y, float z) {
    float a00, a01, a02, a03;
    float a10, a11, a12, a13;
    float a20, a21, a22, a23;

    if (a == out) {
        out->Values[12] = a->Values[0] * x + a->Values[4] * y + a->Values[8] * z + a->Values[12];
        out->Values[13] = a->Values[1] * x + a->Values[5] * y + a->Values[9] * z + a->Values[13];
        out->Values[14] = a->Values[2] * x + a->Values[6] * y + a->Values[10] * z + a->Values[14];
        out->Values[15] = a->Values[3] * x + a->Values[7] * y + a->Values[11] * z + a->Values[15];
    }
    else {
        a00 = a->Values[0]; a01 = a->Values[1]; a02 = a->Values[2];  a03 = a->Values[3];
        a10 = a->Values[4]; a11 = a->Values[5]; a12 = a->Values[6];  a13 = a->Values[7];
        a20 = a->Values[8]; a21 = a->Values[9]; a22 = a->Values[10]; a23 = a->Values[11];
        //
        // out->Values[0] = a00; out->Values[1] = a01; out->Values[2] = a02;  out->Values[3] = a03;
        // out->Values[4] = a10; out->Values[5] = a11; out->Values[6] = a12;  out->Values[7] = a13;
        // out->Values[8] = a20; out->Values[9] = a21; out->Values[10] = a22; out->Values[11] = a23;

        // Copy the first 12 values
        memcpy(&out->Values[0], &a->Values[0], 12 * sizeof(float));

        out->Values[12] = a00 * x + a10 * y + a20 * z + a->Values[12];
        out->Values[13] = a01 * x + a11 * y + a21 * z + a->Values[13];
        out->Values[14] = a02 * x + a12 * y + a22 * z + a->Values[14];
        out->Values[15] = a03 * x + a13 * y + a23 * z + a->Values[15];
    }
}
PUBLIC STATIC void Matrix4x4::Scale(Matrix4x4* out, Matrix4x4* a, float x, float y, float z) {
    out->Values[0]  = a->Values[0] * x;
    out->Values[1]  = a->Values[1] * x;
    out->Values[2]  = a->Values[2] * x;
    out->Values[3]  = a->Values[3] * x;
    out->Values[4]  = a->Values[4] * y;
    out->Values[5]  = a->Values[5] * y;
    out->Values[6]  = a->Values[6] * y;
    out->Values[7]  = a->Values[7] * y;
    out->Values[8]  = a->Values[8] * z;
    out->Values[9]  = a->Values[9] * z;
    out->Values[10] = a->Values[10] * z;
    out->Values[11] = a->Values[11] * z;
    out->Values[12] = a->Values[12];
    out->Values[13] = a->Values[13];
    out->Values[14] = a->Values[14];
    out->Values[15] = a->Values[15];
}
PUBLIC STATIC void Matrix4x4::Rotate(Matrix4x4* out, Matrix4x4* a, float rad, float x, float y, float z) {
    float len = sqrt(x * x + y * y + z * z);
    float s, c, t;
    float a00, a01, a02, a03;
    float a10, a11, a12, a13;
    float a20, a21, a22, a23;

    float b00, b01, b02;
    float b10, b11, b12;
    float b20, b21, b22;

    if (abs(len) < EPSILON) {
        return;
    }

    len = 1.0f / len;
    x *= len;
    y *= len;
    z *= len;

    s = sin(rad);
    c = cos(rad);
    t = 1.0f - c;

    a00 = a->Values[0]; a01 = a->Values[1]; a02 = a->Values[2];  a03 = a->Values[3];
    a10 = a->Values[4]; a11 = a->Values[5]; a12 = a->Values[6];  a13 = a->Values[7];
    a20 = a->Values[8]; a21 = a->Values[9]; a22 = a->Values[10]; a23 = a->Values[11];

    // Construct the elements of the rotation matrix
    b00 = x * x * t + c;     b01 = y * x * t + z * s; b02 = z * x * t - y * s;
    b10 = x * y * t - z * s; b11 = y * y * t + c;     b12 = z * y * t + x * s;
    b20 = x * z * t + y * s; b21 = y * z * t - x * s; b22 = z * z * t + c;

    // Perform rotation-specific matrix multiplication
    out->Values[0]  = a00 * b00 + a10 * b01 + a20 * b02;
    out->Values[1]  = a01 * b00 + a11 * b01 + a21 * b02;
    out->Values[2]  = a02 * b00 + a12 * b01 + a22 * b02;
    out->Values[3]  = a03 * b00 + a13 * b01 + a23 * b02;
    out->Values[4]  = a00 * b10 + a10 * b11 + a20 * b12;
    out->Values[5]  = a01 * b10 + a11 * b11 + a21 * b12;
    out->Values[6]  = a02 * b10 + a12 * b11 + a22 * b12;
    out->Values[7]  = a03 * b10 + a13 * b11 + a23 * b12;
    out->Values[8]  = a00 * b20 + a10 * b21 + a20 * b22;
    out->Values[9]  = a01 * b20 + a11 * b21 + a21 * b22;
    out->Values[10] = a02 * b20 + a12 * b21 + a22 * b22;
    out->Values[11] = a03 * b20 + a13 * b21 + a23 * b22;

    if (a != out) {
        // If the source and destination differ, copy the unchanged last row
        out->Values[12] = a->Values[12];
        out->Values[13] = a->Values[13];
        out->Values[14] = a->Values[14];
        out->Values[15] = a->Values[15];
    }
}

PUBLIC STATIC void Matrix4x4::Print(Matrix4x4* out) {
    // Log::Print(Log::LOG_INFO, "mat4 \n%f %f %f %f \n%f %f %f %f \n%f %f %f %f \n%f %f %f %f", out->Values[0], out->Values[1], out->Values[2], out->Values[3], out->Values[4], out->Values[5], out->Values[6], out->Values[7], out->Values[8], out->Values[9], out->Values[10], out->Values[11], out->Values[12], out->Values[13], out->Values[14], out->Values[15]);
}
