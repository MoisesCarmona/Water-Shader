#pragma once

#include <assert.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <fbxsdk.h>
#include "DDSTextureLoader.h"
#include "Camera.h"
#include <cstdint>
#include <array>
#include "debug_renderer.h"
#include <DirectXMath.h>
#include"Water.h"

using namespace debug_renderer;
using namespace std;

//template<typename T>
//void SafeRelease(T item)
//{
//	if (item)
//
//		item->Release();
//
//}
struct WVP
{

	XMMATRIX worldMatrix = {}; //XMFLOAT4X4 worldMatrix; // storage type
	XMMATRIX viewMatrix = {};
	XMMATRIX projMatrix = {};
	XMVECTOR cam = {};
	//XMFLOAT4X4 AnimJoint[30];

}My_Matrices;

struct LBF
{
	XMVECTOR ambientColor = { 1,1,1 ,1};
	XMVECTOR diffuseColor = { 1,1,1 , 1};
	XMVECTOR lightDirection = { 0,-1,0 };


}lightColorDir;
struct ReflectionBufferType
{
	XMMATRIX reflection;

}RBT;

struct WaterBufferType
{
	float waterTranslation = 0;
	float reflectRefractScale = 0.0001f;
	XMVECTOR padding;

}WBT;

class Object
{
public:
	ID3D11VertexShader* VShader;
	ID3D11PixelShader* PShader;
	ID3D11Buffer* VBuffer;
	ID3D11Buffer* IBuffer;
	ID3D11InputLayout* VLayout;
	ID3D11Buffer* constantBuffer;
	ID3D11Texture2D* diffuseTexture;
	ID3D11Texture2D* emissiveTexture;
	ID3D11Texture2D* specularTexture;
	ID3D11Texture2D* SkyCubeTexture;
	ID3D11Texture2D* NormalTexture;
	ID3D11Texture2D* RTT_Texture;

	ID3D11ShaderResourceView* TextureView_Diffuse;
	ID3D11ShaderResourceView* TextureView_Emissive;
	ID3D11ShaderResourceView* TextureView_Specular;
	ID3D11ShaderResourceView* TextureView_SkyBox;
	ID3D11ShaderResourceView* TextureView_Normal;
	ID3D11ShaderResourceView* TextureView_RTT;



	
	ID3D11RenderTargetView* RTTRenderTV;
	D3D11_VIEWPORT RTT_viewport;
	ID3D11DepthStencilView* zBufferRTT;


	enum Tex { DIFFUSE, EMISSIVE, SPECULAR };
	UINT IndexCount = 0;
	WVP wvp;

	Object()
	{

		VShader = nullptr;
		PShader = nullptr;
		VBuffer = nullptr;
		IBuffer = nullptr;
		VLayout = nullptr;
		constantBuffer = nullptr;
		diffuseTexture = nullptr;
		emissiveTexture = nullptr;
		specularTexture = nullptr;
		SkyCubeTexture = nullptr;
		NormalTexture = nullptr;
		TextureView_Diffuse = nullptr;
		TextureView_Emissive = nullptr;
		TextureView_Specular = nullptr;
		TextureView_SkyBox = nullptr;
		TextureView_Normal = nullptr;
		zBufferRTT = nullptr;

	}
	void Release()
	{

		SafeRelease(VShader);
		SafeRelease(PShader);
		SafeRelease(VBuffer);
		SafeRelease(IBuffer);
		SafeRelease(VLayout);
		SafeRelease(constantBuffer);

		SafeRelease(diffuseTexture);
		SafeRelease(emissiveTexture);
		SafeRelease(specularTexture);
		SafeRelease(SkyCubeTexture);
		SafeRelease(NormalTexture);


		SafeRelease(TextureView_Diffuse);
		SafeRelease(TextureView_Emissive);
		SafeRelease(TextureView_Specular);
		SafeRelease(TextureView_SkyBox);
		SafeRelease(TextureView_Normal);

		SafeRelease(TextureView_RTT);
		SafeRelease(RTT_Texture);
		SafeRelease(RTTRenderTV);
		SafeRelease(zBufferRTT);



	}

	void RenderToTexture(ID3D11Device* g_Device, int textureWidth, int textureHeight)
	{
		D3D11_TEXTURE2D_DESC tDesc;
		ZeroMemory(&tDesc, sizeof(tDesc));
		tDesc.ArraySize = 1;
		tDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		tDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		tDesc.Height = textureHeight;
		tDesc.Width = textureWidth;
		tDesc.Usage = D3D11_USAGE_DEFAULT;
		tDesc.SampleDesc.Count = 1;
		tDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		g_Device->CreateTexture2D(&tDesc, nullptr, &RTT_Texture);


		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;

		g_Device->CreateRenderTargetView(RTT_Texture, &RTVDesc, &RTTRenderTV);

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		g_Device->CreateShaderResourceView(RTT_Texture, &SRVDesc, &TextureView_RTT);

		D3D11_TEXTURE2D_DESC depthTextureDesc = { 0 };
		depthTextureDesc.Width = textureWidth;
		depthTextureDesc.Height = textureHeight;
		depthTextureDesc.ArraySize = 1;
		depthTextureDesc.MipLevels = 1;
		depthTextureDesc.SampleDesc.Count = 1;
		depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		ID3D11Texture2D* depthBuffer;
		g_Device->CreateTexture2D(&depthTextureDesc, NULL, &depthBuffer);

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		g_Device->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &zBufferRTT);
		depthBuffer->Release();

		RTT_viewport.TopLeftX = RTT_viewport.TopLeftY = 0;
		RTT_viewport.Width = textureWidth;
		RTT_viewport.Height = textureHeight;
		RTT_viewport.MinDepth = 0.0f;
		RTT_viewport.MaxDepth = 1.0f;
		 

	}



};




using file_path_t = std::array<char, 260>; // Simple max size file path string
// Simple material definition
struct material_t
{
public:
	enum e_component { EMISSIVE = 0, DIFFUSE, SPECULAR, SHININESS, COUNT };
	struct component_t
	{
		float value[3] = { 0.0f, 0.0f, 0.0f };
		float factor = 0.0f;
		int64_t input = -1;
	};
	component_t& operator[](int i) { return components[i]; }
	const component_t& operator[](int i)const { return components[i]; }
private:
	component_t components[COUNT];
};



template<typename T>
Object InitializeModelData(ID3D11Device* g_Device, vector<T> mVert, vector<int> mIndices,
	const void* vertexShader, const void* pixelShader, SIZE_T vertShaderSize, SIZE_T pixelShadersize,
	D3D11_INPUT_ELEMENT_DESC inputEDesc[], UINT NumElements, vector<const wchar_t*> DDSTextures)
{
	Object model;
	D3D11_BUFFER_DESC bDesc;
	//Load Mesh onto Card
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(T) * mVert.size();
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = mVert.data();

	g_Device->CreateBuffer(&bDesc, &subData, &model.VBuffer);

	//Index Buffer
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * mIndices.size();

	subData.pSysMem = mIndices.data();

	g_Device->CreateBuffer(&bDesc, &subData, &model.IBuffer);

	HRESULT hr = g_Device->CreateVertexShader(vertexShader, (vertShaderSize), nullptr, &model.VShader);
	hr = g_Device->CreatePixelShader(pixelShader, (pixelShadersize), nullptr, &model.PShader);

	g_Device->CreateInputLayout(inputEDesc, NumElements, vertexShader, vertShaderSize, &model.VLayout);
	//create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bDesc.ByteWidth = sizeof(GMATRIXF);//sizeof(XMFLOAT4X4);
	bDesc.ByteWidth = sizeof(WVP);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	subData.pSysMem = 0;

	g_Device->CreateBuffer(&bDesc, nullptr, &model.constantBuffer);
	if (DDSTextures.size() != 0)
	{
		CreateDDSTextureFromFile(g_Device, DDSTextures[model.DIFFUSE], (ID3D11Resource**)&model.diffuseTexture, &model.TextureView_Diffuse);
		if (DDSTextures.size() > 1)
		{
			if (DDSTextures[model.EMISSIVE] != nullptr)
			{
				CreateDDSTextureFromFile(g_Device, DDSTextures[model.EMISSIVE], (ID3D11Resource**)&model.emissiveTexture, &model.TextureView_Emissive);
			}
			if (DDSTextures[model.SPECULAR] != nullptr)
			{
				CreateDDSTextureFromFile(g_Device, DDSTextures[model.SPECULAR], (ID3D11Resource**)&model.specularTexture, &model.TextureView_Specular);
			}

		}
	}

	model.IndexCount = mIndices.size();
	return model;

}





