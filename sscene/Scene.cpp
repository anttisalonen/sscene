#include "Scene.h"

#include <cassert>

#include "HelperFunctions.h"

#include "common/Texture.h"
#include "common/Math.h"

using namespace Common;
using namespace Scene;

namespace Scene {

#include "shaders/scene.vert.h"
#include "shaders/scene.frag.h"
#include "shaders/line.vert.h"
#include "shaders/line.frag.h"
#include "shaders/overlay.vert.h"
#include "shaders/overlay.frag.h"

#define CHECK_GL_ERROR_IMPL(file, line) { \
	do { \
		while(1) { \
			GLenum err = glGetError(); \
			if(err == GL_NO_ERROR) { \
				break; \
			} \
			fprintf(stderr, "%s:%d: GL error 0x%04x\n", file, line, err); \
		} \
	} while(0); \
	}

#define CHECK_GL_ERROR() { do { CHECK_GL_ERROR_IMPL(__FILE__, __LINE__); } while(0); }

const Vector3 WorldForward = Vector3(1, 0, 0);
const Vector3 WorldUp      = Vector3(0, 1, 0);


struct attrib {
	const char* name;
	int elems;
	const std::vector<GLfloat>& data;
};

void loadBufferData(const std::vector<attrib>& attribs, GLuint* vboids)
{
	int i = 0;
	for(auto& a : attribs) {
		glBindBuffer(GL_ARRAY_BUFFER, vboids[i]);
		glBufferData(GL_ARRAY_BUFFER, a.data.size() * sizeof(GLfloat), &a.data[0], GL_STATIC_DRAW);
		glVertexAttribPointer(i, a.elems, GL_FLOAT, GL_FALSE, 0, NULL);
		i++;
	}
}

const unsigned int Line::VERTEX_POS_INDEX = 0;
const unsigned int Line::COLOR_INDEX = 1;

Line::Line()
{
	glGenBuffers(2, mVBOIDs);
}

Line::~Line()
{
	if(mVBOIDs[0]) {
		glDeleteBuffers(2, mVBOIDs);
	}
}

void Line::addSegment(const Common::Vector3& start, const Common::Vector3& end, const Common::Color& color)
{
	mSegments.push_back(std::make_tuple(start, end, color));

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> colors;

	for(const auto& t : mSegments) {
		vertices.push_back(std::get<0>(t).x);
		vertices.push_back(std::get<0>(t).y);
		vertices.push_back(std::get<0>(t).z);
		vertices.push_back(std::get<1>(t).x);
		vertices.push_back(std::get<1>(t).y);
		vertices.push_back(std::get<1>(t).z);
		colors.push_back(std::get<2>(t).r / 255.0f);
		colors.push_back(std::get<2>(t).g / 255.0f);
		colors.push_back(std::get<2>(t).b / 255.0f);
		colors.push_back(std::get<2>(t).r / 255.0f);
		colors.push_back(std::get<2>(t).g / 255.0f);
		colors.push_back(std::get<2>(t).b / 255.0f);
	}

	std::vector<attrib> attribs = { { "a_Position", 3, vertices },
		{ "a_Color", 3, colors } };

	loadBufferData(attribs, mVBOIDs);
}

void Line::clear()
{
	mSegments.clear();
}

bool Line::isEmpty() const
{
	return mSegments.empty();
}

GLuint Line::getVertexBuffer() const
{
	return mVBOIDs[0];
}

GLuint Line::getColorBuffer() const
{
	return mVBOIDs[1];
}

unsigned int Line::getNumVertices() const
{
	return mSegments.size() * 2;
}

const unsigned int Overlay::VERTEX_POS_INDEX = 0;
const unsigned int Overlay::TEXCOORD_INDEX = 1;
Overlay::Overlay(const std::string& filename, unsigned int screenwidth, unsigned int screenheight)
	: mEnabled(false)
{
	mTexture = HelperFunctions::loadTexture(filename);

	float sw2 = screenwidth / 2.0f;
	float sh2 = screenheight / 2.0f;

	glGenBuffers(2, mVBOIDs);

	std::vector<GLfloat> pos = {
		 sw2,   sh2, 0.0f,
		-sw2,   sh2, 0.0f,
		-sw2,  -sh2, 0.0f,
		 sw2,  -sh2, 0.0f
	};

	std::vector<GLfloat> tex = {
		 1.0f,  0.0f,
		 0.0f,  0.0f,
		 0.0f,  1.0f,
		 1.0f,  1.0f,
	};

	std::vector<attrib> attribs = { { "a_Position", 3, pos },
	{ "a_texCoord", 2, tex } };

	loadBufferData(attribs, mVBOIDs);
}

