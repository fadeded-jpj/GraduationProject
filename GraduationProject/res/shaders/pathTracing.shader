#shader vertex
#version 430 core
layout(location = 0) in vec3 aPos;

out vec2 screenCoord;
out vec3 pix;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	//gl_Position = projection * view * vec4(aPos, 1.0f);
	gl_Position = vec4(aPos, 1.0f);

	screenCoord = (vec2(aPos.x, aPos.y) + 1.0) / 2.0;
	pix = aPos;
}


#shader fragment
#version 430 core
out vec4 FragColor;

in vec2 screenCoord;
in vec3 pix;

#define TRIANGLE_SIEZ 9
#define BVHNODE_SIZE 4
#define INF 114514
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

struct BVHNode {
	int left;
	int right;
	int n;
	int index;
	vec3 AA;
	vec3 BB;
};

// ============= uniform===================
uniform Camera camera;
uniform samplerBuffer triangles;
uniform samplerBuffer bvh;
uniform int triangleCount;
uniform int BVHCount;
uniform uint frameCount;

uniform vec3 eye;
uniform mat4 cameraRotate;
uniform int width;
uniform int height;

// ============= funcion ==================
//=========================== Random =======================
uint seed = uint(
	uint((pix.x * 0.5 + 0.5) * width) * uint(1973) +
	uint((pix.y * 0.5 + 0.5) * height) * uint(9277) +
	uint(frameCount) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float rand() {
	return float(wang_hash(seed)) / 4294967296.0;
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
	if (abs(N.x) > 0.99) 
		helper = vec3(0, 0, 1);
	vec3 tangent = normalize(cross(N, helper));
	vec3 bitangent = normalize(cross(N, tangent));
	return V.x * tangent + V.y * bitangent + V.z * N;
}
//==========================================

// 读取BVH
BVHNode getBVHNode(int i)
{
	int offset = i * BVHNODE_SIZE;
	BVHNode node;

	ivec3 childs = ivec3(texelFetch(bvh, offset + 0).xyz);
	ivec3 leafInfo = ivec3(texelFetch(bvh, offset + 1).xyz);
	node.left = int(childs.x);
	node.right = int(childs.y);
	node.n = int(leafInfo.x);
	node.index = int(leafInfo.y);

	node.AA = texelFetch(bvh, offset + 2).xyz;
	node.BB = texelFetch(bvh, offset + 3).xyz;

	return node;
}

// 与AABB求交
float HitAABB(Ray r, vec3 AA, vec3 BB)
{
	vec3 invDir = 1.0 / r.dir;
	
	vec3 rayIn = (AA - r.start) * invDir;
	vec3 rayOut = (BB - r.start) * invDir;

	vec3 tmax = max(rayIn, rayOut);
	vec3 tmin = min(rayIn, rayOut);

	float tenter = max(tmin.x, max(tmin.y, tmin.z));
	float texit = min(tmax.x, min(tmax.y, tmax.z));

	return (texit >= tenter) ? ((tenter > 0f) ? tenter : texit) : -1f;
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
		N = -N;
		res.isInside = true;
	}

	// 平行三角形
	if (abs(dot(N, D)) < 0.00005f)
		return res;

	float T = (dot(N, p1) - dot(S, N)) / dot(D, N);
	if (T < 0.0005f) return res;

	// 交点
	vec3 P = S + D * T;

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

// 先与BVH求交
HitResult HitBVH(Ray ray)
{
	HitResult res;

	res.isHit = false;
	res.distance = INF;

	//实现栈
	int stack[256];
	int sp = 0;

	stack[sp++] = 1;
	while (sp > 0)
	{
		int top = stack[--sp];
		BVHNode node = getBVHNode(top);

		// 叶子节点
		if (node.n > 0)
		{
			int L = node.index;
			int R = node.index + node.n - 1;

			HitResult r = HitArray(ray, L, R);
			if (r.isHit && r.distance < res.distance)
				res = r;
			continue;
		}

		// 非叶子节点
		float dLeft = INF;	// 左孩子距离
		float dRight = INF;	// 右孩子距离

		if (node.left > 0)
		{
			BVHNode leftNode = getBVHNode(node.left);
			dLeft = HitAABB(ray, leftNode.AA, leftNode.BB);
		}
		if (node.right > 0)
		{
			BVHNode rightNode = getBVHNode(node.right);
			dRight = HitAABB(ray, rightNode.AA, rightNode.BB);
		}

		if (dLeft > 0 && dRight > 0)
		{
			if (dLeft < dRight)
			{
				stack[sp++] = node.right;
				stack[sp++] = node.left;
			}
			else
			{
				stack[sp++] = node.left;
				stack[sp++] = node.right;
			}
		}
		else if (dLeft > 0)
		{
			stack[sp++] = node.left;
		}
		else if (dRight > 0)
		{
			stack[sp++] = node.right;
		}
	}
	return res;
}

vec3 pathTracing(HitResult hit, int maxBounce) {
	vec3 Lo = vec3(0);
	vec3 history = vec3(1);

	while (maxBounce > 0)
	{
		maxBounce--;

		vec3 wi = toNormalHemisphere(SampleHemisphere(), hit.normal);

		Ray ray;
		ray.start = hit.HitPoint;
		ray.dir = wi;

		HitResult newHit = HitBVH(ray);

		if (!newHit.isHit)
			break;

		float pdf = 1.0 / (2.0 * PI);                                   // 半球均匀采样概率密度
		float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // 入射光和法线夹角余弦
		float cosine_i = max(0, dot(ray.dir, hit.normal));				// 出射光和法线夹角余弦
		vec3 f_r = hit.material.baseColor / PI;

		vec3 Le = newHit.material.emissive;
		Lo += history * Le * f_r * cosine_i / (pdf * 1);

		hit = newHit;
		history *= f_r * cosine_i / (pdf * 1);
		
	}
	return Lo;
}

//vec3 WorldTrace(Ray ray, int depth)
//{
//	vec3 current_attenuation = vec3(1.0, 1.0, 1.0);
//	Ray current_ray = ray;
//
//	while (depth > 0)
//	{
//		depth--;
//		HitResult hit = HitBVH(current_ray);
//		if (hit.isHit)
//		{
//			vec3 attenuation;
//			Ray scatter_ray;
//
//			current_attenuation *= attenuation;
//			current_ray = scatter_ray;
//		}
//		else
//		{
//			return current_attenuation;
//		}
//	}
//
//	return vec3(0.0, 0.0, 0.0);
//}

Ray CameraGetRay(Camera camera, vec2 offset)
{
	Ray ray;
	ray.start = camera.origin;
	ray.dir = camera.lower_left_corner +
		offset.x * camera.horizontal +
		offset.y * camera.vertical - camera.origin;

	return ray;
}

void main()
{
	float u = screenCoord.x;
	float v = screenCoord.y;
	
	//Ray ray = CameraGetRay(camera, screenCoord);
	Ray ray;
	ray.start = vec3(0, 0, 4);
	ray.dir = normalize(vec3(pix.xy, 2) - ray.start);

	//===========================
	// TODO : fix BVH bug
	//	BVH 构建 or BVH求交
	//===========================
	

	HitResult res = HitBVH(ray);
	vec3 color;
	
	if (res.isHit) 
	{
		color = res.material.baseColor;
	}
	else
		color = vec3(0);
	
	FragColor = vec4(color, 1);


	/*
	for (int i = 0; i < BVHCount; i++)
	{
		BVHNode node = getBVHNode(i);
		if (node.n > 0)
		{
			int L = node.index;
			int R = node.index + node.n - 1;
			HitResult res = HitArray(ray, L, R);
			if (res.isHit)
				FragColor = vec4(res.material.baseColor, 1);
		}
	}
	*/
}