vector<XMFLOAT3> positionList;
vector<XMFLOAT3> normalList;
vector<XMFLOAT3> uvList;
vector< int > positionIndices, uvIndices, normalIndices;

struct SimpleVertex
{

	XMFLOAT3 Position;
	XMFLOAT3 Tex;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
};
struct SimpleVertexAnim
{
	XMFLOAT3 Pos;
	XMFLOAT3 Tex;
	XMFLOAT3 Normal;
	XMINT4 Joints;
	XMFLOAT4 Weights;

};

struct SimpleMesh
{
	vector<SimpleVertex> vertexList;
	vector<int> indicesList;
};
struct SimpleMeshAnim
{
	vector<SimpleVertexAnim> vertexList;
	vector<int> indicesList;
};


#pragma region OBJ
bool Process_OBJ(const char* objFileName, const char* meshfile)
{
	SimpleMesh mesh;


	FILE* file = fopen(objFileName, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1)
	{

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0)
		{
			XMFLOAT3 position;
			fscanf(file, "%f %f %f\n", &position.x, &position.y, &position.z);
			positionList.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			XMFLOAT3 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.z = 0.0f;
			uvList.push_back(uv);

		}
		else if (strcmp(lineHeader, "vn") == 0) {
			XMFLOAT3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			normalList.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}

			SimpleVertex v;
			v.Position = positionList[vertexIndex[0] - 1];
			v.Tex = uvList[uvIndex[0] - 1];
			v.Normal = normalList[normalIndex[0] - 1];

			mesh.vertexList.push_back(v);

			v.Position = positionList[vertexIndex[1] - 1];
			v.Tex = uvList[uvIndex[1] - 1];
			v.Normal = normalList[normalIndex[1] - 1];

			mesh.vertexList.push_back(v);

			v.Position = positionList[vertexIndex[2] - 1];
			v.Tex = uvList[uvIndex[2] - 1];
			v.Normal = normalList[normalIndex[2] - 1];

			mesh.vertexList.push_back(v);







			//positionIndices.push_back(vertexIndex[0]);
			//positionIndices.push_back(vertexIndex[1]);
			//positionIndices.push_back(vertexIndex[2]);
			//uvIndices.push_back(uvIndex[0]);
			//uvIndices.push_back(uvIndex[1]);
			//uvIndices.push_back(uvIndex[2]);
			//normalIndices.push_back(normalIndex[0]);
			//normalIndices.push_back(normalIndex[1]);
			//normalIndices.push_back(normalIndex[2]);
		}




	}

	for (int i = 0; i < mesh.vertexList.size(); i++)
	{
		mesh.indicesList.push_back(i);
	}

	std::ofstream fileWrite(meshfile, std::ios::trunc | std::ios::binary | std::ios::out);

	assert(fileWrite.is_open());

	uint32_t index_count = (uint32_t)mesh.indicesList.size();
	uint32_t vert_count = (uint32_t)mesh.vertexList.size();

	fileWrite.write((const char*)&index_count, sizeof(uint32_t));
	fileWrite.write((const char*)mesh.indicesList.data(), sizeof(uint32_t) * mesh.indicesList.size());
	fileWrite.write((const char*)&vert_count, sizeof(uint32_t));
	fileWrite.write((const char*)mesh.vertexList.data(), sizeof(SimpleVertex) * mesh.vertexList.size());

	fileWrite.close();


}
void Load_OBJ(const char* meshfile, SimpleMesh& mesh)
{


	std::fstream file{ meshfile, std::ios_base::in | std::ios_base::binary };

	assert(file.is_open());

	uint32_t player_index_count;
	file.read((char*)&player_index_count, sizeof(uint32_t));

	mesh.indicesList.resize(player_index_count);

	file.read((char*)mesh.indicesList.data(), sizeof(uint32_t) * player_index_count);

	uint32_t player_vertex_count;
	file.read((char*)&player_vertex_count, sizeof(uint32_t));

	mesh.vertexList.resize(player_vertex_count);

	file.read((char*)mesh.vertexList.data(), sizeof(SimpleVertex) * player_vertex_count);
	file.close();


}
#pragma endregion


#pragma region FBX
struct fbx_joint
{
	FbxNode* node; XMMATRIX global_xform; int parent_index; int childCount;

};
struct keyframe
{
	double time; vector<XMMATRIX> jointsMatrix; vector<int> parents;
};
struct anim_clip
{
	double duration;
	vector<keyframe> frames;
};
struct vert_pos_skinned
{
	XMVECTOR pos;
	int joints[4];
	XMVECTOR weights;


};
const int MAX_INFLUENCES = 4;
struct influence
{
	int joint; float weight;
};

using influence_set = array<influence, MAX_INFLUENCES>;
vector<influence_set> control_point_influences;

class Skeleton
{
public:
	ID3D11VertexShader* VShader;
	ID3D11PixelShader* PShader;
	ID3D11Buffer* VBuffer;
	ID3D11InputLayout* VLayout;
	ID3D11Buffer* constantBuffer;
	WVP wvp;
	vector<fbx_joint> joints;
	anim_clip Animation_Clip;



	Skeleton()
	{

		VShader = nullptr;
		PShader = nullptr;
		VBuffer = nullptr;
	}

	void Release()
	{

		SafeRelease(VShader);
		SafeRelease(PShader);
		SafeRelease(VBuffer);
		SafeRelease(VLayout);
		SafeRelease(constantBuffer);


		VLayout = nullptr;
		constantBuffer = nullptr;
	};



};
class ObjectAnim
{
public:
	ID3D11VertexShader* VShader;
	ID3D11PixelShader* PShader;
	ID3D11Buffer* VBuffer;
	ID3D11Buffer* IBuffer;
	ID3D11InputLayout* VLayout;
	ID3D11Buffer* constantBuffer;
	ID3D11Texture2D* diffuseTexture;
	ID3D11Texture2D* emissiveTexture;
	ID3D11Texture2D* specularTexture;

	ID3D11ShaderResourceView* TextureView_Diffuse;
	ID3D11ShaderResourceView* TextureView_Emissive;
	ID3D11ShaderResourceView* TextureView_Specular;

	vector<fbx_joint> joints;
	anim_clip Animation_Clip;

	enum Tex { DIFFUSE, EMISSIVE, SPECULAR };
	UINT IndexCount = 0;
	WVP wvp;