Overlay::~Overlay()
{
	glDeleteBuffers(2, mVBOIDs);
}

GLuint Overlay::getTexture() const
{
	return mTexture->getTexture();
}

GLuint Overlay::getVertexBuffer() const
{
	return mVBOIDs[0];
}

GLuint Overlay::getTexCoordBuffer() const
{
	return mVBOIDs[1];
}

Camera::Camera()
	: mHRot(0.0f),
	mVRot(0.0f)
{
	setRotation(WorldForward, WorldUp);
}

void Camera::lookAt(const Common::Vector3& tgt, const Common::Vector3& up)
{
	setRotation(tgt, up);
}

void Camera::setMovementKey(const std::string& key, float forward,
		float up, float sideways)
{
	std::tuple<float, float, float> t(forward, up, sideways);
	mMovement[key] = t;
	mMovementCache[t] = calculateMovement(t);
}

Vector3 Camera::calculateMovement(const std::tuple<float, float, float>& v)
{
	Vector3 r;
	if(std::get<0>(v))
		r += getTargetVector() * std::get<0>(v);
	if(std::get<1>(v))
		r += getUpVector() * std::get<1>(v);
	if(std::get<2>(v))
		r += getTargetVector().cross(getUpVector()) * std::get<2>(v);
	return r;
}

void Camera::clearMovementKey(const std::string& key)
{
	auto& t = mMovement[key];
	mMovementCache[t].zero();
	std::get<0>(t) = 0;
	std::get<1>(t) = 0;
	std::get<2>(t) = 0;
}

void Camera::applyMovementKeys(float coeff)
{
	for(auto p : mMovement) {
		auto m = calculateMovement(p.second);
		mPosition += m;
	}
}

void Camera::setForwardMovement(float speed)
{
	setMovementKey("Forward", speed, 0, 0);
}

void Camera::clearForwardMovement()
{
	clearMovementKey("Forward");
}

void Camera::setSidewaysMovement(float speed)
{
	setMovementKey("Sideways", 0, 0, speed);
}

void Camera::clearSidewaysMovement()
{
	clearMovementKey("Sideways");
}

void Camera::setUpwardsMovement(float speed)
{
	setMovementKey("Upwards", 0, speed, 0);
}

void Camera::clearUpwardsMovement()
{
	clearMovementKey("Upwards");
}

void Camera::rotate(float yaw, float pitch)
{
	mHRot += yaw;
	mVRot += pitch;

	Vector3 view = Math::rotate3D(WorldForward, mHRot, WorldUp).normalized();

	auto haxis = WorldUp.cross(view).normalized();

	Vector3 tgt = Math::rotate3D(view, -mVRot, haxis).normalized();
	setRotation(tgt, tgt.cross(haxis).normalized());

	for(auto p : mMovement) {
		mMovementCache[p.second] = calculateMovement(p.second);
	}
}

Light::Light(const Common::Color& col, bool on)
	: mOn(on)
{
	setColor(col);
}

void Light::setState(bool on)
{
	mOn = on;
}

bool Light::isOn() const
{
	return mOn;
}

const Common::Vector3& Light::getColor() const
{
	return mColor;
}

void Light::setColor(const Common::Color& c)
{
	mColor = Vector3(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f);
}

void Light::setColor(const Common::Vector3& c)
{
	mColor = c;
}

PointLight::PointLight(const Common::Vector3& pos, const Common::Vector3& attenuation, const Common::Color& col, bool on)
	: Light(col, on),
	Movable(pos),
	mAttenuation(attenuation)
{
}

const Common::Vector3& PointLight::getAttenuation() const
{
	return mAttenuation;
}

