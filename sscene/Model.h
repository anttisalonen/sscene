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
#include "common/Quaternion.h"

namespace Scene {

class Heightmap {
	public:
		virtual ~Heightmap() { }
		// will be called getWidth()^2 times at getXZScale() intervals
		virtual float getHeightAt(float x, float y) const = 0;
		// number of tiles to create (for both x- and y axes)
		virtual unsigned int getWidth() const = 0;
		// size per tile
		virtual float getXZScale() const = 0;
};

class Model {
	public:
		Model();
		Model(const std::string& filename);
		Model(const Heightmap& heightmap, float uscale, float vscale);

	private:
		friend class Drawable;
		const std::vector<GLfloat>& getVertexCoords() const;
		const std::vector<GLfloat>& getTexCoords() const;
		const std::vector<GLushort>& getIndices() const;
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

		const Common::Matrix44& getRotation() const;
		void setRotationFromEuler(const Common::Vector3& v);
		void setRotation(const Common::Matrix44& m);
		void setRotation(const Common::Quaternion& q);
		void setRotation(const Common::Vector3& axis, float angle);
		void setRotation(const Common::Vector3& forward, const Common::Vector3& up);
		void addRotation(const Common::Matrix44& m, bool local);
		void addRotation(const Common::Vector3& axis, float angle, bool local);
		Common::Vector3 getTargetVector() const;
		Common::Vector3 getUpVector() const;

		void setScale(float x, float y, float z);
		const Common::Vector3& getScale() const;

	protected:
		Common::Vector3 mPosition;
		Common::Matrix44 mRotation;
		Common::Vector3 mScale;
};

class Drawable;

class MeshInstance : public Movable {
	public:
		MeshInstance(const Drawable& m, bool usebackfaceculling, bool useblending);
		const Drawable& getDrawable() const;
		bool useBlending() const;
		bool useBackfaceCulling() const;

	private:
		const Drawable& mDrawable;
		bool mBackfaceCulling;
		bool mBlending;
};

}

#endif