	ObjectAnim()
	{

		VShader = nullptr;
		PShader = nullptr;
		VBuffer = nullptr;
		IBuffer = nullptr;
		VLayout = nullptr;
		constantBuffer = nullptr;
		diffuseTexture = nullptr;
		emissiveTexture = nullptr;
		specularTexture = nullptr;
		TextureView_Diffuse = nullptr;
		TextureView_Emissive = nullptr;
		TextureView_Specular = nullptr;

	}


	void Release()
	{

		SafeRelease(VShader);
		SafeRelease(PShader);
		SafeRelease(VBuffer);
		SafeRelease(IBuffer);
		SafeRelease(VLayout);
		SafeRelease(constantBuffer);

		SafeRelease(diffuseTexture);
		SafeRelease(emissiveTexture);
		SafeRelease(specularTexture);


		SafeRelease(TextureView_Diffuse);
		SafeRelease(TextureView_Emissive);
		SafeRelease(TextureView_Specular);








	}


};


Skeleton InitializeBoneData(ID3D11Device* g_Device, vector<fbx_joint> mJoint,
	const void* vertexShader, const void* pixelShader, SIZE_T vertShaderSize, SIZE_T pixelShadersize,
	D3D11_INPUT_ELEMENT_DESC inputEDesc[], UINT NumElements, anim_clip& clip)
{

	Skeleton skeleton;
	skeleton.Animation_Clip = clip;

	skeleton.Animation_Clip.duration = clip.duration;

	skeleton.Animation_Clip.frames.resize(clip.frames.size());


	for (int i = 0; i < clip.frames.size(); i++)
	{
		skeleton.Animation_Clip.frames[i].time = clip.frames[i].time;


		for (size_t j = 0; j < skeleton.Animation_Clip.frames[i].jointsMatrix.size(); j++)
		{
			skeleton.Animation_Clip.frames[i].jointsMatrix[j] = clip.frames[i].jointsMatrix[j];
			debug_renderer::draw_matrix(clip.frames[i].jointsMatrix[j]);
		}

	}





	skeleton.joints = mJoint;
	D3D11_BUFFER_DESC bDesc;
	//Load Mesh onto Card
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(colored_vertex) * get_line_vert_capacity();
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DEFAULT;

	subData.pSysMem = get_line_verts();

	g_Device->CreateBuffer(&bDesc, &subData, &skeleton.VBuffer);
	debug_renderer::clear_lines();

	HRESULT hr = g_Device->CreateVertexShader(vertexShader, (vertShaderSize), nullptr, &skeleton.VShader);
	hr = g_Device->CreatePixelShader(pixelShader, (pixelShadersize), nullptr, &skeleton.PShader);

	g_Device->CreateInputLayout(inputEDesc, NumElements, vertexShader, vertShaderSize, &skeleton.VLayout);
	//create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bDesc.ByteWidth = sizeof(GMATRIXF);//sizeof(XMFLOAT4X4);
	bDesc.ByteWidth = sizeof(WVP);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	subData.pSysMem = 0;

	g_Device->CreateBuffer(&bDesc, nullptr, &skeleton.constantBuffer);



	return skeleton;

}


template<typename T>
ObjectAnim InitializeAnimModelData(ID3D11Device* g_Device, vector<T> mVert, vector<int> mIndices, vector<fbx_joint> mJoint,
	const void* vertexShader, const void* pixelShader, SIZE_T vertShaderSize, SIZE_T pixelShadersize,
	D3D11_INPUT_ELEMENT_DESC inputEDesc[], UINT NumElements, vector<const wchar_t*> DDSTextures, anim_clip& clip)
{
	ObjectAnim model;
	model.Animation_Clip = clip;

	model.Animation_Clip.duration = clip.duration;

	model.Animation_Clip.frames.resize(clip.frames.size());


	for (int i = 0; i < clip.frames.size(); i++)
	{
		model.Animation_Clip.frames[i].time = clip.frames[i].time;


		for (size_t j = 0; j < model.Animation_Clip.frames[i].jointsMatrix.size(); j++)
		{
			model.Animation_Clip.frames[i].jointsMatrix[j] = clip.frames[i].jointsMatrix[j];
			//debug_renderer::draw_matrix(clip.frames[i].jointsMatrix[j]);
		}

	}
	model.joints = mJoint;
	D3D11_BUFFER_DESC bDesc;
	//Load Mesh onto Card
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));
	bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bDesc.ByteWidth = sizeof(T) * mVert.size();
	bDesc.CPUAccessFlags = 0;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_IMMUTABLE;

	subData.pSysMem = mVert.data();

	g_Device->CreateBuffer(&bDesc, &subData, &model.VBuffer);

	//Index Buffer
	bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bDesc.ByteWidth = sizeof(int) * mIndices.size();

	subData.pSysMem = mIndices.data();

	g_Device->CreateBuffer(&bDesc, &subData, &model.IBuffer);

	HRESULT hr = g_Device->CreateVertexShader(vertexShader, (vertShaderSize), nullptr, &model.VShader);
	hr = g_Device->CreatePixelShader(pixelShader, (pixelShadersize), nullptr, &model.PShader);

	g_Device->CreateInputLayout(inputEDesc, NumElements, vertexShader, vertShaderSize, &model.VLayout);
	//create constant buffer
	ZeroMemory(&bDesc, sizeof(bDesc));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bDesc.ByteWidth = sizeof(GMATRIXF);//sizeof(XMFLOAT4X4);
	bDesc.ByteWidth = sizeof(WVP);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	subData.pSysMem = 0;

	g_Device->CreateBuffer(&bDesc, nullptr, &model.constantBuffer);

	CreateDDSTextureFromFile(g_Device, DDSTextures[model.DIFFUSE], (ID3D11Resource**)&model.diffuseTexture, &model.TextureView_Diffuse);
	if (DDSTextures.size() > 1)
	{
		if (DDSTextures[model.EMISSIVE] != nullptr)
		{
			CreateDDSTextureFromFile(g_Device, DDSTextures[model.EMISSIVE], (ID3D11Resource**)&model.emissiveTexture, &model.TextureView_Emissive);
		}
		if (DDSTextures[model.SPECULAR] != nullptr)
		{
			CreateDDSTextureFromFile(g_Device, DDSTextures[model.SPECULAR], (ID3D11Resource**)&model.specularTexture, &model.TextureView_Specular);
		}

	}

	model.IndexCount = mIndices.size();
	return model;

}


vector<fbx_joint> joints;

SimpleVertexAnim* verticesAnim;
SimpleVertex* vertices;

int numVertices = 0;
int* indices;
int numIndices = 0;
float scale = 1.0f;

std::vector<material_t> materials;
std::vector<file_path_t> paths;

void ProcessFbxMesh(FbxNode* Node, const char* meshfile, const char* matpath = "", const char* matfile = "");
void Compactify(SimpleVertex* verticesCompact, const char* meshfile);
void CompactifyAnim(SimpleVertexAnim* verticesCompact, const char* meshfile);

void ProcessFbxSkeleton(FbxNode* Node, int parentindex);
void WriteOutSkeletonAnim(const char* AnimFile);
void GetClusters(FbxSkin* skin);
XMMATRIX FBXAMatrix_To_XMMATRIX(FbxAMatrix& m);



void  FBX_InitLoad(const char* fbxfile, const char* meshfile, const char* matPath, const char* matfile)
{

	// Change the following filename to a suitable filename value.
	const char* lFilename = fbxfile;

	// Initialize the SDK manager. This object handles memory management.
	FbxManager* lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
	}
	// Create a new scene so that it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();


	// Process the scene and build DirectX Arrays
	ProcessFbxMesh(lScene->GetRootNode(), meshfile, matPath, matfile);




};

