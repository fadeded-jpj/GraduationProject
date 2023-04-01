#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

out vec2 screenCoord;
out vec3 pix;

void main()
{
	//gl_Position = projection * view * vec4(aPos, 1.0f);
	gl_Position = vec4(aPos, 1.0f);

	screenCoord = (vec2(aPos.x, aPos.y) + 1.0) / 2.0;
	pix = aPos;
}


#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 screenCoord;
in vec3 pix;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;

void main()
{	
	FragColor = vec4(texture(texture0, screenCoord).rgb, 1);
	//FragColor = vec4(1,0,0,1);
}