void PointLight::setAttenuation(const Common::Vector3& v)
{
	mAttenuation = v;
}

DirectionalLight::DirectionalLight(const Common::Vector3& dir, const Common::Color& col, bool on)
	: Light(col, true),
	mDirection(dir.normalized())
{
}

const Common::Vector3& DirectionalLight::getDirection() const
{
	return mDirection;
}

void DirectionalLight::setDirection(const Common::Vector3& dir)
{
	mDirection = dir.normalized();
}


class Drawable {
	public:
		Drawable(GLuint programObject, const Model& model);
		~Drawable();
		Drawable& operator=(const Drawable&) = delete;
		Drawable(const Drawable&) = delete;

		GLuint getVertexBuffer() const;
		GLuint getTexCoordBuffer() const;
		GLuint getNormalBuffer() const;
		GLuint getIndexBuffer() const;
		unsigned int getNumIndices() const;
		unsigned int getNumVertices() const;

		static const unsigned int VERTEX_POS_INDEX;
		static const unsigned int TEXCOORD_INDEX;
		static const unsigned int NORMAL_INDEX;

	private:
		void initBuffers(GLuint programObject, const Model& model);

		GLuint mVBOIDs[4];
		unsigned int mNumIndices;
		unsigned int mNumVertices;
};

const unsigned int Drawable::VERTEX_POS_INDEX = 0;
const unsigned int Drawable::TEXCOORD_INDEX = 1;
const unsigned int Drawable::NORMAL_INDEX = 2;

Drawable::Drawable(GLuint programObject, const Model& model)
{
	initBuffers(programObject, model);
	mNumIndices = model.getIndices().size();
	mNumVertices = model.getVertexCoords().size() / 3;
}

Drawable::~Drawable()
{
	glDeleteBuffers(4, mVBOIDs);
}

GLuint Drawable::getVertexBuffer() const
{
	return mVBOIDs[0];
}

GLuint Drawable::getTexCoordBuffer() const
{
	return mVBOIDs[1];
}

GLuint Drawable::getNormalBuffer() const
{
	return mVBOIDs[2];
}

GLuint Drawable::getIndexBuffer() const
{
	return mVBOIDs[3];
}

unsigned int Drawable::getNumIndices() const
{
	return mNumIndices;
}

unsigned int Drawable::getNumVertices() const
{
	return mNumVertices;
}

void Drawable::initBuffers(GLuint programObject, const Model& model)
{
	glGenBuffers(4, mVBOIDs);

	std::vector<attrib> attribs = { { "a_Position", 3, model.getVertexCoords() },
		{ "a_texCoord", 2, model.getTexCoords() },
		{ "a_Normal", 3, model.getNormals() } };

	loadBufferData(attribs, mVBOIDs);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOIDs[3]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.getIndices().size() * sizeof(GLushort),
			&model.getIndices()[0], GL_STATIC_DRAW);
}

struct Shader {
	const char* vertexShader;
	const char* fragmentShader;
	std::vector<const char*> uniforms;
	std::vector<std::pair<GLuint, const char*>> attribs;
};

GLuint Scene::loadShader(const Shader& s)
{
	GLuint vshader;
	GLuint fshader;
	GLint linked;
	GLuint program;

	vshader = HelperFunctions::loadShader(GL_VERTEX_SHADER, s.vertexShader);
	fshader = HelperFunctions::loadShader(GL_FRAGMENT_SHADER, s.fragmentShader);

	program = glCreateProgram();

	if(program == 0) {
		std::cerr << "Unable to create program.\n";
		throw std::runtime_error("Error initialising 3D");
	}

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	for(const auto& attr : s.attribs) {
		glEnableVertexAttribArray(attr.first);
		glBindAttribLocation(program, attr.first, attr.second);
	}

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &linked);

	if(!linked) {
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1) {
			char* infoLog = new char[infoLen];
			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
			std::cerr << "Error linking program: " << infoLog << "\n";
			delete[] infoLog;
		} else {
			std::cerr << "Unknown error when linking program.\n";
		}

		glDeleteProgram(program);
		throw std::runtime_error("Error initialising 3D");
	}

	for(auto& p : s.uniforms) {
		mUniformLocationMap[program][p] = glGetUniformLocation(program, p);
	}

	return program;
}

