//
//  Shader.h
//  glfwTutorial
//
//  Created by Saburo Okita on 10/4/13.
//  Copyright (c) 2013 Saburo Okita. All rights reserved.
//

#ifndef Shader_H
#define Shader_H

#include <iostream>
#include <map>

// GLEW
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

/**
 * A helper class to parse shader files, and acts as a facade(?) pattern
 */
class Shader {

private:
    GLuint vertexShaderId;
    GLuint fragmentShaderId;
    GLuint programId;
    map<string, int> uniforms;

    void getUniform(const string uniform_name);
    string readShader(const char* filename);
    static void getShaderLog(const char* message, ostream& os, GLuint shader_id);
    static void getProgramLog(ostream& os, GLuint program_id);

public:
    Shader();
    Shader(const char* vert_filename, const char* frag_filename);
    ~Shader();

    void init(const char* vert_filename, const char* frag_filename);
    void bind();
    void unbind();
    unsigned int getId();

    GLint getAttribute(const char* attribute_name);
    void setUniform(const char* uniform_name, GLint v0);
    void setUniform(const char* uniform_name, GLfloat v0);
    void setUniform(const char* uniform_name, glm::mat3 matrix);
    void setUniform(const char* uniform_name, glm::mat4 matrix);
    void setUniform(const char* uniform_name, glm::vec3 vector);

};

string& rtrim(string& s);
#endif 
