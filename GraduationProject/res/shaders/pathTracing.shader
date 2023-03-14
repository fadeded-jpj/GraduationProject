#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

out vec2 screenCoord;

uniform mat4 view;
uniform mat4 projection;

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

#define TRIANGLE_SIEZ 9
#define INF 12138
#define PI 3.1415926535859

// =========== Struct=============
struct Material {
	vec3 emissive;
	vec3 baseColor;

	float ao;
	float roughness;
	float metallic;
};

struct Triangle {
	vec3 p1, p2, p3;
	vec3 n1, n2, n3;
};

struct Ray {
	vec3 start;
	vec3 dir;
};

struct HitResult {
	bool isHit;
	bool isInside;
	float distance;
	vec3 HitPoint;
	vec3 normal;
	vec3 viewDir;
	Material material;
};

struct Camera
{
	vec3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 origin;
};


// ============= uniform===================
uniform Camera camera;
uniform samplerBuffer triangles;
uniform int triangleCount;
uniform uint frameCount;

// ============= funcion ==================
//=========================== Random =======================
uint wseed = frameCount;

uint whash(uint seed)
{
	seed = (seed ^ uint(61)) ^ (seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> uint(4));
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> uint(15));
	return seed;
}

float randcore4()
{
	wseed = whash(wseed);

	return float(wseed) * (1.0 / 4294967296.0);
}


float rand()
{
	return randcore4();
}

vec3 randVec3()
{
	return vec3(rand(), rand(), rand());
}

vec2 rand2()
{
	return vec2(rand(), rand());
}