void Anim_FBXSkeleton_InitLoad(const char* fbxfile, const char* AnimFile, anim_clip& out_clip)
{
	std::fstream skelefile{ "ObjectData/Test.Anim", std::ios_base::in | std::ios_base::binary };

	// Change the following filename to a suitable filename value.
	const char* lFilename = fbxfile;

	// Initialize the SDK manager. This object handles memory management.
	FbxManager* lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
	}
	// Create a new scene so that it can be populated by the imported file.
	FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();

	//Skeleton
	for (unsigned int j = 0; j < lScene->GetPoseCount(); j++)
	{


		FbxPose* bindPose = lScene->GetPose(j);
		if (bindPose->IsBindPose())
		{
			for (unsigned int i = 0; i < bindPose->GetCount(); i++)
			{


				FbxNode* skeletonNode = bindPose->GetNode(i);



				FbxSkeleton* skeleton = skeletonNode->GetSkeleton();
				if (skeleton != NULL)
				{
					if (skeleton->IsSkeletonRoot())
					{
						ProcessFbxSkeleton(skeletonNode, -1);
					}
				}
			}
		}
	}
	//Animate Model


	for (unsigned int j = 0; j < lScene->GetPoseCount(); j++)
	{


		FbxPose* bindPose = lScene->GetPose(j);
		if (bindPose->IsBindPose())
		{
			for (unsigned int i = 0; i < bindPose->GetCount(); i++)
			{


				FbxNode* skeletonNode = bindPose->GetNode(i);
				FbxMesh* mesh = skeletonNode->GetMesh();
				if (mesh != NULL)

				{
					for (unsigned int j = 0; j < mesh->GetDeformerCount(); j++)
					{
						if (mesh->GetDeformer(j)->Is<FbxSkin>())
						{
							FbxSkin* skin = FbxCast<FbxSkin>(mesh->GetDeformer(j));

							control_point_influences.resize(mesh->GetControlPointsCount());

							GetClusters(skin);
							//					int w = 0;

						}

					}

				}

			}
		}
	}
	FbxAnimStack* stack = lScene->GetCurrentAnimationStack();
	FbxTime duration = stack->GetLocalTimeSpan().GetDuration();
	out_clip.duration = duration.Get();
	FbxTime temp = duration;
	int frames = duration.GetFrameCount(duration.eFrames24);
	for (size_t i = 0; i < frames; i++)
	{



		temp.SetFrame(i, duration.eFrames24);


		keyframe key;
		key.time = temp.Get();
		for (int i = 0; i < joints.size(); i++)
		{

			key.jointsMatrix.push_back(FBXAMatrix_To_XMMATRIX(joints[i].node->EvaluateGlobalTransform(key.time)));
			key.parents.push_back(joints[i].parent_index);
		}

		out_clip.frames.push_back(key);
	}

	if (skelefile.is_open() == false)
	{
		WriteOutSkeletonAnim(AnimFile);
	};



}
void GetClusters(FbxSkin* skin)
{

	for (unsigned int k = 0; k < skin->GetClusterCount(); k++)
	{
		FbxCluster* cluster = skin->GetCluster(k);
		FbxNode* link = cluster->GetLink();
		int jointIndex = -1;
		for (unsigned int jIndex = 0; jIndex < joints.size(); jIndex++)
		{
			if (link == joints[jIndex].node)
			{
				jointIndex = jIndex;
				break;

			}
		}
		assert(jointIndex != -1);
		int controlPointICount = cluster->GetControlPointIndicesCount();
		auto controlPointIndices = cluster->GetControlPointIndices();
		auto controlPointWeghts = cluster->GetControlPointWeights();

		for (unsigned int c = 0; c < controlPointICount; c++)
		{
			int index = controlPointIndices[c];
			influence_set* influenceSet = &control_point_influences[index];
			double weight = controlPointWeghts[c];
			influence temp = influence({ jointIndex, (float)weight });
			for (auto influence = influenceSet->begin(); influence != influenceSet->end(); influence++)
			{
				if (influence->weight < temp.weight)
				{
					swap(temp, *influence);
				}
			}


		}


	}
};
void  ProcessFbxMesh(FbxNode* Node, const char* meshfile, const char* matpath, const char* matfile)
{
	// set up output console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	//FBX Mesh stuff
	int childrenCount = Node->GetChildCount();

	//std::cout << "\nName:" << Node->GetName();
	SimpleVertex* vertices2;
	for (int i = 0; i < childrenCount; i++)
	{
		FbxNode* childNode = Node->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();

		if (mesh != NULL)
		{
			//std::cout << "\nMesh:" << childNode->GetName();

			// Get index count from mesh
			numIndices = mesh->GetPolygonVertexCount();
			//std::cout << "\nIndice Count:" << numIndices;

			// No need to allocate int array, FBX does for us
			indices = mesh->GetPolygonVertices();

			// Get vertex count from mesh
			numVertices = mesh->GetControlPointsCount();
			//std::cout << "\nVertex Count:" << numVertices;

			// Create SimpleVertex array to size of this mesh
			vertices = new SimpleVertex[numVertices];




#pragma region ================= Process Vertices ===================
			for (int j = 0; j < numVertices; j++)
			{

				FbxVector4 vert = mesh->GetControlPointAt(j);

				vertices[j].Position.x = (float)vert.mData[0] / scale;
				vertices[j].Position.y = (float)vert.mData[1] / scale;
				vertices[j].Position.z = (float)vert.mData[2] / scale;

				// Get the Normals array from the mesh

			}


			FbxArray<FbxVector4> normalsVec;
			mesh->GetPolygonVertexNormals(normalsVec);
			//std::cout << "\nNormalVec Count:" << normalsVec.Size();
			// Declare a new array for the second vertex array
	// Note the size is numIndices not numVertices
			vertices2 = new SimpleVertex[numIndices];

			FbxStringList lUVSetNameList;
			mesh->GetUVSetNames(lUVSetNameList);
			const char* lUVSetName = lUVSetNameList.GetStringAt(0);
			const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetName);
			const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;



			// align (expand) vertex array and set the normals and UVs
			for (int k = 0; k < numIndices; k++)
			{
				FbxVector2 lUVValue;

				vertices2[k] = vertices[indices[k]];
				vertices2[k].Normal.x = normalsVec[k].mData[0];
				vertices2[k].Normal.y = normalsVec[k].mData[1];
				vertices2[k].Normal.z = normalsVec[k].mData[2];

				int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(k) : k;
				lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

				vertices2[k].Tex.x = lUVValue[0];
				vertices2[k].Tex.y = lUVValue[1];
				vertices2[k].Tex.z = 0.0f;

			}
			// vertices is an "out" var so make sure it points to the new array
			// and clean up first array
			delete  vertices;
			vertices = vertices2;

			// make new indices to match the new vertex(2) array
			delete  indices;
			indices = new int[numIndices];
			for (int j = 0; j < numIndices; j++)
			{
				indices[j] = j;
			}
#pragma endregion

#pragma region =================== Texture ==================================

			//int materialCount = childNode->GetSrcObjectCount<FbxSurfaceMaterial>();
			int materialCount = childNode->GetMaterialCount();
			for (int index = 0; index < materialCount; index++)
			{
				material_t my_mat;
				FbxSurfaceMaterial* mat = childNode->GetMaterial(index);
				//Diffuse
				if (mat->Is<FbxSurfaceLambert>() == false)
					continue;

				FbxSurfaceLambert* lam = (FbxSurfaceLambert*)mat;
				FbxDouble3 diffuse_color = lam->Diffuse.Get();
				FbxDouble diffuse_factor = lam->DiffuseFactor.Get();

				my_mat[my_mat.DIFFUSE].value[0] = diffuse_color[0];
				my_mat[my_mat.DIFFUSE].value[1] = diffuse_color[1];
				my_mat[my_mat.DIFFUSE].value[2] = diffuse_color[2];

				my_mat[my_mat.DIFFUSE].factor = diffuse_factor;

				if (FbxFileTexture* file_texture = lam->Diffuse.GetSrcObject<FbxFileTexture>())
				{
					bool slash = false;
					string file_name = file_texture->GetRelativeFileName();
					file_path_t file_path;
					file_name.pop_back();
					file_name.pop_back();
					file_name.pop_back();
					file_name.append("dds");
					string path = matpath;
					string dds;
					for (int i = 0; i < file_name.size(); i++)
					{
						if (slash)
							dds.push_back(file_name[i]);
						if (file_name[i] == 92)
							slash = true;

					}

					file_name.clear();
					file_name.append(path);
					file_name.append(dds);
					strcpy(file_path.data(), file_name.c_str());

					my_mat[my_mat.DIFFUSE].input = paths.size();
					paths.push_back(file_path);


				}

#pragma region Emissive
				FbxDouble3 emissive_color = lam->Emissive.Get();
				FbxDouble emissive_factor = lam->EmissiveFactor.Get();

				my_mat[my_mat.EMISSIVE].value[0] = emissive_color[0];
				my_mat[my_mat.EMISSIVE].value[1] = emissive_color[1];
				my_mat[my_mat.EMISSIVE].value[2] = emissive_color[2];

				my_mat[my_mat.EMISSIVE].factor = emissive_factor;
				if (FbxFileTexture* file_texture = lam->Emissive.GetSrcObject<FbxFileTexture>())
				{
					bool slash = false;
					string file_name = file_texture->GetRelativeFileName();
					file_path_t file_path;
					file_name.pop_back();
					file_name.pop_back();
					file_name.pop_back();
					file_name.append("dds");
					string path = matpath;
					string dds;
					for (int i = 0; i < file_name.size(); i++)
					{
						if (slash)
							dds.push_back(file_name[i]);
						if (file_name[i] == 92)
							slash = true;

					}

					file_name.clear();
					file_name.append(path);
					file_name.append(dds);
					strcpy(file_path.data(), file_name.c_str());

					my_mat[my_mat.EMISSIVE].input = paths.size();
					paths.push_back(file_path);


				}
#pragma endregion

#pragma region Specular

				if (mat->Is<FbxSurfacePhong>())
				{
					FbxSurfacePhong* spec = (FbxSurfacePhong*)mat;
					FbxDouble3 specular_color = spec->Specular.Get();
					FbxDouble specular_factor = spec->SpecularFactor.Get();

					my_mat[my_mat.SPECULAR].value[0] = specular_color[0];
					my_mat[my_mat.SPECULAR].value[1] = specular_color[1];
					my_mat[my_mat.SPECULAR].value[2] = specular_color[2];

					my_mat[my_mat.SPECULAR].factor = specular_factor;
					if (FbxFileTexture* file_texture = spec->Specular.GetSrcObject<FbxFileTexture>())
					{
						bool slash = false;
						string file_name = file_texture->GetRelativeFileName();
						file_path_t file_path;
						file_name.pop_back();
						file_name.pop_back();
						file_name.pop_back();
						file_name.append("dds");
						string path = matpath;
						string dds;
						for (int i = 0; i < file_name.size(); i++)
						{
							if (slash)
								dds.push_back(file_name[i]);
							if (file_name[i] == 92)
								slash = true;

						}

						file_name.clear();
						file_name.append(path);
						file_name.append(dds);
						strcpy(file_path.data(), file_name.c_str());

						my_mat[my_mat.SPECULAR].input = paths.size();
						paths.push_back(file_path);






					}

				}
#pragma endregion


			}
			std::ofstream file(matfile, std::ios::trunc | std::ios::binary | std::ios::out);

			assert(file.is_open());

			uint32_t count = (uint32_t)paths.size();

			file.write((const char*)&count, sizeof(uint32_t));
			file.write((const char*)paths.data(), sizeof(file_path_t) * paths.size());

			file.close();
#pragma endregion




			if (true)
			{
				Compactify(vertices, meshfile);

			}
			else
			{
				// numVertices is an "out" var so set to new size
				// this is used in the DrawIndexed functions to set the 
				// the right number of triangles
				numVertices = numIndices;

			}
		}
		ProcessFbxMesh(childNode, meshfile, matpath);
	}



};
void ProcessFbxMeshAnim(FbxNode* Node, const char* meshfile, const char* matpath)
{
	// set up output console
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	//FBX Mesh stuff
	int childrenCount = Node->GetChildCount();

	//std::cout << "\nName:" << Node->GetName();
	SimpleVertexAnim* vertices2;
	for (int i = 0; i < childrenCount; i++)
	{
		FbxNode* childNode = Node->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();

		if (mesh != NULL)
		{
			//std::cout << "\nMesh:" << childNode->GetName();

			// Get index count from mesh
			numIndices = mesh->GetPolygonVertexCount();
			//std::cout << "\nIndice Count:" << numIndices;

			// No need to allocate int array, FBX does for us
			indices = mesh->GetPolygonVertices();

			// Get vertex count from mesh
			numVertices = mesh->GetControlPointsCount();
			//std::cout << "\nVertex Count:" << numVertices;

			// Create SimpleVertex array to size of this mesh
			verticesAnim = new SimpleVertexAnim[numVertices];




#pragma region ================= Process Vertices ===================
			for (int j = 0; j < numVertices; j++)
			{

				FbxVector4 vert = mesh->GetControlPointAt(j);

				verticesAnim[j].Pos.x = (float)vert.mData[0] / scale;
				verticesAnim[j].Pos.y = (float)vert.mData[1] / scale;
				verticesAnim[j].Pos.z = (float)vert.mData[2] / scale;
				influence_set set = control_point_influences[j];
				verticesAnim[j].Joints.x = set[0].joint;
				verticesAnim[j].Joints.y = set[1].joint;
				verticesAnim[j].Joints.z = set[2].joint;
				verticesAnim[j].Joints.w = set[3].joint;
				float sum = set[0].weight + set[1].weight + set[2].weight + set[3].weight;
				verticesAnim[j].Weights.x = set[0].weight / sum;
				verticesAnim[j].Weights.y = set[1].weight / sum;
				verticesAnim[j].Weights.z = set[2].weight / sum;
				verticesAnim[j].Weights.w = set[3].weight / sum;

				// Get the Normals array from the mesh

			}


			FbxArray<FbxVector4> normalsVec;
			mesh->GetPolygonVertexNormals(normalsVec);
			//std::cout << "\nNormalVec Count:" << normalsVec.Size();
			// Declare a new array for the second vertex array
	// Note the size is numIndices not numVertices
			vertices2 = new SimpleVertexAnim[numIndices];

			FbxStringList lUVSetNameList;
			mesh->GetUVSetNames(lUVSetNameList);
			const char* lUVSetName = lUVSetNameList.GetStringAt(0);
			const FbxGeometryElementUV* lUVElement = mesh->GetElementUV(lUVSetName);
			const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;



			// align (expand) vertex array and set the normals and UVs
			for (int k = 0; k < numIndices; k++)
			{
				FbxVector2 lUVValue;

				vertices2[k] = verticesAnim[indices[k]];
				vertices2[k].Normal.x = normalsVec[k].mData[0];
				vertices2[k].Normal.y = normalsVec[k].mData[1];
				vertices2[k].Normal.z = normalsVec[k].mData[2];

				int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(k) : k;
				lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

				vertices2[k].Tex.x = lUVValue[0];
				vertices2[k].Tex.y = lUVValue[1];
				vertices2[k].Tex.z = 0.0f;





			}
			// vertices is an "out" var so make sure it points to the new array
			// and clean up first array
			delete verticesAnim;
			verticesAnim = vertices2;

			// make new indices to match the new vertex(2) array
			delete indices;
			indices = new int[numIndices];
			for (int j = 0; j < numIndices; j++)
			{
				indices[j] = j;
			}
#pragma endregion

#pragma region =================== Texture ==================================

			//int materialCount = childNode->GetSrcObjectCount<FbxSurfaceMaterial>();
			int materialCount = childNode->GetMaterialCount();
			for (int index = 0; index < materialCount; index++)
			{
				material_t my_mat;
				FbxSurfaceMaterial* mat = childNode->GetMaterial(index);
				//Diffuse
				if (mat->Is<FbxSurfaceLambert>() == false)
					continue;

				FbxSurfaceLambert* lam = (FbxSurfaceLambert*)mat;
				FbxDouble3 diffuse_color = lam->Diffuse.Get();
				FbxDouble diffuse_factor = lam->DiffuseFactor.Get();

				my_mat[my_mat.DIFFUSE].value[0] = diffuse_color[0];
				my_mat[my_mat.DIFFUSE].value[1] = diffuse_color[1];
				my_mat[my_mat.DIFFUSE].value[2] = diffuse_color[2];

				my_mat[my_mat.DIFFUSE].factor = diffuse_factor;

				if (FbxFileTexture* file_texture = lam->Diffuse.GetSrcObject<FbxFileTexture>())
				{
					bool slash = false;
					string file_name = file_texture->GetRelativeFileName();
					file_path_t file_path;
					file_name.pop_back();
					file_name.pop_back();
					file_name.pop_back();
					file_name.append("dds");
					string path = "Assets/BattleMage.fbm/";
					string dds;
					for (int i = 0; i < file_name.size(); i++)
					{
						if (slash)
							dds.push_back(file_name[i]);
						if (file_name[i] == 92)
							slash = true;

					}

					file_name.clear();
					file_name.append(path);
					file_name.append(dds);
					strcpy(file_path.data(), file_name.c_str());

					my_mat[my_mat.DIFFUSE].input = paths.size();
					paths.push_back(file_path);


				}

#pragma region Emissive
				FbxDouble3 emissive_color = lam->Emissive.Get();
				FbxDouble emissive_factor = lam->EmissiveFactor.Get();

				my_mat[my_mat.EMISSIVE].value[0] = emissive_color[0];
				my_mat[my_mat.EMISSIVE].value[1] = emissive_color[1];
				my_mat[my_mat.EMISSIVE].value[2] = emissive_color[2];

				my_mat[my_mat.EMISSIVE].factor = emissive_factor;
				if (FbxFileTexture* file_texture = lam->Emissive.GetSrcObject<FbxFileTexture>())
				{
					bool slash = false;
					string file_name = file_texture->GetRelativeFileName();
					file_path_t file_path;
					file_name.pop_back();
					file_name.pop_back();
					file_name.pop_back();
					file_name.append("dds");
					string path = "Assets/BattleMage.fbm/";
					string dds;
					for (int i = 0; i < file_name.size(); i++)
					{
						if (slash)
							dds.push_back(file_name[i]);
						if (file_name[i] == 92)
							slash = true;

					}

					file_name.clear();
					file_name.append(path);
					file_name.append(dds);
					strcpy(file_path.data(), file_name.c_str());

					my_mat[my_mat.EMISSIVE].input = paths.size();
					paths.push_back(file_path);


				}
#pragma endregion

#pragma region Specular

				if (mat->Is<FbxSurfacePhong>())
				{
					FbxSurfacePhong* spec = (FbxSurfacePhong*)mat;
					FbxDouble3 specular_color = spec->Specular.Get();
					FbxDouble specular_factor = spec->SpecularFactor.Get();

					my_mat[my_mat.SPECULAR].value[0] = specular_color[0];
					my_mat[my_mat.SPECULAR].value[1] = specular_color[1];
					my_mat[my_mat.SPECULAR].value[2] = specular_color[2];

					my_mat[my_mat.SPECULAR].factor = specular_factor;
					if (FbxFileTexture* file_texture = spec->Specular.GetSrcObject<FbxFileTexture>())
					{
						bool slash = false;
						string file_name = file_texture->GetRelativeFileName();
						file_path_t file_path;
						file_name.pop_back();
						file_name.pop_back();
						file_name.pop_back();
						file_name.append("dds");
						string path = "Assets/BattleMage.fbm/";
						string dds;
						for (int i = 0; i < file_name.size(); i++)
						{
							if (slash)
								dds.push_back(file_name[i]);
							if (file_name[i] == 92)
								slash = true;

						}

						file_name.clear();
						file_name.append(path);
						file_name.append(dds);
						strcpy(file_path.data(), file_name.c_str());

						my_mat[my_mat.SPECULAR].input = paths.size();
						paths.push_back(file_path);






					}

				}
#pragma endregion


			}
			std::ofstream file("mage.materials", std::ios::trunc | std::ios::binary | std::ios::out);

			assert(file.is_open());

			uint32_t count = (uint32_t)paths.size();

			file.write((const char*)&count, sizeof(uint32_t));
			file.write((const char*)paths.data(), sizeof(file_path_t) * paths.size());

			file.close();
#pragma endregion




			if (true)
			{
				Compactify(vertices, meshfile);

			}
			else
			{
				// numVertices is an "out" var so set to new size
				// this is used in the DrawIndexed functions to set the 
				// the right number of triangles
				numVertices = numIndices;

			}
		}
		ProcessFbxMesh(childNode, meshfile, matpath);
	}



}

