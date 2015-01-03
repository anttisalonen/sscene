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

	for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
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

	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace& face = mesh->mFaces[i];
		if(face.mNumIndices != 3) {
			std::cerr << "Warning: number of indices should be three.\n";
			throw std::runtime_error("Error while loading model");
		} else {
			for(unsigned int j = 0; j < face.mNumIndices; j++) {
				mIndices.push_back(face.mIndices[j]);
			}
		}
	}
}

Model::Model()
{
}

Model::Model(const Heightmap& heightmap, float uscale, float vscale)
{
	unsigned int w = heightmap.getWidth() + 1;
	float xzscale = heightmap.getXZScale();

	for(int j = 0; j < w; j++) {
		for(int i = 0; i < w; i++) {
			float xp = xzscale * i;
			float yp = xzscale * j;
			addVertex(Vector3(xp, heightmap.getHeightAt(xp, yp), yp));
			addTexCoord(uscale * i / (float)w, vscale * j / (float)w);
			Vector3 p1(xp,
					heightmap.getHeightAt(xp, yp),
					yp);
			Vector3 p2(xp + xzscale,
					heightmap.getHeightAt(xp + xzscale, yp),
					yp);
			Vector3 p3(xp,
					heightmap.getHeightAt(xp, yp + xzscale),
					yp + xzscale);
			Vector3 u(p2 - p1);
			Vector3 v(p3 - p1);
			addNormal(v.cross(u).normalized());
		}
	}

	for(int j = 0; j < w - 1; j++) {
		for(int i = 0; i < w - 1; i++) {
			addQuadIndices(j * w + i,
					j * w + i + 1,
					(j + 1) * w + i + 1,
					(j + 1) * w + i);
		}
	}
}

Model::Model(const std::vector<Common::Vector3>& vertexcoords,
		const std::vector<Common::Vector2>& texcoords,
		const std::vector<unsigned int>& indices,
		const std::vector<Common::Vector3>& normals)
{
	for(auto v : vertexcoords)
		addVertex(v);
	for(auto v : texcoords)
		addTexCoord(v.x, v.y);
	for(auto v : indices)
		addIndex(v);
	for(auto v : normals)
		addNormal(v);
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

const std::vector<GLushort>& Model::getIndices() const
{
	return mIndices;
}

const std::vector<GLfloat>& Model::getNormals() const
{
	return mNormals;
}

Movable::Movable()
	: mScale(1.0f, 1.0f, 1.0f)
{
}

Movable::Movable(const Common::Vector3& pos)
	: mPosition(pos),
	mScale(1.0f, 1.0f, 1.0f)
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
	float x, y, z;
	q.toEuler(x, y, z);
	setRotationFromEuler(Common::Vector3(x, y, z));
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

void Movable::setScale(float x, float y, float z)
{
	mScale.x = x;
	mScale.y = y;
	mScale.z = z;
}

const Common::Vector3& Movable::getScale() const
{
	return mScale;
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


MeshInstance::MeshInstance(const Drawable& m, bool usebackfaceculling, bool useblending)
	: mDrawable(m),
	mBackfaceCulling(usebackfaceculling),
	mBlending(useblending)
{
}

const Drawable& MeshInstance::getDrawable() const
{
	return mDrawable;
}

bool MeshInstance::useBackfaceCulling() const
{
	return mBackfaceCulling;
}

bool MeshInstance::useBlending() const
{
	return mBlending;
}

}