Scene::Scene(float screenWidth, float screenHeight)
	: mScreenWidth(screenWidth),
	mScreenHeight(screenHeight),
	mAmbientLight(Color::White, false),
	mDirectionalLight(Vector3(1, 0, 0), Color::White, false),
	mPointLight(Vector3(), Vector3(), Color::White, false),
	mFOV(90.0f),
	mZFar(200.0f),
	mClearColor(0, 0, 0)
{
	GLenum glewerr = glewInit();
	if (glewerr != GLEW_OK) {
		std::cerr << "Unable to initialise GLEW.\n";
		throw std::runtime_error("Error initialising 3D");
	}
	if (!GLEW_VERSION_2_1) {
		std::cerr << "OpenGL 2.1 not supported.\n";
		throw std::runtime_error("Error initialising 3D");
	}

	printf("%-20s: %s\n", "GL vendor", glGetString(GL_VENDOR));
	printf("%-20s: %s\n", "GL renderer", glGetString(GL_RENDERER));
	printf("%-20s: %s\n", "GL version", glGetString(GL_VERSION));
	printf("%-20s: %s\n", "GLSL version", glGetString(GL_SHADING_LANGUAGE_VERSION));

	Shader scene;
	scene.vertexShader = scene_vert;
	scene.fragmentShader = scene_frag;
	scene.uniforms = {
		"u_MVP",
		"u_inverseMVP",
		"s_texture",
		"u_ambientLight",
		"u_directionalLightDirection",
		"u_directionalLightColor",
		"u_pointLightPosition",
		"u_pointLightAttenuation",
		"u_pointLightColor",
		"u_ambientLightEnabled",
		"u_directionalLightEnabled",
		"u_pointLightEnabled"
	};

	scene.attribs = {
		{ Drawable::VERTEX_POS_INDEX, "a_Position" },
		{ Drawable::TEXCOORD_INDEX, "a_texCoord" },
		{ Drawable::NORMAL_INDEX, "a_Normal" }
	};

	mSceneProgram = loadShader(scene);

	Shader line;
	line.vertexShader = line_vert;
	line.fragmentShader = line_frag;
	line.uniforms = {
		"u_MVP"
	};

	line.attribs = {
		{ Line::VERTEX_POS_INDEX, "a_Position" },
		{ Line::COLOR_INDEX, "a_Color" }
	};
	mLineProgram = loadShader(line);

	{
		Shader overlay;
		overlay.vertexShader = overlay_vert;
		overlay.fragmentShader = overlay_frag;
		overlay.uniforms = {
			"u_MVP",
			"s_texture"
		};

		overlay.attribs = {
			{ Overlay::VERTEX_POS_INDEX, "a_Position" },
			{ Overlay::TEXCOORD_INDEX, "a_texCoord" }
		};
		mOverlayProgram = loadShader(overlay);
	}

	HelperFunctions::enableDepthTest();
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, screenWidth, screenHeight);

	glUseProgram(mSceneProgram);
}

boost::shared_ptr<Common::Texture> Scene::getModelTexture(const std::string& mname) const
{
	auto it = mMeshInstanceTextures.find(mname);
	if(it == mMeshInstanceTextures.end()) {
		throw std::runtime_error("Couldn't find texture for model");
	} else {
		return it->second;
	}
}

Camera& Scene::getDefaultCamera()
{
	return mDefaultCamera;
}

void Scene::addSkyBox()
{
	/* TODO */
}

Light& Scene::getAmbientLight()
{
	return mAmbientLight;
}

DirectionalLight& Scene::getDirectionalLight()
{
	return mDirectionalLight;
}

PointLight& Scene::getPointLight()
{
	return mPointLight;
}

void Scene::calculateModelMatrix(const MeshInstance& mi)
{
	auto translation = HelperFunctions::translationMatrix(mi.getPosition());
	auto rotation = mi.getRotation();
	auto scale = HelperFunctions::scaleMatrix(mi.getScale());
	mModelMatrix = scale * rotation * translation;

	auto invTranslation(translation);
	invTranslation.m[3] = -invTranslation.m[3];
	invTranslation.m[7] = -invTranslation.m[7];
	invTranslation.m[11] = -invTranslation.m[11];

	auto invRotation = rotation.transposed();

	auto invScale = scale;
	invScale.m[0] = 1.0f / invScale.m[0];
	invScale.m[5] = 1.0f / invScale.m[5];
	invScale.m[10] = 1.0f / invScale.m[10];

	mInverseModelMatrix = invTranslation * invRotation * invScale;
}

