/* Handle seriously boring stuff. */

#ifndef GraphMan_H_
#define GraphMan_H_

#include <GL/glew.h>
#include <GL/glfw.h>


class GraphicsManager
{
	friend void ResizeFunc(int32,int32);
	friend void InputFunc(int32,int32);
	glm::vec2 size;
	glm::vec2 matrixSize;
	glm::mat4 projection;

	GLuint defaultVertexShader, defaultFragShader, defaultShaderProgram, defaultVao;

	void SetupCEGUI();
	void SetupShaders();

public:
	GraphicsManager();
	bool isGuiInputEnabled;
	void SetupWindow(GLFWvidmode videomode);
	void AutoSetupWindow();
	void ClearWindow(); // basically wrapping up glClear
	void Cleanup();

	// assigns this matrix on the stack
	void SetMatrix(GLenum MatrixMode, glm::mat4 matrix);

	// returns the mouse position relative to the matrix and window size
	glm::vec2 GetRelativeMPos();

	// returns the size of the window
	glm::vec2 GetWindowSize();

	// returns the size of the orthogonal matrix
	glm::vec2 GetMatrixSize();

	int32 GetDefaultFragShader();
	int32 GetDefaultVertexShader();
	int32 GetShaderProgram();
};

extern GraphicsManager GraphMan;

#endif