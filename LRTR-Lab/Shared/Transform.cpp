#include "Transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

LRTR::Transform::Transform(const Matrix4x4f& transform) :
	mTransform(transform), mInverse(glm::inverse(mTransform))
{

}

LRTR::Transform::Transform(const Matrix4x4f& transform, const Matrix4x4f& inverse) :
	mTransform(transform), mInverse(inverse)
{

}

LRTR::Transform::Transform(const Vector3f& translation, const Vector4f& rotation, const Vector3f& scale)
{
	//T * R * S
	const auto I = Matrix4x4f(1);
	const auto T = glm::translate(I, translation);
	const auto R = Vector3f(rotation) == Vector3f(0) ? I : glm::rotate(I, rotation.w, Vector3f(rotation));
	const auto S = glm::scale(I, scale);
	
	mTransform = T * R * S;
	mInverse = glm::inverse(mTransform);
}

LRTR::Transform::Transform(const Vector3f& translation, const QuaternionF& rotation, const Vector3f& scale)
{
	//T * R * S
	const auto I = Matrix4x4f(1);
	const auto T = glm::translate(I, translation);
	const auto R = glm::mat4_cast(rotation);
	const auto S = glm::scale(I, scale);

	mTransform = T * R * S;
	mInverse = glm::inverse(mTransform);
}

LRTR::Transform& LRTR::Transform::operator*(const Transform& right)
{
	mTransform = mTransform * right.mTransform;
	mInverse = right.mInverse * mInverse;

	return *this;
}

auto LRTR::Transform::matrix() const noexcept -> Matrix4x4f
{
	return mTransform;
}

auto LRTR::Transform::inverseMatrix() const noexcept -> Matrix4x4f
{
	return mInverse;
}

auto LRTR::Transform::inverse(const Transform& transform) -> Transform
{
	return Transform(transform.mInverse, transform.mTransform);
}

auto LRTR::Transform::translate(const Vector3f& delta) -> Transform
{
	return Transform(
		glm::translate(glm::mat4(1), delta),
		glm::translate(glm::mat4(1), -delta)
	);
}

auto LRTR::Transform::rotate(Real angle, const Vector3f& axis) -> Transform
{
	const auto transform = glm::rotate(glm::mat4(1), angle, axis);

	return Transform(transform, glm::transpose(transform));
}

auto LRTR::Transform::scale(const Vector3f& value) -> Transform
{
	return Transform(
		glm::scale(glm::mat4(1), value),
		glm::scale(glm::mat4(1), 1.0f / value)
	);
}

auto LRTR::Transform::lookAt(const Vector3f& eye, const Vector3f& at, const Vector3f& up) -> Transform
{
	return Transform(glm::lookAtRH(eye, at, up));
}

auto LRTR::Transform::perspectiveFov(Real fovy, Real width, Real height, Real zNear, Real zFar) -> Transform
{
	return Transform(glm::perspectiveFovRH(fovy, width, height, zNear, zFar));
}

auto LRTR::Transform::ortho(Real left, Real right, Real bottom, Real top, Real zNear, Real zFar) -> Transform
{
	return Transform(glm::orthoRH_ZO(left, right, bottom, top, zNear, zFar));
}