vec3 SampleHemisphere() {
	float z = rand();
	float r = max(0, sqrt(1 - z * z));
	float phi = 2.0 * PI * rand();
	return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 toNormalHemisphere(vec3 V, vec3 N)
{
	vec3 helper = vec3(1, 0, 0);
	if (abs(N.x) > 0.99) helper = vec3(0, 0, 1);
	vec3 tangent = normalize(cross(N, helper));
	vec3 bitangent = normalize(cross(N, tangent));
	return V.x * tangent + V.y * bitangent + V.z * N;
}

Triangle getTriangle(int i) {
	int offset = i * TRIANGLE_SIEZ;
	Triangle res;

	// vertex position
	res.p1 = texelFetch(triangles, offset + 0).xyz;
	res.p2 = texelFetch(triangles, offset + 1).xyz;
	res.p3 = texelFetch(triangles, offset + 2).xyz;

	// normal position
	res.n1 = texelFetch(triangles, offset + 3).xyz;
	res.n2 = texelFetch(triangles, offset + 4).xyz;
	res.n3 = texelFetch(triangles, offset + 5).xyz;

	return res;
}

Material getMaterial(int i) {
	int offset = i * TRIANGLE_SIEZ;
	Material res;

	res.emissive = texelFetch(triangles, offset + 6).xyz;
	res.baseColor = texelFetch(triangles, offset + 7).xyz;
	vec3 tmp = texelFetch(triangles, offset + 8).xyz;

	res.ao = tmp.x;
	res.roughness = tmp.y;
	res.metallic = tmp.z;

	return res;
}

// ���ߺ��������ཻ
HitResult TriangleIntersect(Triangle t, Ray ray)
{
	//bool isHit;
	//bool isInside;
	//float distance;
	//vec3 HitPoint;
	//vec3 normal;
	//vec3 viewDir;
	//Material material;

	HitResult res;

	res.isHit = false;
	res.isInside = false;
	res.distance = INF;

	vec3 p1 = t.p1;
	vec3 p2 = t.p2;
	vec3 p3 = t.p3;

	vec3 S = ray.start;
	vec3 D = ray.dir;
	vec3 N = normalize(cross(p2 - p1, p3 - p1));

	// ���ߴӱ���ﵽ����
	if (dot(N, D) > 0.0f) {
		res.isInside = true;
		N = -N;
	}

	// ƽ��������
	if (abs(dot(N, D)) < 0.00005f)
		return res;

	float T = (dot(N, p1) - dot(S, N)) / dot(D, N);
	if (T < 0.0005f) return res;

	// ����
	vec3 P = S + T * D;

	// �ж��Ƿ�����������
	vec3 c1 = cross(p2 - p1, P - p1);
	vec3 c2 = cross(p3 - p2, P - p2);
	vec3 c3 = cross(p1 - p3, P - p3);
	bool res1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
	bool res2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

	//����
	if (res1 || res2)
	{
		res.isHit = true;
		res.distance = T;
		res.HitPoint = P;
		res.viewDir = D;

		//���߲�ֵ
		float alpha = (-(P.x - p2.x) * (p3.y - p2.y) + (P.y - p2.y) * (p3.x - p2.x)) / (-(p1.x - p2.x - 0.00005) * (p3.y - p2.y + 0.00005) + (p1.y - p2.y + 0.00005) * (p3.x - p2.x + 0.00005));
		float beta = (-(P.x - p3.x) * (p1.y - p3.y) + (P.y - p3.y) * (p1.x - p3.x)) / (-(p2.x - p3.x - 0.00005) * (p1.y - p3.y + 0.00005) + (p2.y - p3.y + 0.00005) * (p1.x - p3.x + 0.00005));
		float gama = 1.0 - alpha - beta;
		vec3 finalNormal = normalize(alpha * t.n1 + beta * t.n2 + gama * t.n3);
		res.normal = res.isInside ? -finalNormal : finalNormal;
	}

	return res;
}

// ������������ཻ������
HitResult HitArray(Ray ray, int left, int right)
{
	HitResult res;
	res.isHit = false;
	res.distance = INF;

	for (int i = left; i <= right; i++)
	{
		Triangle t = getTriangle(i);

		HitResult tmp = TriangleIntersect(t, ray);
		if (tmp.isHit && tmp.distance < res.distance) {
			res = tmp;
			res.material = getMaterial(i);
		}
	}
	return res;
}

vec3 pathTracing(HitResult hit, int maxBounce) {
	vec3 Lo = vec3(0);
	vec3 history = vec3(1);

	while (maxBounce > 0)
	{
		vec3 wi = toNormalHemisphere(SampleHemisphere(), hit.normal);

		Ray ray;
		ray.start = hit.HitPoint;
		ray.dir = wi;

		HitResult newHit = HitArray(ray, 0, triangleCount);

		if (!newHit.isHit)
			break;

		float pdf = 1.0 / (2.0 * PI);                                   // ������Ȳ��������ܶ�
		float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // �����ͷ��߼н�����
		float cosine_i = max(0, dot(ray.dir, hit.normal));				// �����ͷ��߼н�����
		vec3 f_r = hit.material.baseColor / PI;

		vec3 Le = newHit.material.emissive;
		Lo += history * Le * f_r * cosine_i / (pdf * 1);

		hit = newHit;
		history *= f_r * cosine_i / (pdf * 1);
		
	}
	return Lo;
}

vec3 WorldTrace(Ray ray, int depth)
{
	vec3 current_attenuation = vec3(1.0, 1.0, 1.0);
	Ray current_ray = ray;

	while (depth > 0)
	{
		depth--;
		HitResult hit = HitArray(current_ray, 0, triangleCount);
		if (hit.isHit)
		{
			vec3 attenuation;
			Ray scatter_ray;

			current_attenuation *= attenuation;
			current_ray = scatter_ray;
		}
		else
		{
			return current_attenuation;
		}
	}

	return vec3(0.0, 0.0, 0.0);
}

void main()
{
	float u = screenCoord.x;
	float v = screenCoord.y;

	Ray ray;
	ray.start = camera.origin;
	ray.dir = camera.lower_left_corner +
		u * camera.horizontal +
		v * camera.vertical - camera.origin;

	HitResult res = HitArray(ray, 0, triangleCount);
	FragColor.xyz = pathTracing(res, 2);
	FragColor.w = 1.0;
}