void Load_AnimFBX(const char* meshFileName, SimpleMeshAnim& mesh)
{

	std::fstream file{ meshFileName, std::ios_base::in | std::ios_base::binary };

	assert(file.is_open());

	uint32_t player_index_count;
	file.read((char*)&player_index_count, sizeof(uint32_t));

	mesh.indicesList.resize(player_index_count);

	file.read((char*)mesh.indicesList.data(), sizeof(uint32_t) * player_index_count);

	uint32_t player_vertex_count;
	file.read((char*)&player_vertex_count, sizeof(uint32_t));

	mesh.vertexList.resize(player_vertex_count);

	file.read((char*)mesh.vertexList.data(), sizeof(SimpleVertexAnim) * player_vertex_count);

	//Example mesh conditioning if needed - this flips handedness
	for (auto& v : mesh.vertexList)
	{
		v.Pos.x = -v.Pos.x;
		v.Normal.x = -v.Normal.x;
		v.Tex.y = 1.0f - v.Tex.y;

	}

	int tri_count = (int)(mesh.indicesList.size() / 3);

	for (int i = 0; i < tri_count; ++i)
	{
		int* tri = mesh.indicesList.data() + i * 3;

		int temp = tri[0];
		tri[0] = tri[2];
		tri[2] = temp;
	}
	file.close();
};

