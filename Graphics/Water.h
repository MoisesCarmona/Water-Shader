#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "WaterVertexShader.csh"
#include "WaterPixelShader.csh"
#include "RefractionVertexShader.csh"
#include "RefractionPixelShader.csh"
#include "DDSTextureLoader.h"



using namespace DirectX;
template<typename T>
void SafeRelease(T item)
{
	if (item)

		item->Release();

}


class Water
{
public:

	ID3D11Texture2D* NormalTexture;
	ID3D11ShaderResourceView* TextureView_Normal;

	//RenderToTexture
	ID3D11RenderTargetView* RTTRenderTV_Reflection;
	ID3D11RenderTargetView* RTTRenderTV_Refraction;

	D3D11_VIEWPORT RTT_viewport;
	ID3D11DepthStencilView* zBufferRTT;

	ID3D11Texture2D* RTT_Texture_Reflection;
	ID3D11Texture2D* RTT_Texture_Refraction;

	ID3D11ShaderResourceView* TextureView_RTT_Reflection;
	ID3D11ShaderResourceView* TextureView_RTT_Refraction;

	//Shaders
	ID3D11VertexShader* VertexShader_Water;
	ID3D11PixelShader* PixelShader_Water;

	ID3D11VertexShader* VertexShader_Refraction;
	ID3D11PixelShader* PixelShader_Refraction;



	//Buffers
	ID3D11Buffer* clipBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* reflectionBuffer;
	ID3D11Buffer* waterBuffer;
	ID3D11Buffer* WVPCamBuffer;

	//Layouts
	ID3D11InputLayout* VertexLayout;
	ID3D11InputLayout* VertexLayout_Refract;

	ID3D11SamplerState* wrapState;




	struct ReflectionBufferType
	{
		XMMATRIX reflection;

	}ReflectionBufferData;
	struct WaterBufferType
	{
		float waterTranslation = 0;
		float reflectRefractScale = 0.0001f;
		XMVECTOR padding;

	}waterBufferData;

	struct WVP
	{

		XMMATRIX worldMatrix = {};
		XMMATRIX viewMatrix = {};
		XMMATRIX projMatrix = {};
		XMVECTOR cam = {};

	}WaterWVPCam;

	//Set Everything to nullptr
	Water()
	{

		NormalTexture = nullptr;
		TextureView_Normal = nullptr;

		RTTRenderTV_Reflection = nullptr;
		RTTRenderTV_Refraction = nullptr;

		zBufferRTT = nullptr;
		RTT_Texture_Reflection = nullptr;
		RTT_Texture_Refraction = nullptr;
		TextureView_RTT_Reflection = nullptr;
		TextureView_RTT_Refraction = nullptr;


		VertexShader_Water = nullptr;
		PixelShader_Water = nullptr;
		VertexShader_Refraction = nullptr;
		PixelShader_Refraction = nullptr;


		clipBuffer = nullptr;
		lightBuffer = nullptr;
		reflectionBuffer = nullptr;
		waterBuffer = nullptr;
		WVPCamBuffer = nullptr;


		VertexLayout = nullptr;
		VertexLayout_Refract = nullptr;

		wrapState = nullptr;
	}
	//~Water()
	//{
	//	Release();

	//}

	struct LightBufferType
	{

		XMVECTOR ambientColor = { 1,1,1 ,1 };
		XMVECTOR diffuseColor = { 1,1,1 , 1 };
		XMVECTOR lightDirection = { 0,-1,0 };


	}lightBufferData;
	//Initializes and Creates Buffers and SamplerStatte for Water
	void InitializeBuffersandSamplerState(ID3D11Device* g_Device)
	{
		D3D11_BUFFER_DESC bDesc;
		D3D11_SUBRESOURCE_DATA subData;
		ZeroMemory(&bDesc, sizeof(bDesc));
		ZeroMemory(&subData, sizeof(subData));

		bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bDesc.ByteWidth = sizeof(XMVECTOR);
		bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;
		bDesc.Usage = D3D11_USAGE_DYNAMIC;

		subData.pSysMem = 0;

		g_Device->CreateBuffer(&bDesc, nullptr, &clipBuffer);

		bDesc.ByteWidth = sizeof(LightBufferType);

		HRESULT hr;
		 g_Device->CreateBuffer(&bDesc, nullptr, &lightBuffer);

		bDesc.ByteWidth = sizeof(ReflectionBufferType);

		 g_Device->CreateBuffer(&bDesc, nullptr, &reflectionBuffer);
		bDesc.ByteWidth = sizeof(WaterBufferType);
		hr = g_Device->CreateBuffer(&bDesc, nullptr, &waterBuffer);

		bDesc.ByteWidth = sizeof(WVP);
		g_Device->CreateBuffer(&bDesc, nullptr, &WVPCamBuffer);

		D3D11_SAMPLER_DESC sWDesc =
		{
			D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
			D3D11_TEXTURE_ADDRESS_WRAP,
				0.0f,
				1,
				D3D11_COMPARISON_NEVER,
			(1.0f,1.0f,1.0f,1.0f),
			(-FLT_MAX),
			(FLT_MAX),




		};


		g_Device->CreateSamplerState(&sWDesc, &wrapState);






	}