void Scene::updateMVPMatrix(const MeshInstance& mi)
{
	calculateModelMatrix(mi);
	auto mvp = mModelMatrix * mViewMatrix * mPerspectiveMatrix;
	auto imvp = mInverseModelMatrix;

	glUniformMatrix4fv(mUniformLocationMap[mSceneProgram]["u_MVP"], 1, GL_FALSE, mvp.m);
	glUniformMatrix4fv(mUniformLocationMap[mSceneProgram]["u_inverseMVP"], 1, GL_FALSE, imvp.m);
}

void Scene::updateFrameMatrices(const Camera& cam)
{
	mPerspectiveMatrix = HelperFunctions::perspectiveMatrix(mFOV, mScreenWidth, mScreenHeight, mZFar);
	auto camrot = HelperFunctions::cameraRotationMatrix(cam.getTargetVector(), cam.getUpVector());
	auto camtrans = HelperFunctions::translationMatrix(cam.getPosition().negated());
	mViewMatrix = camtrans * camrot;
}

Common::Matrix44 Scene::getOrthoMVP() const
{
	return HelperFunctions::orthoMatrix(mScreenWidth, mScreenHeight);
}

void Scene::render()
{
	glClearColor(mClearColor.r / 256.0f, mClearColor.g / 256.0f, mClearColor.b / 256.0f, 1.0f);

	glUseProgram(mSceneProgram);
	glUniform1i(mUniformLocationMap[mSceneProgram]["u_ambientLightEnabled"], mAmbientLight.isOn());
	glUniform1i(mUniformLocationMap[mSceneProgram]["u_directionalLightEnabled"], mDirectionalLight.isOn());
	glUniform1i(mUniformLocationMap[mSceneProgram]["u_pointLightEnabled"], mPointLight.isOn());

	updateFrameMatrices(mDefaultCamera);

	if(mPointLight.isOn()) {
		auto at = mPointLight.getAttenuation();
		auto col = mPointLight.getColor();
		glUniform3f(mUniformLocationMap[mSceneProgram]["u_pointLightAttenuation"], at.x, at.y, at.z);
		glUniform3f(mUniformLocationMap[mSceneProgram]["u_pointLightColor"], col.x, col.y, col.z);
	}

	if(mDirectionalLight.isOn()) {
		auto col = mDirectionalLight.getColor();
		glUniform3f(mUniformLocationMap[mSceneProgram]["u_directionalLightColor"], col.x, col.y, col.z);
	}

	if(mAmbientLight.isOn()) {
		auto col = mAmbientLight.getColor();
		glUniform3f(mUniformLocationMap[mSceneProgram]["u_ambientLight"], col.x, col.y, col.z);
	}

	for(const auto& mi : mMeshInstances) {
		/* TODO: add support for vertex colors. */
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, getModelTexture(mi.first)->getTexture());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glUniform1i(mUniformLocationMap[mSceneProgram]["s_texture"], 0);

		updateMVPMatrix(*mi.second);

		if(mPointLight.isOn()) {
			// inverse translation matrix
			Vector3 plpos(mPointLight.getPosition());
			Vector3 plposrel = mi.second->getPosition() - plpos;
			glUniform3f(mUniformLocationMap[mSceneProgram]["u_pointLightPosition"],
					plposrel.x, plposrel.y, plposrel.z);
		}

		if(mDirectionalLight.isOn()) {
			// inverse rotation matrix (normal matrix)
			Vector3 dir = mDirectionalLight.getDirection();
			glUniform3f(mUniformLocationMap[mSceneProgram]["u_directionalLightDirection"], dir.x, dir.y, dir.z);
		}

		const auto& d = mi.second->getDrawable();
		if(mi.second->useBlending()) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glDisable(GL_BLEND);
		}

		if(mi.second->useBackfaceCulling()) {
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}

		const auto& ib = d.getIndexBuffer();
		if(d.getNumIndices() != 0) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		}

		glEnableVertexAttribArray(Drawable::VERTEX_POS_INDEX);
		glEnableVertexAttribArray(Drawable::TEXCOORD_INDEX);
		glEnableVertexAttribArray(Drawable::NORMAL_INDEX);
		glBindBuffer(GL_ARRAY_BUFFER, d.getVertexBuffer());
		glVertexAttribPointer(Drawable::VERTEX_POS_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, d.getTexCoordBuffer());
		glVertexAttribPointer(Drawable::TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, d.getNormalBuffer());
		glVertexAttribPointer(Drawable::NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);

		if(d.getNumIndices() != 0) {
			glDrawElements(GL_TRIANGLES, d.getNumIndices(),
					GL_UNSIGNED_SHORT, NULL);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, d.getNumVertices());
		}
		glDisableVertexAttribArray(Drawable::VERTEX_POS_INDEX);
		glDisableVertexAttribArray(Drawable::TEXCOORD_INDEX);
		glDisableVertexAttribArray(Drawable::NORMAL_INDEX);

		CHECK_GL_ERROR();
	}

	glUseProgram(mLineProgram);
	auto mvp = mViewMatrix * mPerspectiveMatrix;
	glUniformMatrix4fv(mUniformLocationMap[mSceneProgram]["u_MVP"], 1, GL_FALSE, mvp.m);
	for(const auto& kv : mLines) {
		if(kv.second.isEmpty())
			continue;

		glEnableVertexAttribArray(Line::VERTEX_POS_INDEX);
		glEnableVertexAttribArray(Line::COLOR_INDEX);
		glBindBuffer(GL_ARRAY_BUFFER, kv.second.getVertexBuffer());
		glVertexAttribPointer(Line::VERTEX_POS_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, kv.second.getColorBuffer());
		glVertexAttribPointer(Line::COLOR_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_LINES, 0, kv.second.getNumVertices());
		glDisableVertexAttribArray(Line::VERTEX_POS_INDEX);
		glDisableVertexAttribArray(Line::COLOR_INDEX);
		CHECK_GL_ERROR();
	}

	if(!mOverlays.empty()) {
		glUseProgram(mOverlayProgram);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for(const auto& kv : mOverlays) {
			if(!kv.second->isEnabled()) {
				continue;
			}

			auto mvp = getOrthoMVP();
			glUniformMatrix4fv(mUniformLocationMap[mOverlayProgram]["u_MVP"], 1, GL_FALSE, mvp.m);
			glUniform1i(mUniformLocationMap[mOverlayProgram]["s_texture"], 0);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, kv.second->getTexture());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glEnableVertexAttribArray(Overlay::VERTEX_POS_INDEX);
			glEnableVertexAttribArray(Overlay::TEXCOORD_INDEX);
			glBindBuffer(GL_ARRAY_BUFFER, kv.second->getVertexBuffer());
			glVertexAttribPointer(Overlay::VERTEX_POS_INDEX, 3, GL_FLOAT, GL_FALSE, 0, 0);

			glBindBuffer(GL_ARRAY_BUFFER, kv.second->getTexCoordBuffer());
			glVertexAttribPointer(Overlay::TEXCOORD_INDEX, 2, GL_FLOAT, GL_FALSE, 0, 0);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDisableVertexAttribArray(Overlay::VERTEX_POS_INDEX);
			glDisableVertexAttribArray(Overlay::TEXCOORD_INDEX);
			CHECK_GL_ERROR();
		}
	}
}