void  Load_FBX(const char* meshFileName, SimpleMesh& mesh)
{

	std::fstream file{ meshFileName, std::ios_base::in | std::ios_base::binary };

	assert(file.is_open());

	uint32_t player_index_count;
	file.read((char*)&player_index_count, sizeof(uint32_t));

	mesh.indicesList.resize(player_index_count);

	file.read((char*)mesh.indicesList.data(), sizeof(uint32_t) * player_index_count);

	uint32_t player_vertex_count;
	file.read((char*)&player_vertex_count, sizeof(uint32_t));

	mesh.vertexList.resize(player_vertex_count);

	file.read((char*)mesh.vertexList.data(), sizeof(SimpleVertex) * player_vertex_count);

	//Example mesh conditioning if needed - this flips handedness
	for (auto& v : mesh.vertexList)
	{
		v.Position.x = -v.Position.x;
		v.Normal.x = -v.Normal.x;
		v.Tex.y = 1.0f - v.Tex.y;

	}

	int tri_count = (int)(mesh.indicesList.size() / 3);

	for (int i = 0; i < tri_count; ++i)
	{
		int* tri = mesh.indicesList.data() + i * 3;

		int temp = tri[0];
		tri[0] = tri[2];
		tri[2] = temp;
	}
	file.close();
};
void  ComputeTangent(SimpleMesh& simpleMesh)
{
	//////////////////////Compute Normals///////////////////////////
	//If computeNormals was set to true then we will create our own
	//normals, if it was set to false we will use the obj files normals
	std::vector<XMFLOAT3> tempNormal;

	//normalized and unnormalized normals
	XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

	///////////////**************new**************////////////////////
	//tangent stuff
	std::vector<XMFLOAT3> tempTangent;
	XMFLOAT3 tangent = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float tcU1, tcV1, tcU2, tcV2;
	///////////////**************new**************////////////////////

	//Used to get vectors (sides) from the position of the verts
	float vecX, vecY, vecZ;

	//Two edges of our triangle
	XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	//Compute face normals
	//And Tangents
	for (int i = 0; i < simpleMesh.indicesList.size() / 3; ++i)
	{
		//Get the vector describing one edge of our triangle (edge 0,2)
		vecX = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3)]].Position.x - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.x;
		vecY = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3)]].Position.y - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.y;
		vecZ = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3)]].Position.z - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.z;
		edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge

		//Get the vector describing another edge of our triangle (edge 2,1)
		vecX = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.x - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 1]].Position.x;
		vecY = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.y - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 1]].Position.y;
		vecZ = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Position.z - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 1]].Position.z;
		edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge

		//Cross multiply the two edge vectors to get the un-normalized face normal
		XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));

		tempNormal.push_back(unnormalized);

		///////////////**************new**************////////////////////
		//Find first texture coordinate edge 2d vector
		tcU1 = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3)]].Tex.x - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Tex.x;
		tcV1 = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3)]].Tex.y - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Tex.y;

		//Find second texture coordinate edge 2d vector
		tcU2 = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Tex.x - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 1]].Tex.x;
		tcV2 = simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 2]].Tex.y - simpleMesh.vertexList[simpleMesh.indicesList[(i * 3) + 1]].Tex.y;

		//Find tangent using both tex coord edges and position edges
		tangent.x = (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.y = (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));
		tangent.z = (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * (1.0f / (tcU1 * tcV2 - tcU2 * tcV1));

		tempTangent.push_back(tangent);
		///////////////**************new**************////////////////////
	}

	//Compute vertex normals (normal Averaging)
	XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	int facesUsing = 0;
	float tX, tY, tZ;    //temp axis variables

	//Go through each vertex
	for (int i = 0; i < simpleMesh.vertexList.size(); ++i)
	{
		//Check which triangles use this vertex
		for (int j = 0; j < simpleMesh.indicesList.size() / 3; ++j)
		{
			if (simpleMesh.indicesList[j * 3] == i ||
				simpleMesh.indicesList[(j * 3) + 1] == i ||
				simpleMesh.indicesList[(j * 3) + 2] == i)
			{
				tX = XMVectorGetX(normalSum) + tempNormal[j].x;
				tY = XMVectorGetY(normalSum) + tempNormal[j].y;
				tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

				normalSum = XMVectorSet(tX, tY, tZ, 0.0f);    //If a face is using the vertex, add the unormalized face normal to the normalSum

				///////////////**************new**************////////////////////        
				//We can reuse tX, tY, tZ to sum up tangents
				tX = XMVectorGetX(tangentSum) + tempTangent[j].x;
				tY = XMVectorGetY(tangentSum) + tempTangent[j].y;
				tZ = XMVectorGetZ(tangentSum) + tempTangent[j].z;

				tangentSum = XMVectorSet(tX, tY, tZ, 0.0f); //sum up face tangents using this vertex
				///////////////**************new**************////////////////////

				facesUsing++;
			}
		}

		//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
		normalSum = normalSum / facesUsing;
		///////////////**************new**************////////////////////
		tangentSum = tangentSum / facesUsing;
		///////////////**************new**************////////////////////

		//Normalize the normalSum vector and tangent
		normalSum = XMVector3Normalize(normalSum);
		///////////////**************new**************////////////////////
		tangentSum = XMVector3Normalize(tangentSum);
		///////////////**************new**************////////////////////

		//Store the normal and tangent in our current vertex
		simpleMesh.vertexList[i].Normal.x = XMVectorGetX(normalSum);
		simpleMesh.vertexList[i].Normal.y = XMVectorGetY(normalSum);
		simpleMesh.vertexList[i].Normal.z = XMVectorGetZ(normalSum);

		///////////////**************new**************////////////////////
		simpleMesh.vertexList[i].Tangent.x = XMVectorGetX(tangentSum);
		simpleMesh.vertexList[i].Tangent.y = XMVectorGetY(tangentSum);
		simpleMesh.vertexList[i].Tangent.z = XMVectorGetZ(tangentSum);
		///////////////**************new**************////////////////////

		//Clear normalSum, tangentSum and facesUsing for next vertex
		normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		///////////////**************new**************////////////////////
		tangentSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		///////////////**************new**************////////////////////
		facesUsing = 0;

	}
};

