#pragma once

#include <simd/simd.h>

namespace Rendering
{
class TransformController
{
    public:

        TransformController();

        void setPosition(simd::float3 position);
        void setDirection(simd::float3 direction);
        void setUp(simd::float3 up);
        //--------------------------------------------------------------------------------------------------------------

        void setPerspective(float fovy, float aspect, float near, float far);
        //--------------------------------------------------------------------------------------------------------------

        void rotX(float dPhi);
        void rotY(float dPhi);
        void rotZ(float dPhi);
        void zoom(float dz);
        //--------------------------------------------------------------------------------------------------------------

        const simd::float4x4& viewMatrix() const;
        const simd::float4x4& projectionMatrix() const;
        const simd::float4x4& pvmMatrix() const;
        //--------------------------------------------------------------------------------------------------------------

    private:

        void updateView();
        void update();
        //--------------------------------------------------------------------------------------------------------------

        float        mZoom;
        simd::float3 mPosition;
        simd::float3 mDirection;
        simd::float3 mUp;
        //--------------------------------------------------------------------------------------------------------------

        simd::float4x4 mView;
        simd::float4x4 mProjection;
        simd::float4x4 mRotate;
        simd::float4x4 mTransform;
        //--------------------------------------------------------------------------------------------------------------
};
//----------------------------------------------------------------------------------------------------------------------

}// namespace ----------------------------------------------------------------------------------------------------------