#include "TransformController.h"
#include "mtlm.h"

#include <simd/simd.h>

using namespace Rendering;
using namespace simd;

TransformController::TransformController()
    : mZoom{5.0f},
      mPosition{0.0f, 0.0f, mZoom},
      mDirection{0.0f, 0.0f, -1.0f},
      mUp{0.0f, 1.0f, 0.0f},
      mView(mtlm::identity()),
      mProjection(mtlm::identity()),
      mRotate(mtlm::identity()),
      mTransform(mtlm::identity())
{
    updateView();
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::setPosition(simd::float3 position)
{
    mPosition = position;
    updateView();
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::setDirection(simd::float3 direction)
{
    mDirection = simd::normalize(direction);
    updateView();
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::setUp(simd::float3 up)
{
    mUp = simd::normalize(up);
    updateView();
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::setPerspective(float fovy, float aspect, float near, float far)
{
    mProjection = mtlm::perspective_projection(fovy, aspect, near, far);
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::updateView()
{
    auto right = simd::normalize(simd::cross(mDirection, mUp));
    auto up = simd::cross(right, mDirection);

    mView = mtlm::camera_view(right, up, mDirection, mPosition);
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::update()
{
    mTransform = mProjection * mView * mRotate;
}
//----------------------------------------------------------------------------------------------------------------------


const simd::float4x4& TransformController::viewMatrix() const
{
    return mView;
}
//----------------------------------------------------------------------------------------------------------------------

const simd::float4x4& TransformController::projectionMatrix() const
{
    return mProjection;
}
//----------------------------------------------------------------------------------------------------------------------

const simd::float4x4& TransformController::pvmMatrix() const
{
    return mTransform;
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::rotX(float dPhi)
{
    mRotate = mtlm::rotation(simd::float3{1, 0, 0}, dPhi) * mRotate;
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::rotY(float dPhi)
{
    mRotate = mtlm::rotation(simd::float3{0, 1, 0}, dPhi) * mRotate;
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::rotZ(float dPhi)
{
    mRotate = mtlm::rotation(simd::float3{0, 0, 1}, dPhi) * mRotate;
    update();
}
//----------------------------------------------------------------------------------------------------------------------

void TransformController::zoom(float dz)
{
    mZoom += dz;
    setPosition(float3{0.f, 0.f, mZoom});
    update();
}
//----------------------------------------------------------------------------------------------------------------------
