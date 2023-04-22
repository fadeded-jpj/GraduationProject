#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

out vec2 screenCoord;

void main()
{
	//gl_Position = projection * view * vec4(aPos, 1.0f);
	gl_Position = vec4(aPos, 1.0f);

	screenCoord = (vec2(aPos.x, aPos.y) + 1.0) / 2.0;
}


#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 screenCoord;

uniform sampler2D texture0;
uniform int frameCout;

//const float offset = 1.0 / 900.0;
//
//void main()
//{
//    vec2 offsets[9] = vec2[](
//        vec2(-offset, offset), // ����
//        vec2(0.0f, offset), // ����
//        vec2(offset, offset), // ����
//        vec2(-offset, 0.0f),   // ��
//        vec2(0.0f, 0.0f),   // ��
//        vec2(offset, 0.0f),   // ��
//        vec2(-offset, -offset), // ����
//        vec2(0.0f, -offset), // ����
//        vec2(offset, -offset)  // ����
//        );
//
//    float kernel[9] = float[](
//        1.0 / 16, 2.0 / 16, 1.0 / 16,
//        2.0 / 16, 4.0 / 16, 2.0 / 16,
//        1.0 / 16, 2.0 / 16, 1.0 / 16
//        );
//
//    vec3 sampleTex[9];
//    for (int i = 0; i < 9; i++)
//    {
//        sampleTex[i] = vec3(texture(texture0, screenCoord.st + offsets[i]));
//    }
//    vec3 col = vec3(0.0);
//    for (int i = 0; i < 9; i++)
//        col += sampleTex[i] * kernel[i];
//
//    FragColor = vec4(col, 1.0);
//}

void main()
{
	vec3 color = texture(texture0, screenCoord).xyz;
	FragColor = vec4(color, 1.0);
}