void Scene::addTexture(const std::string& name, const std::string& filename)
{
	if(mTextures.find(name) != mTextures.end()) {
		throw std::runtime_error("Tried adding an already existing texture");
	} else {
		mTextures.insert({name, HelperFunctions::loadTexture(filename)});
	}
}

void Scene::addModel(const std::string& name, const Model& model)
{
	if(mDrawables.find(name) != mDrawables.end()) {
		throw std::runtime_error("Tried adding a model with an already existing name");
	} else {
		boost::shared_ptr<Drawable> d(new Drawable(mSceneProgram, model));
		std::cout << (d->getNumVertices()) << " vertices.\n";
		std::cout << (d->getNumIndices() / 3) << " triangles.\n";
		mDrawables.insert({name, d});
	}
}

void Scene::addModel(const std::string& name, const std::string& filename)
{
	auto m = Model(filename);
	addModel(name, m);
}

void Scene::addModelFromHeightmap(const std::string& name, const Heightmap& heightmap)
{
	auto m = Model(heightmap, 1.0f, 1.0f);
	addModel(name, m);
}

void Scene::addLine(const std::string& name, const Common::Vector3& start, const Common::Vector3& end, const Common::Color& color)
{
	mLines[name].addSegment(start, end, color);
}

class PlaneHeightmap : public Heightmap {
	public:
		PlaneHeightmap(unsigned int segments)
			: mSegments(segments) { }
		virtual float getHeightAt(float x, float y) const
		{
			return 0.0f;
		}
		virtual unsigned int getWidth() const
		{
			return mSegments;
		}
		virtual float getXZScale() const
		{
			return 1.0f / static_cast<float>(mSegments);
		}

