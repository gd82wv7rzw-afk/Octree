#include "Camera.h"
#include "mtlm.h"

#include <simd/simd.h>

using namespace Rendering;

Camera::Camera()
    : mPosition{0.0f, 0.0f, 5.0f},
      mDirection{0.0f, 0.0f, -1.0f},
      mUp{0.0f, 1.0f, 0.0f},
      mView(mtlm::identity()),
      mProjection(mtlm::identity())
{
    updateView();
}
//----------------------------------------------------------------------------------------------------------------------

void Camera::setPosition(simd::float3 position)
{
    mPosition = position;
    updateView();
}
//----------------------------------------------------------------------------------------------------------------------

void Camera::setDirection(simd::float3 direction)
{
    mDirection = simd::normalize(direction);
    updateView();
}
//----------------------------------------------------------------------------------------------------------------------

void Camera::setUp(simd::float3 up)
{
    mUp = simd::normalize(up);
    updateView();
}
//----------------------------------------------------------------------------------------------------------------------

void Camera::setPerspective(float fovy, float aspect, float near, float far)
{
    mProjection = mtlm::perspective_projection(fovy, aspect, near, far);
}
//----------------------------------------------------------------------------------------------------------------------

void Camera::updateView()
{
    auto right = simd::normalize(simd::cross(mDirection, mUp));
    auto up = simd::cross(right, mDirection);

    mView = mtlm::camera_view(right, up, mDirection, mPosition);
}
//----------------------------------------------------------------------------------------------------------------------

const simd::float4x4& Camera::viewMatrix() const
{
    return mView;
}
//----------------------------------------------------------------------------------------------------------------------

const simd::float4x4& Camera::projectionMatrix() const
{
    return mProjection;
}