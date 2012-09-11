#ifndef SCENE_HELPERFUNCTIONS_H
#define SCENE_HELPERFUNCTIONS_H

#include <string>

#include <boost/shared_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include "common/Vector3.h"
#include "common/Matrix44.h"
#include "common/Texture.h"

namespace Scene {

class HelperFunctions {
	public:
		static Common::Matrix44 translationMatrix(const Common::Vector3& v);
		static Common::Matrix44 rotationMatrixFromEuler(const Common::Vector3& v);
		static Common::Matrix44 perspectiveMatrix(float fov, int screenwidth, int screenheight);
		static Common::Matrix44 cameraRotationMatrix(const Common::Vector3& tgt, const Common::Vector3& up);

		static Common::Matrix44 rotationMatrixFromAxisAngle(const Common::Vector3& axis, float angle);

		static GLuint loadShader(GLenum type, const char* src);
		static GLuint loadShaderFromFile(GLenum type, const char* filename);

		static boost::shared_ptr<Common::Texture> loadTexture(const std::string& filename);

		static void enableDepthTest();
};

}

#endif