	private:
		unsigned int mSegments;
};

void Scene::addPlane(const std::string& name, float uscale, float vscale, unsigned int segments)
{
	PlaneHeightmap heightmap(segments);

	auto m = Model(heightmap, uscale, vscale);
	addModel(name, m);
}

void Scene::addModel(const std::string& name, const std::vector<Common::Vector3>& vertexcoords,
				const std::vector<Common::Vector2>& texcoords,
				const std::vector<unsigned int>& indices,
				const std::vector<Common::Vector3>& normals)
{
	auto m = Model(vertexcoords, texcoords, indices, normals);
	addModel(name, m);
}

void Scene::clearLine(const std::string& name)
{
	mLines[name].clear();
}

void Scene::setFOV(float angle)
{
	mFOV = angle;
}

float Scene::getFOV() const
{
	return mFOV;
}

void Scene::setZFar(float zfar)
{
	mZFar = zfar;
}

float Scene::getZFar() const
{
	return mZFar;
}

void Scene::setClearColor(const Common::Color& color)
{
	mClearColor = color;
}

void Scene::addOverlay(const std::string& name, const std::string& filename)
{
	if(mOverlays.find(name) != mOverlays.end()) {
		throw std::runtime_error("Tried adding an already existing overlay");
	} else {
		auto ov = boost::shared_ptr<Overlay>(new Overlay(filename, mScreenWidth, mScreenHeight));
		mOverlays.insert({name, ov});
	}
}

void Scene::setOverlayEnabled(const std::string& name, bool enabled)
{
	auto it = mOverlays.find(name);
	if(it == mOverlays.end()) {
		throw std::runtime_error("Tried getting a non-existing model\n");
	} else {
		it->second->setEnabled(enabled);
	}
}

void Scene::setWireframe(bool w)
{
	glPolygonMode(GL_FRONT_AND_BACK, w ? GL_LINE : GL_FILL);
}

void Scene::getModel(const std::string& name)
{
	auto it = mDrawables.find(name);
	if(it == mDrawables.end()) {
		throw std::runtime_error("Tried getting a non-existing model\n");
	}
}

boost::shared_ptr<MeshInstance> Scene::addMeshInstance(const std::string& name,
		const std::string& modelname, const std::string& texturename, bool usebackfaceculling, bool useblending)
{
	if(mMeshInstances.find(name) != mMeshInstances.end()) {
		throw std::runtime_error("Tried adding a mesh instance with an already existing name");
	}

	auto modelit = mDrawables.find(modelname);
	if(modelit == mDrawables.end())
		throw std::runtime_error("Tried getting a non-existing model\n");

	auto textit = mTextures.find(texturename);
	if(textit == mTextures.end())
		throw std::runtime_error("Tried getting a non-existing texture\n");

	auto mi = boost::shared_ptr<MeshInstance>(new MeshInstance(*modelit->second, usebackfaceculling, useblending));
	mMeshInstances.insert({name, mi});

	mMeshInstanceTextures.insert({name, textit->second});

	return mi;
}

}

