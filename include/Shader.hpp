#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class Shader
{
	public:
		GLuint ID;
		Shader(const std::string& vFileName, const std::string& fFileName)
		{
			std::string vertexShaderSource = LoadShaderAsString(vFileName);
  	  std::string fragmentShaderSource = LoadShaderAsString(fFileName);

			ID = glCreateProgram();
			
			GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
			GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
			
			  // Link our two shader programs together
 			 // Consider this the equivalent of taking two .cpp files, and linking them into one executable file
  			glAttachShader(ID, myVertexShader);
  			glAttachShader(ID, myFragmentShader);
  			glLinkProgram(ID);

  			// Validate our program
  			glValidateProgram(ID);

  			// Once our final program Object has been created, we can detach and then delete our individual shaders
  			glDetachShader(ID,myVertexShader);
  			glDetachShader(ID, myFragmentShader);
  			// Delete individual shaders once we are done
  			glDeleteShader(myVertexShader);
  			glDeleteShader(myFragmentShader);
		}
		
		void use()
		{
			glUseProgram(ID);
		}

    // Utility uniform functions
    void setBool(const std::string &name, bool value) const
    {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const
    {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloat(const std::string &name, float value) const
    {
      glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec3(const std::string &name, float value1, float value2, float value3) const
    {
      glUniform3f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3);
    }

    void setMat4(const std::string &name, glm::mat4 matrix)
    {
      int location = glGetUniformLocation(ID, name.c_str());
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
      
    }

	private:
		std::string LoadShaderAsString(const std::string& filename)
		{
  			// Resulting shader program loaded as a single string
  			std::string result = "";

  			std::string line = "";
  			std::ifstream myFile(filename.c_str());

  			if(myFile.is_open())
  			{
    			while(std::getline(myFile, line))
    			{
      			result += line + '\n';
    			}
    			myFile.close();
  			}

  			return result;
		}
		
		GLuint CompileShader(GLuint type, const std::string& source)
		{
  			// Compile our shaders
  			GLuint shaderObject;

  			// Based on the type passed in, we create a shader object specifically for that
  			if (type == GL_VERTEX_SHADER)
  			{
    			shaderObject = glCreateShader(GL_VERTEX_SHADER);
  
  			} 
  			else if (type == GL_FRAGMENT_SHADER)
  			{
   			 	shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
  			}

  			const char* src = source.c_str();
  			// The source of our shader
  			glShaderSource(shaderObject, 1, &src, nullptr);
  			// Now compile our shader
  			glCompileShader(shaderObject);

  			// Retrieve the result of our compilation
  			int result;
  			// Our goal with glGetShaderiv is to retrieve the compilation status
  			glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

  			if (result == GL_FALSE)
  			{
    			int length;
    			glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
    			char * errorMessages = new char[length]; // Could also use alloc here
    			glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

    			if (type == GL_VERTEX_SHADER)
    			{
     			 std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
   			 }
    			else if (type == GL_FRAGMENT_SHADER)
    			{
      			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
    			}

    			// Reclaim our memory
    			delete[] errorMessages;

    			// Delete our broken shader
    			glDeleteShader(shaderObject);

    			return 0;
  			}

  			return shaderObject;
			}
};
#endif
