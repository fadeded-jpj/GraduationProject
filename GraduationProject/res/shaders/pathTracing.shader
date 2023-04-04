#shader vertex
#version 330 core
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
#version 330 core
out vec4 FragColor;

in vec2 screenCoord;
in vec3 pix;

#define TRIANGLE_SIEZ 12
#define BVHNODE_SIZE 4
#define INF 114514
#define PI 3.1415926535859

// =========== Struct=============
struct Material {
	vec3 emissive;
	vec3 baseColor;

	float subsurface;
	float roughness;
	float metallic;

	float specular;
	float specularTint;
	float anisotropic;

	float sheen;
	float sheenTine;
	float clearcoat;

	float clearcoatGloss;
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
uniform int frameCount;
uniform sampler2D lastFrame;

uniform vec3 eye;
uniform mat4 cameraRotate;
uniform int width;
uniform int height;

// ============= funcion ==================
//=========================== Random =======================
uint seed = uint(
	uint(screenCoord.x * width) * uint(1973) +
	uint(screenCoord.y * height) * uint(9277) +
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

// --------- Sample----------------
vec3 myMul(vec3 v, mat3 m)
{
	return v.x * m[0] + v.y * m[1] + v.z* m[2];
}

float sdot(vec3 x, vec3 y, float f = 1.0f)
{
	return clamp(dot(x, y) * f, 0.0, 1.0);
}

mat3 GetTangentSpace(vec3 N)
{
	vec3 helper = vec3(1, 0, 0);
	if (abs(N.x) > 0.99)
		helper = vec3(0, 0, 1);
	
	vec3 tangent = normalize(cross(N, helper));
	vec3 binormal = normalize(cross(N, tangent));
	return mat3(tangent, binormal, N);
}

vec3 SampleHemisphere(vec3 normal) {
	float z = rand();
	float r = max(0, sqrt(1 - z * z));
	float phi = 2.0 * PI * rand();

	vec3 tangentSpaceDir = vec3(r * cos(phi), r * sin(phi), z);

	return myMul(tangentSpaceDir, GetTangentSpace(normal));
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

vec3 SampleHemisphereRand()
{
	float z = rand();
	float r = max(0, sqrt(1.0 - z * z));
	float phi = 2.0 * PI * rand();
	return vec3(r * cos(phi), r * sin(phi), z);
}
//==========================================



// ��ȡBVH
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

// ��AABB��
float HitAABB(Ray r, vec3 AA, vec3 BB)
{
	//vec3 invDir = 1.0 / r.dir;
	//
	//vec3 rayIn = (AA - r.start) * invDir;
	//vec3 rayOut = (BB - r.start) * invDir;

	//vec3 tmax = max(rayIn, rayOut);
	//vec3 tmin = min(rayIn, rayOut);

	//float tenter = max(tmin.x, max(tmin.y, tmin.z));
	//float texit = min(tmax.x, min(tmax.y, tmax.z));

	//return tenter < texit && texit >= 0;

	vec3 invdir = 1.0 / r.dir;

	vec3 f = (BB - r.start) * invdir;
	vec3 n = (AA - r.start) * invdir;

	vec3 tmax = max(f, n);
	vec3 tmin = min(f, n);

	float t1 = min(tmax.x, min(tmax.y, tmax.z));
	float t0 = max(tmin.x, max(tmin.y, tmin.z));

	return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
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
	vec3 param1 = texelFetch(triangles, offset + 8).xyz;
	vec3 param2 = texelFetch(triangles, offset + 9).xyz;
	vec3 param3 = texelFetch(triangles, offset + 10).xyz;
	vec3 param4 = texelFetch(triangles, offset + 11).xyz;

	res.subsurface = param1.x;
	res.roughness = param1.y;
	res.metallic = param1.z;

	res.specular = param2.x;
	res.specularTint = param2.y;
	res.anisotropic = param2.z;
	
	res.sheen = param3.x;
	res.sheenTine = param3.y;
	res.clearcoat = param3.z;
	
	res.clearcoatGloss = param4.x;

	return res;
}

// ���ߺ��������ཻ
HitResult HitTriangle(Triangle t, Ray ray)
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
	//vec3 N = normalize(t.n1 + t.n2 + t.n3);

	// ���ߴӱ���ﵽ����
	if (dot(N, D) > 0.0f) {
		N = -N;
		res.isInside = true;
	}

	// ƽ��������
	if (abs(dot(N, D)) < 0.00001f)
		return res;

	// �󽻵�
	float T = (dot(N, p1) - dot(S, N)) / dot(D, N);
	if (T < 0.0005f) return res;

	// ����
	vec3 P = S + D * T;

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
		res.normal = N;

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

		HitResult tmp = HitTriangle(t, ray);
		if (tmp.isHit && tmp.distance < res.distance) {
			res = tmp;
			res.material = getMaterial(i);
		}
	}
	return res;
}

// ����BVH��
HitResult HitBVH(Ray ray)
{
	HitResult res;

	res.isHit = false;
	res.distance = INF;

	//ʵ��ջ
	int stack[256];
	int sp = 0;

	stack[sp++] = 1;
	while (sp > 0)
	{
		int top = stack[--sp];
		BVHNode node = getBVHNode(top);

		// Ҷ�ӽڵ�
		if (node.n > 0)
		{
			int L = node.index;
			int R = node.index + node.n - 1;

			HitResult r = HitArray(ray, L, R);
			if (r.isHit && r.distance < res.distance)
				res = r;
			continue;
		}

		// ��Ҷ�ӽڵ�
		float dLeft = INF;// ���Ӿ���
		float dRight = INF;	// �Һ��Ӿ���

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
			stack[sp++] = node.right;
			stack[sp++] = node.left;
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


//============PRB===================
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float schlickFresnel(float cosTheta)
{
	return pow(clamp(1 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 PBR(vec3 V, vec3 N, vec3 L, in Material material)
{
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);
	if (NdotL < 0 || NdotV < 0) 
		return vec3(0);

	vec3 H = normalize(L + V);
	float NdotH = dot(N, H);
	float LdotH = dot(L, H);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, material.baseColor, material.metallic);
	// ��������
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

	float NDF = DistributionGGX(N, H, material.roughness);
	float G = GeometrySmith(N, V, L, material.roughness);

	vec3 nominator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
	vec3 specular = nominator / denominator;

	vec3 ks = F;
	vec3 kd = vec3(1.0) - ks;

	kd *= 1.0 - material.metallic;

	return kd * material.baseColor / PI + specular;
}

float SchlickFresnel(float u)
{
	float m = clamp(1 - u, 0, 1);
	float m2 = m * m;
	return m2 * m2 * m; // pow(m,5)
}

float GTR1(float NdotH, float a) {
	if (a >= 1) return 1 / PI;
	float a2 = a * a;
	float t = 1 + (a2 - 1) * NdotH * NdotH;
	return (a2 - 1) / (PI * log(a2) * t);
}

float GTR2(float NdotH, float a) {
	float a2 = a * a;
	float t = 1 + (a2 - 1) * NdotH * NdotH;
	return a2 / (PI * t * t);
}

float smithG_GGX(float NdotV, float alphaG) {
	float a = alphaG * alphaG;
	float b = NdotV * NdotV;
	return 1 / (NdotV + sqrt(a + b - a * b));
}

vec3 mon2lin(vec3 x)
{
	return vec3(pow(x[0], 2.2), pow(x[1], 2.2), pow(x[2], 2.2));
}

vec3 Disney_BRDF(vec3 V, vec3 N, vec3 L, in Material material)
{
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);
	if (NdotL < 0 || NdotV < 0)
		return vec3(0);

	vec3 H = normalize(L + V);
	float NdotH = dot(N, H);
	float LdotH = dot(L, H);
	
	vec3 Cdlin = mon2lin(material.baseColor);

	// diffuse
	float Fd90 = 0.5 + 2.0 * material.roughness * LdotH * LdotH;
	float FL = schlickFresnel(NdotL);
	float FV = schlickFresnel(NdotV);

	float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

	float Fss90 = LdotH * LdotH * material.roughness;
	float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
	float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

	vec3 diffuse = Cdlin / PI * mix(Fd, ss, material.subsurface) * (1 - material.metallic);
	
	// specular
	float Cdlum = 0.3 * Cdlin.x + 0.6 * Cdlin.y + 0.1 * Cdlin.z;
	vec3 Ctint = (Cdlum) > 0 ? (Cdlin / Cdlum) : vec3(1);
	vec3 Cspec = material.specular * mix(vec3(1), Ctint, material.specularTint);
	vec3 Cspec0 = mix(0.08 * Cspec, Cdlin, material.metallic);
	
	float alpha = material.roughness * material.roughness;
	float Ds = GTR2(NdotH, alpha);
	float FH = SchlickFresnel(LdotH);
	vec3 Fs = mix(Cspec0, vec3(1), FH);
	float Gs = smithG_GGX(NdotL, material.roughness);
	Gs *= smithG_GGX(NdotV, material.roughness);

	vec3 specular = Gs * Fs * Ds;

	return diffuse + specular;
}

//============================
vec3 pathTracing(HitResult hit, float RR) {
	vec3 Lo = vec3(0);
	vec3 history = vec3(1);

	float P = rand();

	// RR �㷨
	while (P < RR)
	{
		//maxBounce--;
		P = rand();

		//vec3 wi = SampleHemisphereRand();
		vec3 wi = SampleHemisphere(hit.normal);

		Ray ray;
		ray.start = hit.HitPoint;
		ray.dir = wi;

		HitResult newHit = HitBVH(ray);
		//HitResult newHit = HitArray(ray, 0, triangleCount - 1);


		if (!newHit.isHit)
			break;

		float pdf = 1.0 / (2.0 * PI);                                   // ������Ȳ��������ܶ�
		float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // �����ͷ��߼н�����
		float cosine_i = max(0, dot(ray.dir, hit.normal));				// �����ͷ��߼н�����
		
		vec3 V = -hit.viewDir;
		vec3 N = hit.normal;

		//vec3 f_r = Disney_BRDF(V, N, wi, hit.material);
		vec3 f_r = PBR(V, N, wi, hit.material);

		vec3 Le = newHit.material.emissive;

		history *= f_r * cosine_i / pdf;
	
		Lo += history * Le;
		hit = newHit;

	}
	return Lo / RR;
}

vec3 rayTracing(HitResult hit, int maxBounce) {
	vec3 Lo = vec3(0);
	vec3 history = vec3(1);

	while (maxBounce > 0)
	{
		maxBounce--;

		//vec3 wi = toNormalHemisphere(SampleHemisphereRand(), hit.normal);
		vec3 wi = SampleHemisphere(hit.normal);

		Ray ray;
		ray.start = hit.HitPoint;
		ray.dir = wi;

		HitResult newHit = HitBVH(ray);
		//HitResult newHit = HitArray(ray, 0, triangleCount - 1);

		if (!newHit.isHit)
			break;

		float pdf = 1.0 / (2.0 * PI);                                   // ������Ȳ��������ܶ�
		float cosine_o = max(0, dot(-hit.viewDir, hit.normal));         // �����ͷ��߼н�����
		float cosine_i = max(0, dot(ray.dir, hit.normal));				// �����ͷ��߼н�����

		vec3 V = -hit.viewDir;
		vec3 N = hit.normal;

		vec3 f_r = Disney_BRDF(V, N, wi, hit.material);
		//vec3 f_r = PBR(V, N, wi, hit.material);

		vec3 Le = newHit.material.emissive;
		Lo += history * Le * f_r * cosine_i / (pdf);

		hit = newHit;
		history *= f_r * cosine_i / (pdf);

	}
	return Lo;
}


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
	
	Ray ray = CameraGetRay(camera, screenCoord);


	//===========================
	// TODO : fix BVH bug
	//	BVH ���䣿
	//===========================
	
	HitResult r = HitBVH(ray);
	//HitResult r = HitArray(ray, 0, triangleCount - 1);

	vec3 color = vec3(0);
	
	int spp = 32;
	for (int i = 0; i < spp; i++) {
		if (r.isHit)
		{
			color += r.material.emissive + pathTracing(r, 0.8);
		}
	}
	color = color / spp * 8;
	// ����һ���
	vec3 lastColor = texture(lastFrame, screenCoord.xy).rgb;
	color = mix(lastColor, color, 1.0 / float(frameCount + 1));
	
	//color = vec3(r.material.specular);
	
	FragColor = vec4(color, 1);
}