void CompactifyAnim(SimpleVertexAnim* verticesCompact, const char* meshfile)
{

	float epsilon = 0.0001;
	bool Duplicate = false;
	int pos = 0;
	vector<SimpleVertexAnim> vertexList;
	vector<int> indicesList;



	for (int i = 0; i < numIndices; i++)
	{



		Duplicate = false;

		for (int j = 0; j < vertexList.size(); j++)
		{


			if ((abs(verticesCompact[i].Pos.x - vertexList[j].Pos.x) < epsilon) &&
				(abs(verticesCompact[i].Pos.y - vertexList[j].Pos.y) < epsilon) &&
				(abs(verticesCompact[i].Pos.z - vertexList[j].Pos.z) < epsilon) &&
				(abs(verticesCompact[i].Normal.x - vertexList[j].Normal.x) < epsilon) &&
				(abs(verticesCompact[i].Normal.y - vertexList[j].Normal.y) < epsilon) &&
				(abs(verticesCompact[i].Normal.z - vertexList[j].Normal.z) < epsilon) &&
				(abs(verticesCompact[i].Tex.x - vertexList[j].Tex.x) < epsilon) &&
				(abs(verticesCompact[i].Tex.y - vertexList[j].Tex.y) < epsilon) &&
				(abs(verticesCompact[i].Tex.z - vertexList[j].Tex.z) < epsilon) &&
				(abs(verticesCompact[i].Joints.x - vertexList[j].Joints.x) < epsilon) &&
				(abs(verticesCompact[i].Joints.y - vertexList[j].Joints.y) < epsilon) &&
				(abs(verticesCompact[i].Joints.z - vertexList[j].Joints.z) < epsilon) &&
				(abs(verticesCompact[i].Joints.w - vertexList[j].Joints.w) < epsilon) &&
				(abs(verticesCompact[i].Weights.x - vertexList[j].Weights.x) < epsilon) &&
				(abs(verticesCompact[i].Weights.y - vertexList[j].Weights.y) < epsilon) &&
				(abs(verticesCompact[i].Weights.z - vertexList[j].Weights.z) < epsilon) &&
				(abs(verticesCompact[i].Weights.w - vertexList[j].Weights.w) < epsilon))
			{
				Duplicate = true;
				indices[i] = j;
				break;
			}
		}



		if (Duplicate == false)
		{
			vertexList.push_back(verticesCompact[i]);
			indices[i] = vertexList.size() - 1;


		}



	}
	for (int i = 0; i < vertexList.size(); i++)
	{
		verticesCompact[i] = vertexList[i];



	}
	for (int i = 0; i < numIndices; i++)
	{
		indicesList.push_back(indices[i]);
	}


	// print out some stats
	//std::cout << "\nindex count BEFORE/AFTER compaction " << numIndices;
	//std::cout << "\nvertex count ORIGINAL (FBX source): " << numVertices;
	//std::cout << "\nvertex count AFTER expansion: " << numIndices;
	//std::cout << "\nvertex count AFTER compaction: " << vertexList.size();
	//std::cout << "\nSize reduction: " << ((numVertices - vertexList.size()) / (float)numVertices) * 100.00f << "%";
	//std::cout << "\nor " << (vertexList.size() / (float)numVertices) << " of the expanded size";

	//write out Mesh
	const char* path = meshfile;
	std::ofstream file(path, std::ios::trunc | std::ios::binary | std::ios::out);

	assert(file.is_open());

	uint32_t index_count = (uint32_t)indicesList.size();
	uint32_t vert_count = (uint32_t)vertexList.size();

	file.write((const char*)&index_count, sizeof(uint32_t));
	file.write((const char*)indicesList.data(), sizeof(uint32_t) * indicesList.size());
	file.write((const char*)&vert_count, sizeof(uint32_t));
	file.write((const char*)vertexList.data(), sizeof(SimpleVertexAnim) * vertexList.size());

	file.close();


}
void  Compactify(SimpleVertex* verticesCompact, const char* meshfile)
{

	float epsilon = 0.0001;
	bool Duplicate = false;
	int pos = 0;
	vector<SimpleVertex> vertexList;
	vector<int> indicesList;



	for (int i = 0; i < numIndices; i++)
	{



		Duplicate = false;

		for (int j = 0; j < vertexList.size(); j++)
		{


			if ((abs(verticesCompact[i].Position.x - vertexList[j].Position.x) < epsilon) &&
				(abs(verticesCompact[i].Position.y - vertexList[j].Position.y) < epsilon) &&
				(abs(verticesCompact[i].Position.z - vertexList[j].Position.z) < epsilon) &&
				(abs(verticesCompact[i].Normal.x - vertexList[j].Normal.x) < epsilon) &&
				(abs(verticesCompact[i].Normal.y - vertexList[j].Normal.y) < epsilon) &&
				(abs(verticesCompact[i].Normal.z - vertexList[j].Normal.z) < epsilon) &&
				(abs(verticesCompact[i].Tex.x - vertexList[j].Tex.x) < epsilon) &&
				(abs(verticesCompact[i].Tex.y - vertexList[j].Tex.y) < epsilon) &&
				(abs(verticesCompact[i].Tex.z - vertexList[j].Tex.z) < epsilon)) {
				Duplicate = true;
				indices[i] = j;
				break;
			}
		}



		if (Duplicate == false)
		{
			vertexList.push_back(verticesCompact[i]);
			indices[i] = vertexList.size() - 1;


		}



	}
	for (int i = 0; i < vertexList.size(); i++)
	{
		verticesCompact[i] = vertexList[i];



	}
	for (int i = 0; i < numIndices; i++)
	{
		indicesList.push_back(indices[i]);
	}

	SimpleMesh GenerateTangents;
	GenerateTangents.vertexList = vertexList;
	GenerateTangents.indicesList = indicesList;
	ComputeTangent(GenerateTangents);
	vertexList = GenerateTangents.vertexList;
	indicesList = GenerateTangents.indicesList;

	// print out some stats
	//std::cout << "\nindex count BEFORE/AFTER compaction " << numIndices;
	//std::cout << "\nvertex count ORIGINAL (FBX source): " << numVertices;
	//std::cout << "\nvertex count AFTER expansion: " << numIndices;
	//std::cout << "\nvertex count AFTER compaction: " << vertexList.size();
	//std::cout << "\nSize reduction: " << ((numVertices - vertexList.size()) / (float)numVertices) * 100.00f << "%";
	//std::cout << "\nor " << (vertexList.size() / (float)numVertices) << " of the expanded size";

	//write out Mesh
	const char* path = meshfile;
	std::ofstream file(path, std::ios::trunc | std::ios::binary | std::ios::out);

	assert(file.is_open());


	uint32_t index_count = (uint32_t)indicesList.size();
	uint32_t vert_count = (uint32_t)vertexList.size();

	file.write((const char*)&index_count, sizeof(uint32_t));
	file.write((const char*)indicesList.data(), sizeof(uint32_t) * indicesList.size());
	file.write((const char*)&vert_count, sizeof(uint32_t));
	file.write((const char*)vertexList.data(), sizeof(SimpleVertex) * vertexList.size());

	file.close();


};

