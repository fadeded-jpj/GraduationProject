#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 pix;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	pix = aPos;
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}


#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 pix;

#define TRIANGLE_SIEZ 9
#define INF 12138

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

//============ Plan ==============
struct Model {
	int begin, end; // 模型三角形的起始和最后

	// TODO: AABB
};

uniform uint frameCount;
uniform vec2 screenSize;

uint wseed = frameCount;

//================================

uniform samplerBuffer triangles;
uniform int nums;
uniform vec3 cameraPos;

//=========================== Random =======================
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

//===========================================================


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

// 光线和三角形相交
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

	// 光线从背面达到物体
	if (dot(N, D) > 0.0f) {
		res.isInside = true;
		N = -N;
	}

	// 平行三角形
	if (abs(dot(N, D)) < 0.0005f)
		return res;

	float T = (dot(N, p1) - dot(N, S)) / dot(N, D);
	if (T < 0.0005f) return res;

	// 交点
	vec3 P = S + T * D;

	// 判断是否在三角形内
	vec3 c1 = cross(p2 - p1, P - p1);
	vec3 c2 = cross(p3 - p2, P - p2);
	vec3 c3 = cross(p1 - p3, P - p3);
	bool res1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
	bool res2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

	//命中
	if (res1 || res2)
	{
		res.isHit = true;
		res.distance = T;
		res.HitPoint = P;
		res.viewDir = D;
		
		//法线插值
		float alpha = (-(P.x - p2.x) * (p3.y - p2.y) + (P.y - p2.y) * (p3.x - p2.x)) / (-(p1.x - p2.x - 0.00005) * (p3.y - p2.y + 0.00005) + (p1.y - p2.y + 0.00005) * (p3.x - p2.x + 0.00005));
		float beta = (-(P.x - p3.x) * (p1.y - p3.y) + (P.y - p3.y) * (p1.x - p3.x)) / (-(p2.x - p3.x - 0.00005) * (p1.y - p3.y + 0.00005) + (p2.y - p3.y + 0.00005) * (p1.x - p3.x + 0.00005));
		float gama = 1.0 - alpha - beta;
		vec3 finalNormal = normalize(alpha * t.n1 + beta * t.n2 + gama * t.n3);
		res.normal = res.isInside ? -finalNormal : finalNormal;
	}

	return res;
}

// 遍历求最近的相交三角形
HitResult HitArray(Ray ray, int left, int right)
{
	HitResult res;
	res.isHit = false;
	res.distance = INF;
	
	for (int i = left; i < right; i++)
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

void main()
{
	Ray ray;
	ray.start = vec3(5, 5, 5);
	vec3 dir = vec3(0.0, 0.0, 0.0) - ray.start;
	ray.dir = normalize(dir);
	
	HitResult res = HitArray(ray, 0, nums);

	vec3 color = randVec3();

	FragColor = vec4(color, 1.0f);
}