	//Initialize and Create Water Shaders and Layout, also Refraction's if needed
	void InitializeShadersAndLayout(ID3D11Device* g_Device, bool Refraction = false)
	{

		HRESULT hr = g_Device->CreateVertexShader(WaterVertexShader, sizeof(WaterVertexShader), nullptr, &VertexShader_Water);
		hr = g_Device->CreatePixelShader(WaterPixelShader, sizeof(WaterPixelShader), nullptr, &PixelShader_Water);
		D3D11_INPUT_ELEMENT_DESC waterInputEDesc[]
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

		};
		g_Device->CreateInputLayout(waterInputEDesc, 2, WaterVertexShader, sizeof(WaterVertexShader), &VertexLayout);

		if (Refraction == true)
		{
			D3D11_INPUT_ELEMENT_DESC RefractInputEDesc[]
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};
			g_Device->CreateInputLayout(RefractInputEDesc, 3, RefractionVertexShader, sizeof(RefractionVertexShader), &VertexLayout_Refract);

			HRESULT hr = g_Device->CreateVertexShader(RefractionVertexShader, sizeof(RefractionVertexShader), nullptr, &VertexShader_Refraction);
			hr = g_Device->CreatePixelShader(RefractionPixelShader, sizeof(RefractionPixelShader), nullptr, &PixelShader_Refraction);
		}
	}

	//RenderTo Texture Setup for Reflection, also Refraction if needed. Call GenerateMips(TextureView_RTT_(Reflection or Refraction)) after Draw Call
	void RenderToTexture(ID3D11Device* g_Device, int textureWidth, int textureHeight, bool Refraction = false)
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
		g_Device->CreateTexture2D(&tDesc, nullptr, &RTT_Texture_Reflection);
		if (Refraction == true)
		{
			g_Device->CreateTexture2D(&tDesc, nullptr, &RTT_Texture_Refraction);
		}



		D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
		RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		RTVDesc.Texture2D.MipSlice = 0;

		g_Device->CreateRenderTargetView(RTT_Texture_Reflection, &RTVDesc, &RTTRenderTV_Reflection);
		if (Refraction == true)
		{
			g_Device->CreateRenderTargetView(RTT_Texture_Refraction, &RTVDesc, &RTTRenderTV_Refraction);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		g_Device->CreateShaderResourceView(RTT_Texture_Reflection, &SRVDesc, &TextureView_RTT_Reflection);
		if (Refraction == true)
		{
			g_Device->CreateShaderResourceView(RTT_Texture_Refraction, &SRVDesc, &TextureView_RTT_Refraction);
		}

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

	//Pass In Camera View Matrix. Set the Object Being Reflected's View Matrix  and ReflectionBufferType's reflection to the Output
	XMMATRIX CreateReflectionViewMatrix(XMMATRIX& Camera)
	{
		XMMATRIX m_reflectionViewMatrix = Camera;
		m_reflectionViewMatrix = (XMMatrixInverse(nullptr, m_reflectionViewMatrix));

		XMMATRIX reflectionMatrix_Y =
		{
			{1,0,0,0 },
			{0,-1,0,0 },
			{0,0,1,0 },
			{0,0,0,1 },
		};
		m_reflectionViewMatrix = XMMatrixMultiply(m_reflectionViewMatrix, reflectionMatrix_Y);

		m_reflectionViewMatrix = (XMMatrixInverse(nullptr, m_reflectionViewMatrix));


		return m_reflectionViewMatrix;



	}

	void SetNormalTexture(ID3D11Device* g_Device, const wchar_t* normalTextureFile)
	{
		CreateDDSTextureFromFile(g_Device, normalTextureFile, (ID3D11Resource**)&NormalTexture, &TextureView_Normal);

	}
	//Draw Water
	void DrawWater(ID3D11DeviceContext* g_DeviceContext, ID3D11Buffer* VBuffer, ID3D11Buffer* IBuffer, SIZE_T strideSize, int IndexCount, bool Refraction = false)
	{
		ID3D11Buffer* meshVBW[] = { VBuffer };
		UINT mesh_stridesW[] = { strideSize };
		UINT mesh_offsetsW[] = { 0 };

		g_DeviceContext->IASetVertexBuffers(0, 1, meshVBW, mesh_stridesW, mesh_offsetsW);
		g_DeviceContext->IASetIndexBuffer(IBuffer, DXGI_FORMAT_R32_UINT, 0);
		g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		g_DeviceContext->VSSetShader(VertexShader_Water, 0, 0);

		g_DeviceContext->IASetInputLayout(VertexLayout);

		if (Refraction == false)
		{
			ID3D11ShaderResourceView* texViewsW[]{ TextureView_Normal,TextureView_RTT_Reflection, nullptr };

			g_DeviceContext->PSSetShader(PixelShader_Water, 0, 0);
			g_DeviceContext->PSSetShaderResources(0, 3, texViewsW);


		}
		else
		{


			ID3D11ShaderResourceView* texViewsW[]{ TextureView_Normal,TextureView_RTT_Reflection, TextureView_RTT_Refraction };

			g_DeviceContext->PSSetShader(PixelShader_Water, 0, 0);
			g_DeviceContext->PSSetShaderResources(0, 3, texViewsW);
		}
		g_DeviceContext->PSSetSamplers(0, 1, &wrapState);


		WaterWVPCam.worldMatrix = XMMatrixIdentity();

		WaterWVPCam.worldMatrix = XMMatrixMultiply(WaterWVPCam.worldMatrix, XMMatrixScaling(300000, 300000, 300000));



		waterBufferData.reflectRefractScale = 0.01f;

		// Send it to Video Card
		D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		ID3D11Buffer* constantsW[] = { WVPCamBuffer, reflectionBuffer };

		g_DeviceContext->Map(WVPCamBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((WVP*)(gpuBuffer.pData)) = WaterWVPCam;
		g_DeviceContext->Unmap(WVPCamBuffer, 0);

		g_DeviceContext->Map(reflectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((ReflectionBufferType*)(gpuBuffer.pData)) = ReflectionBufferData;
		g_DeviceContext->Unmap(reflectionBuffer, 0);
		g_DeviceContext->VSSetConstantBuffers(0, 2, constantsW);

		g_DeviceContext->Map(waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		*((WaterBufferType*)(gpuBuffer.pData)) = waterBufferData;
		g_DeviceContext->Unmap(waterBuffer, 0);
		g_DeviceContext->PSSetConstantBuffers(0, 1, &waterBuffer);





		//Draw
		g_DeviceContext->DrawIndexed(IndexCount, 0, 0);
		ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr };
		g_DeviceContext->PSSetShaderResources(0, 3, null);


	}
	
	//Clean Up Water Memory
	void Release()
	{
		//Release Textures
		SafeRelease(NormalTexture);
		SafeRelease(TextureView_Normal);

		//Release RenderToTexture Vars
		SafeRelease(TextureView_RTT_Reflection);
		SafeRelease(TextureView_RTT_Refraction);

		SafeRelease(RTT_Texture_Reflection);
		SafeRelease(RTT_Texture_Refraction);

		SafeRelease(RTTRenderTV_Reflection);
		SafeRelease(RTTRenderTV_Refraction);
		SafeRelease(zBufferRTT);

		//Release Buffers
		SafeRelease(clipBuffer);
		SafeRelease(lightBuffer);
		SafeRelease(reflectionBuffer);
		SafeRelease(waterBuffer);
		SafeRelease(WVPCamBuffer);

		//Release Layouts
		SafeRelease(VertexLayout);
		SafeRelease(VertexLayout_Refract);


		//Release Shaders
		SafeRelease(VertexShader_Water);
		SafeRelease(VertexShader_Refraction);
		SafeRelease(PixelShader_Water);
		SafeRelease(PixelShader_Refraction);

		//Release Sampler
		SafeRelease(wrapState);


	}

};