XMMATRIX FBXAMatrix_To_XMMATRIX(FbxAMatrix& m)
{
	XMMATRIX out;
	for (int i = 0; i < 4; i++)
	{
		out.r[i].m128_f32[0] = m.Get(i, 0);
		out.r[i].m128_f32[1] = m.Get(i, 1);
		out.r[i].m128_f32[2] = m.Get(i, 2);
		out.r[i].m128_f32[3] = m.Get(i, 3);



	}
	return out;

}

void ProcessFbxSkeleton(FbxNode* Node, int parentindex)
{
	int childCount = Node->GetChildCount();
	XMMATRIX m = FBXAMatrix_To_XMMATRIX((Node->EvaluateGlobalTransform()));
	fbx_joint joint = { Node, m, parentindex,  childCount };
	joints.push_back(joint);


	int jointCount = joints.size();
	for (int i = 0; i < childCount; i++)
	{
		FbxNode* childNode = Node->GetChild(i);
		FbxSkeleton* skeleton = childNode->GetSkeleton();
		if (skeleton != nullptr)
		{

			ProcessFbxSkeleton(childNode, jointCount - 1);

		}
	}


}

void WriteOutSkeletonAnim(const char* AnimFile)
{
	const char* path = AnimFile;
	std::ofstream file(path, std::ios::trunc | std::ios::binary | std::ios::out);

	assert(file.is_open());

	uint32_t joint_count = (uint32_t)joints.size();

	file.write((const char*)&joint_count, sizeof(uint32_t));
	file.write((const char*)joints.data(), sizeof(fbx_joint) * joints.size());

	file.close();



}
void Load_AnimSkeletonFBX(const char* animFileName, vector<fbx_joint>& skeleton)
{

	std::fstream file{ animFileName, std::ios_base::in | std::ios_base::binary };

	assert(file.is_open());

	uint32_t joint_Count;
	file.read((char*)&joint_Count, sizeof(uint32_t));
	skeleton.resize(joint_Count);
	file.read((char*)skeleton.data(), sizeof(fbx_joint) * joint_Count);









	file.close();
};


#pragma endregion

