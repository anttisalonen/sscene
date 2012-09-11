#include "Model.h"

#include <stdexcept>
#include <iostream>

#include "HelperFunctions.h"

using namespace Common;
using namespace Scene;

namespace Scene {

Model::Model(const std::string& filename)
{
	mScene = mImporter.ReadFile(filename,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);
	if(!mScene) {
		std::cerr << "Unable to load model from " << filename << "\n";
		throw std::runtime_error("Error while loading model");
	}
	if(mScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !mScene->mNumMeshes) {
		std::cerr << "Model file " << filename << " is incomplete\n";
		throw std::runtime_error("Error while loading model");
	}

	aiMesh* mesh = mScene->mMeshes[0];
	if(!mesh->HasTextureCoords(0) || mesh->GetNumUVChannels() != 1) {
		std::cerr << "Model file " << filename << " has unsupported texture coordinates.\n";
		throw std::runtime_error("Error while loading model");
	}

	std::cout << mesh->mNumVertices << " vertices.\n";
	std::cout << mesh->mNumFaces << " faces.\n";

	for(int i = 0; i < mesh->mNumVertices; i++) {
		const aiVector3D& vertex = mesh->mVertices[i];
		mVertexCoords.push_back(vertex.x);
		mVertexCoords.push_back(vertex.y);
		mVertexCoords.push_back(vertex.z);

		const aiVector3D& texcoord = mesh->mTextureCoords[0][i];
		mTexCoords.push_back(texcoord.x);
		mTexCoords.push_back(texcoord.y);

		if(mesh->HasNormals()) {
			const aiVector3D& normal = mesh->mNormals[i];
			mNormals.push_back(normal.x);
			mNormals.push_back(normal.y);
			mNormals.push_back(normal.z);
		}
	}

	for(int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		if(face.mNumIndices != 3) {
			std::cerr << "Warning: number of indices should be three.\n";
			throw std::runtime_error("Error while loading model");
		} else {
			for(int j = 0; j < face.mNumIndices; j++) {
				mIndices.push_back(face.mIndices[j]);
			}
		}
	}
}

Model::Model()
{
}

void Model::addVertex(const Common::Vector3& v)
{
	mVertexCoords.push_back(v.x);
	mVertexCoords.push_back(v.y);
	mVertexCoords.push_back(v.z);
}

void Model::addNormal(const Common::Vector3& v)
{
	mNormals.push_back(v.x);
	mNormals.push_back(v.y);
	mNormals.push_back(v.z);
}

void Model::addTexCoord(float u, float v)
{
	mTexCoords.push_back(u);
	mTexCoords.push_back(v);
}

void Model::addIndex(unsigned short i)
{
	mIndices.push_back(i);
}

void Model::addTriangleIndices(unsigned short i1,
		unsigned short i2,
		unsigned short i3)
{
	mIndices.push_back(i3);
	mIndices.push_back(i2);
	mIndices.push_back(i1);
}

void Model::addQuadIndices(unsigned short i1,
		unsigned short i2,
		unsigned short i3,
		unsigned short i4)
{
	addTriangleIndices(i1, i2, i3);
	addTriangleIndices(i1, i3, i4);
}

const std::vector<GLfloat>& Model::getVertexCoords() const
{
	return mVertexCoords;
}

const std::vector<GLfloat>& Model::getTexCoords() const
{
	return mTexCoords;
}

const std::vector<GLushort> Model::getIndices() const
{
	return mIndices;
}

const std::vector<GLfloat>& Model::getNormals() const
{
	return mNormals;
}

Movable::Movable()
{
}

Movable::Movable(const Common::Vector3& pos)
	: mPosition(pos)
{
}

void Movable::setPosition(const Common::Vector3& p)
{
	mPosition = p;
}

const Common::Vector3& Movable::getPosition() const
{
	return mPosition;
}

void Movable::move(const Common::Vector3& v)
{
	mPosition += v;
}

const Matrix44& Movable::getRotation() const
{
	return mRotation;
}

void Movable::setRotationFromEuler(const Vector3& v)
{
	mRotation = HelperFunctions::rotationMatrixFromEuler(v);
}

void Movable::setRotation(const Matrix44& m)
{
	mRotation = m;
}

void Movable::setRotation(const Common::Quaternion& q)
{
	const float& x = q.x;
	const float& y = q.y;
	const float& z = q.z;
	const float& w = q.w;

	float Nq = w * w + x * x + y * y + z * z;
	float s = Nq > 0.0f ? 2.0f / Nq : 0.0f;

	float X = x * s;
	float Y = y * s;
	float Z = z * s;
	float wX = w * X; float wY = w * Y; float wZ = w * Z;
	float xX = x * X; float xY = x * Y; float xZ = x * Z;
	float yY = y * Y; float yZ = y * Z;
	float zZ = z * Z;

	mRotation.m[0] = 1.0f - yY - zZ;
	mRotation.m[1] = xY - wZ;
	mRotation.m[2] = xZ + wY;

	mRotation.m[4] = xY + wZ;
	mRotation.m[5] = 1.0f - xX - zZ;
	mRotation.m[6] = yZ - wX;

	mRotation.m[8] = xZ - wY;
	mRotation.m[9] = yZ + wX;
	mRotation.m[10] = 1.0f - xX - yY;
}

void Movable::setRotation(const Common::Vector3& axis, float angle)
{
	mRotation = HelperFunctions::rotationMatrixFromAxisAngle(axis, angle);
}

void Movable::setRotation(const Common::Vector3& forward, const Common::Vector3& up)
{
	Vector3 fw = forward.normalized();
	Vector3 u = up.normalized();
	Vector3 side = fw.cross(u);

	mRotation.m[0] = side.x;
	mRotation.m[1] = side.y;
	mRotation.m[2] = side.z;

	mRotation.m[4] = u.x;
	mRotation.m[5] = u.y;
	mRotation.m[6] = u.z;

	mRotation.m[8] = fw.x;
	mRotation.m[9] = fw.y;
	mRotation.m[10] = fw.z;

}

void Movable::addRotation(const Common::Matrix44& m, bool local)
{
	if(local)
		mRotation = m * mRotation;
	else
		mRotation = mRotation * m;
}

void Movable::addRotation(const Common::Vector3& axis, float angle, bool local)
{
	Matrix44 temp = HelperFunctions::rotationMatrixFromAxisAngle(axis, angle);
	addRotation(temp, local);
}


Common::Vector3 Movable::getTargetVector() const
{
	Vector3 v;
	v.x = mRotation.m[8];
	v.y = mRotation.m[9];
	v.z = mRotation.m[10];
	return v;
}

Common::Vector3 Movable::getUpVector() const
{
	Vector3 v;
	v.x = mRotation.m[4];
	v.y = mRotation.m[5];
	v.z = mRotation.m[6];
	return v;
}


MeshInstance::MeshInstance(const Drawable& m)
	: mDrawable(m)
{
}

const Drawable& MeshInstance::getDrawable() const
{
	return mDrawable;
}

}


