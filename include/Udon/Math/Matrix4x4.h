#ifndef MATRIX4X4_H
#define MATRIX4X4_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL



class Matrix4x4 {
public:
    float Values[16];

    static Matrix4x4* Create();
    static void Perspective(Matrix4x4* out, float fovy, float aspect, float near, float far);
    static void Ortho(Matrix4x4* out, float left, float right, float bottom, float top, float near, float far);
    static void Copy(Matrix4x4* out, Matrix4x4* a);
    static void Translate(Matrix4x4* out, Matrix4x4* a, float x, float y, float z);
    static void Scale(Matrix4x4* out, Matrix4x4* a, float x, float y, float z);
    static void Rotate(Matrix4x4* out, Matrix4x4* a, float rad, float x, float y, float z);
    static void Print(Matrix4x4* out);
};

#endif /* MATRIX4X4_H */
