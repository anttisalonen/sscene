#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "common/Vector3.h"
#include "common/Matrix44.h"

namespace Scene {

class Model {
	public:
		Model();
		Model(const std::string& filename);
		const std::vector<GLfloat>& getVertexCoords() const;
		const std::vector<GLfloat>& getTexCoords() const;
		const std::vector<GLushort> getIndices() const;
		const std::vector<GLfloat>& getNormals() const;

		void addVertex(const Common::Vector3& v);
		void addNormal(const Common::Vector3& v);
		void addTexCoord(float u, float v);
		void addIndex(unsigned short i);
		void addTriangleIndices(unsigned short i1,
				unsigned short i2,
				unsigned short i3);
		void addQuadIndices(unsigned short i1,
				unsigned short i2,
				unsigned short i3,
				unsigned short i4);

	private:
		std::vector<GLfloat> mVertexCoords;
		std::vector<GLfloat> mTexCoords;
		std::vector<GLushort> mIndices;
		std::vector<GLfloat> mNormals;

		Assimp::Importer mImporter;
		const aiScene* mScene;
};

class Movable {
	public:
		Movable();
		Movable(const Common::Vector3& pos);
		void setPosition(const Common::Vector3& p);
		const Common::Vector3& getPosition() const;
		void move(const Common::Vector3& v);

	protected:
		Common::Vector3 mPosition;
};

class Drawable;

class MeshInstance : public Movable {
	public:
		MeshInstance(const Drawable& m);
		const Common::Matrix44& getRotation() const;
		void setRotationFromEuler(const Common::Vector3& v);
		void setRotation(const Common::Matrix44& m);
		const Drawable& getDrawable() const;

	private:
		const Drawable& mDrawable;
		Common::Matrix44 mRotation;
};

}

#endif

