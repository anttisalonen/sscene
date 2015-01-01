#ifndef SCENE_SCENE_H
#define SCENE_SCENE_H

#include <tuple>
#include <map>

#include <boost/shared_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include "common/Vector3.h"
#include "common/Matrix44.h"
#include "common/Color.h"
#include "common/Texture.h"

#include "Model.h"

namespace Scene {

extern const Common::Vector3 WorldForward;
extern const Common::Vector3 WorldUp;

enum class Reference {
	World,
	Local
};

class Camera : public Movable {
	public:
		Camera();
		void lookAt(const Common::Vector3& tgt, const Common::Vector3& right);
		void applyMovementKeys(float coeff);
		void setForwardMovement(float speed);
		void clearForwardMovement();
		void setSidewaysMovement(float speed);
		void clearSidewaysMovement();
		void setUpwardsMovement(float speed);
		void clearUpwardsMovement();
		void rotate(float yaw, float pitch);

	private:
		void setMovementKey(const std::string& key, float forward,
				float up, float sideways);
		void clearMovementKey(const std::string& key);

		Common::Vector3 calculateMovement(const std::tuple<float, float, float>& tuple);

		std::map<std::string, std::tuple<float, float, float>> mMovement;
		std::map<std::tuple<float, float, float>, Common::Vector3> mMovementCache;

		float mHRot;
		float mVRot;
};

class Light {
	public:
		Light(const Common::Color& col, bool on = true);
		void setState(bool on);
		bool isOn() const;
		const Common::Vector3& getColor() const;
		void setColor(const Common::Color& c);
		void setColor(const Common::Vector3& c);

	private:
		bool mOn;
		Common::Vector3 mColor;
};

class PointLight : public Light, public Movable {
	public:
		PointLight(const Common::Vector3& pos, const Common::Vector3& attenuation, const Common::Color& col, bool on = true);
		const Common::Vector3& getAttenuation() const;
		void setAttenuation(const Common::Vector3& v);

	private:
		Common::Vector3 mAttenuation;
};

class DirectionalLight : public Light {
	public:
		DirectionalLight(const Common::Vector3& dir, const Common::Color& col, bool on = true);
		const Common::Vector3& getDirection() const;
		void setDirection(const Common::Vector3& dir);

	private:
		Common::Vector3 mDirection;
};

class Drawable;

class Line {
	public:
		Line();
		~Line();
		GLuint getVertexBuffer() const;
		GLuint getColorBuffer() const;
		unsigned int getNumVertices() const;
		void addSegment(const Common::Vector3& start, const Common::Vector3& end, const Common::Color& color);
		void clear();
		bool isEmpty() const;

		static const unsigned int VERTEX_POS_INDEX;
		static const unsigned int COLOR_INDEX;

	private:
		std::vector<std::tuple<Common::Vector3, Common::Vector3, Common::Color>> mSegments;
		GLuint mVBOIDs[2];
};

class Overlay {
	public:
		Overlay(const std::string& filename, unsigned int screenwidth, unsigned int screenheight);
		~Overlay();
		GLuint getTexture() const;
		GLuint getVertexBuffer() const;
		GLuint getTexCoordBuffer() const;
		void setEnabled(bool e) { mEnabled = e; }
		bool isEnabled() const { return mEnabled; }
		static const unsigned int VERTEX_POS_INDEX;
		static const unsigned int TEXCOORD_INDEX;

	private:
		boost::shared_ptr<Common::Texture> mTexture;
		GLuint mVBOIDs[2];
		bool mEnabled;
};

struct Shader;

class Scene {
	public:
		Scene(float screenWidth, float screenHeight);
		Camera& getDefaultCamera();
		void addSkyBox();
		Light& getAmbientLight();
		DirectionalLight& getDirectionalLight();
		PointLight& getPointLight();
		void render();
		void addTexture(const std::string& name, const std::string& filename);
		void addModel(const std::string& name, const std::string& filename);
		void addModel(const std::string& name, const Model& model);
		// resulting model will span from (0, 0) to (width * xzscale, width * xzscale)
		void addModelFromHeightmap(const std::string& name, const Heightmap& heightmap);
		void addPlane(const std::string& name, float uscale, float vscale, unsigned int segments);
		void addLine(const std::string& name, const Common::Vector3& start, const Common::Vector3& end, const Common::Color& color);
		void clearLine(const std::string& name);
		void getModel(const std::string& name);
		void setFOV(float angle);
		float getFOV() const;
		void setZFar(float angle);
		float getZFar() const;
		void setClearColor(const Common::Color& color);
		void addOverlay(const std::string& name, const std::string& filename);
		void setOverlayEnabled(const std::string& name, bool enabled);
		void setWireframe(bool w);
		boost::shared_ptr<MeshInstance> addMeshInstance(const std::string& name,
				const std::string& modelname,
				const std::string& texturename, bool usebackfaceculling = true, bool useblending = false);

	private:
		void calculateModelMatrix(const MeshInstance& mi);
		void updateMVPMatrix(const MeshInstance& mi);
		void updateFrameMatrices(const Camera& cam);
		GLuint loadShader(const Shader& s);
		boost::shared_ptr<Common::Texture> getModelTexture(const std::string& mname) const;
		Common::Matrix44 getOrthoMVP() const;

		float mScreenWidth;
		float mScreenHeight;

		GLuint mSceneProgram;
		GLuint mLineProgram;
		GLuint mOverlayProgram;
		std::map<GLuint, std::map<const char*, GLint>> mUniformLocationMap;

		Camera mDefaultCamera;

		Light mAmbientLight;
		DirectionalLight mDirectionalLight;
		PointLight mPointLight;

		std::map<std::string, boost::shared_ptr<Common::Texture>> mTextures;

		Common::Matrix44 mInverseModelMatrix;
		Common::Matrix44 mModelMatrix;

		Common::Matrix44 mViewMatrix;
		Common::Matrix44 mPerspectiveMatrix;

		std::map<std::string, boost::shared_ptr<Drawable>> mDrawables;
		std::map<std::string, boost::shared_ptr<MeshInstance>> mMeshInstances;
		std::map<std::string, boost::shared_ptr<Common::Texture>> mMeshInstanceTextures;
		std::map<std::string, Line> mLines;
		std::map<std::string, boost::shared_ptr<Overlay>> mOverlays;

		float mFOV;
		float mZFar;
		Common::Color mClearColor;
};

}

#endif

