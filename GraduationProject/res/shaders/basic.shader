#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
//layout(location = 1) in vec3 aNormal;
//layout(location = 2) in vec2 aTex;

//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//gl_Position = vec4(aPos, 1.0f);
	gl_Position = projection * view * vec4(aPos, 1.0f);
}


#shader fragment
#version 330 core
out vec4 FragColor;

void main()
{
	FragColor = vec4(1.0, 1.0 ,1.0, 1.0f);
}