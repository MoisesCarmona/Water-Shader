#pragma once

#include "DirectXModelLoader.h"
#include "MeshPixelShader.csh"
#include "MeshVertexShader.csh"  
#include "AnimPixelShader.csh"
#include "AnimVertexShader.csh"  
#include "LineVertexShader.csh"
#include "LinePixelShader.csh"
#include "SkyboxVertexShader.csh"
#include "SkyboxPixelShader.csh"

#include "Logger.h"



class Renderer
{
	//Globals
//Initialization and Utility
	ID3D11Device* g_Device;
	IDXGISwapChain* g_Swapchain;
	ID3D11DeviceContext* g_DeviceContext;
	ID3D11DepthStencilView* g_zBuffer = nullptr;
	ID3D11DepthStencilState* g_zState;
	ID3D11RasterizerState* g_rasterStateCullNone;

	ID3D11Buffer* g_reflectionBuffer = nullptr;
	ID3D11Buffer* g_waterBuffer = nullptr;

	float g_aspectRatio;
	float g_width;
	float g_height;

	//Drawing
	ID3D11RenderTargetView* g_RenderTargetView;
	D3D11_VIEWPORT g_viewport;

	Object IslandModel;
	Object IslandTexture;

	ObjectAnim MageModel;
	Object Skybox;
	Object SphereReflect;
	Object Water_Old;
	Skeleton Anim;
	ID3D11SamplerState* wrapState = nullptr;
	ID3D11Buffer* clipBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;

	XMMATRIX defaultView = XMMatrixLookToLH({ 0,1000,1000 }, { 0, 0, 1 }, { 0,1,0 });
	Water WATA;

	HWND handle;

	~Renderer();
public:

	Renderer(HWND hWnd);


	void Draw();
	Water InitializeObjects();
	void Release();


};
//Initialization
Renderer::Renderer(HWND hWnd)
{

#pragma region DirextX11Init
	handle = hWnd;
	RECT rect;
	GetClientRect(hWnd, &rect);
	g_width = rect.right - rect.left;
	g_height = rect.bottom - rect.top;


	//Attach DirectX to WIndow
	D3D_FEATURE_LEVEL DX11 = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = 1;
	swapDesc.OutputWindow = hWnd;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = g_width;
	swapDesc.BufferDesc.Height = g_height;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.SampleDesc.Count = 1;

	g_aspectRatio = swapDesc.BufferDesc.Width / (float)swapDesc.BufferDesc.Height;

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &DX11, 1, D3D11_SDK_VERSION, &swapDesc, &g_Swapchain, &g_Device, 0, &g_DeviceContext);
	assert(!FAILED(hr));

	ID3D11Resource* backBuffer;
	hr = g_Swapchain->GetBuffer(0, __uuidof(backBuffer), (void**)&backBuffer);
	hr = g_Device->CreateRenderTargetView(backBuffer, NULL, &g_RenderTargetView);
	assert(!FAILED(hr));

	//Setup viewport 
	g_viewport.Width = swapDesc.BufferDesc.Width;
	g_viewport.Height = swapDesc.BufferDesc.Height;
	g_viewport.TopLeftY = g_viewport.TopLeftX = 0;
	g_viewport.MinDepth = 0;
	g_viewport.MaxDepth = 1;

	//Release the resource to decrement the counter by one
	//This is nescessary to keep the buffer from leaking memory
	backBuffer->Release();

	/////////////////////////////////
	// Create Depth Buffer Texture //
	/////////////////////////////////
	D3D11_TEXTURE2D_DESC depthTextureDesc = { 0 };
	depthTextureDesc.Width = g_width;
	depthTextureDesc.Height = g_height;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.SampleDesc.Count = 1;

	depthTextureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer;
	g_Device->CreateTexture2D(&depthTextureDesc, NULL, &depthBuffer);
	///////////////////////////////
	// Create Depth Stencil View //
	///////////////////////////////

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;

	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = g_Device->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &g_zBuffer);
	depthBuffer->Release();

	/////////////////////////
	// Initialize Viewport //
	/////////////////////////

	D3D11_VIEWPORT viewport;
	viewport.Width = g_width;
	viewport.Height = g_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = rect.left;
	viewport.TopLeftY = rect.top;

	//Rasterizer
	g_DeviceContext->RSSetViewports(1, &viewport);


#pragma endregion
//Extra
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilStateDesc.DepthEnable = true;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	g_Device->CreateDepthStencilState(&depthStencilStateDesc, &g_zState);

	D3D11_RASTERIZER_DESC cullModeDesc;
	ZeroMemory(&cullModeDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullModeDesc.FillMode = D3D11_FILL_SOLID;
	cullModeDesc.CullMode = D3D11_CULL_NONE;
	cullModeDesc.FrontCounterClockwise = false;
	g_Device->CreateRasterizerState(&cullModeDesc, &g_rasterStateCullNone);




	WATA = InitializeObjects();

}
Renderer::~Renderer()
{
	WATA.Release();
	SphereReflect.Release();
	IslandTexture.Release();
	IslandModel.Release();
	MageModel.Release();
	Skybox.Release();
	Water_Old.Release();
	Anim.Release();
	SafeRelease(g_Device);
	SafeRelease(g_DeviceContext);
	SafeRelease(g_RenderTargetView);
	SafeRelease(g_Swapchain);
	SafeRelease(g_zBuffer);
	SafeRelease(g_zState);
	SafeRelease(g_rasterStateCullNone);
	SafeRelease(wrapState);
	SafeRelease(clipBuffer);
	SafeRelease(lightBuffer);
	SafeRelease(g_reflectionBuffer);
	SafeRelease(g_waterBuffer);

}

void Renderer::Draw()
{
	XMVECTOR clipPlane = { 0.0f, -1.0f, 1.0f };

	
	Logger::log("M");
	


	
	
WATA.waterBufferData.waterTranslation=  WBT.waterTranslation = timeX.TotalTime() ;
	
	
	
	D3D11_MAPPED_SUBRESOURCE gpuBuffer;

	//Rendering
	g_DeviceContext->ClearDepthStencilView(g_zBuffer, D3D11_CLEAR_DEPTH, 1, 0); // clear it to Z exponential Far.

	ID3D11RenderTargetView* tempRTV[] = { g_RenderTargetView };
	g_DeviceContext->OMSetRenderTargets(1, tempRTV, g_zBuffer);
	const float color[] = { 0,0,0,1 };
	g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, color);
	g_DeviceContext->RSSetViewports(1, &g_viewport);
	RECT rect;
	GetClientRect(handle, &rect);
	g_width = rect.right - rect.left;
	g_height = rect.bottom - rect.top;
	g_aspectRatio = g_width / g_height;


	WATA.WaterWVPCam.projMatrix =IslandTexture.wvp.projMatrix = Water_Old.wvp.projMatrix = SphereReflect.wvp.projMatrix = Anim.wvp.projMatrix = IslandModel.wvp.projMatrix = Skybox.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);
	WATA.WaterWVPCam.viewMatrix = IslandTexture.wvp.viewMatrix =	 Water_Old.wvp.viewMatrix =  IslandModel.wvp.viewMatrix = Anim.wvp.viewMatrix =  Skybox.wvp.viewMatrix;
	 
#pragma region Skybox
	
	XMMATRIX InvView = XMMatrixInverse(nullptr, IslandModel.wvp.viewMatrix);
	XMVECTOR YReflect = { 0,-1,0 };
	
	WATA.ReflectionBufferData.reflection = RBT.reflection  = SphereReflect.wvp.viewMatrix = RenderReflection(IslandModel.wvp.viewMatrix, 2.75);;
	SphereReflect.wvp.cam = { InvView.r[3].m128_f32[0],-InvView.r[3].m128_f32[1],InvView.r[3].m128_f32[2], 1 };

	WATA.WaterWVPCam.cam = Water_Old.wvp.cam = Skybox.wvp.cam = { InvView.r[3].m128_f32[0],InvView.r[3].m128_f32[1],InvView.r[3].m128_f32[2], 1 };

	ID3D11Buffer* meshVBSky[] = { Skybox.VBuffer };
	UINT mesh_stridesSky[] = { sizeof(SimpleVertex) };
	UINT mesh_offsetsSky[] = { 0 };

	g_DeviceContext->IASetVertexBuffers(0, 1, meshVBSky, mesh_stridesSky, mesh_offsetsSky);
	g_DeviceContext->IASetIndexBuffer(Skybox.IBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_DeviceContext->OMSetDepthStencilState(g_zState, 0);
	g_DeviceContext->RSSetState(g_rasterStateCullNone);
	g_DeviceContext->VSSetShader(Skybox.VShader, 0, 0);

	g_DeviceContext->IASetInputLayout(Skybox.VLayout);

	ID3D11ShaderResourceView* texViewsSky[]{ Skybox.TextureView_SkyBox };


	g_DeviceContext->PSSetShader(Skybox.PShader, 0, 0);

	g_DeviceContext->PSSetShaderResources(0, 1, texViewsSky);
	g_DeviceContext->PSSetSamplers(0, 1, &wrapState);


	//Modify world Matrix before drawing
	//My_Matrices.time = timeX.TotalTimeExact();
	XMMATRIX zeroSky
	{ 0,0,0,0,
	  0,0,0,0,
	  0,0,0,0,
	  0,0,0,0 };



	Skybox.wvp.worldMatrix = XMMatrixMultiply(Skybox.wvp.worldMatrix, zeroSky);
	Skybox.wvp.worldMatrix = XMMatrixIdentity();

	XMMATRIX T = XMMatrixTranslation(Skybox.wvp.cam.m128_f32[0], Skybox.wvp.cam.m128_f32[1], Skybox.wvp.cam.m128_f32[2]);

	T = XMMatrixMultiply(XMMatrixScaling(1000.0f, 1000.0f, 1000.0f), T);
	Skybox.wvp.worldMatrix = XMMatrixMultiply(Skybox.wvp.worldMatrix, T);

	

	// Send it to Video Card


	g_DeviceContext->Map(Skybox.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((WVP*)(gpuBuffer.pData)) = Skybox.wvp;
	g_DeviceContext->Unmap(Skybox.constantBuffer, 0);
	ID3D11Buffer* constantsSky[] = { Skybox.constantBuffer };

	g_DeviceContext->VSSetConstantBuffers(0, 1, constantsSky);


	//Draw
	g_DeviceContext->DrawIndexed(Skybox.IndexCount, 0, 0);
#pragma endregion


#pragma region RendertoTextureIslandRefraction

	g_DeviceContext->ClearDepthStencilView(WATA.zBufferRTT, D3D11_CLEAR_DEPTH, 1, 0); // clear it to Z exponential Far.
	
	ID3D11RenderTargetView* const RTTtarget[] = {WATA.RTTRenderTV_Refraction };
	g_DeviceContext->OMSetRenderTargets(1, RTTtarget, WATA.zBufferRTT);
	g_DeviceContext->ClearRenderTargetView(RTTtarget[0], color);
	g_DeviceContext->RSSetViewports(1, &WATA.RTT_viewport);



	ID3D11Buffer* meshVBRTT[] = { IslandTexture.VBuffer };
	UINT mesh_stridesRTT[] = { sizeof(SimpleVertex) };
	UINT mesh_offsetsRTT[] = { 0 };
					 
	g_DeviceContext->IASetVertexBuffers(0, 1, meshVBRTT, mesh_stridesRTT, mesh_offsetsRTT);
	g_DeviceContext->IASetIndexBuffer(IslandTexture.IBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_DeviceContext->VSSetShader(WATA.VertexShader_Refraction, 0, 0);

	g_DeviceContext->IASetInputLayout(WATA.VertexLayout_Refract);

	

	ID3D11ShaderResourceView* texViews[]{ IslandTexture.TextureView_Diffuse };



	g_DeviceContext->PSSetShader(WATA.PixelShader_Refraction, 0, 0);
	g_DeviceContext->PSSetShaderResources(0, 1, texViews);
	g_DeviceContext->PSSetSamplers(0, 1, &WATA.wrapState);


	//Modify world Matrix before drawing
	//My_Matrices.time = timeX.TotalTimeExact();
	XMMATRIX zero
	{ 0,0,0,0,
	  0,0,0,0,
	  0,0,0,0,
	  0,0,0,0 };

	IslandTexture.wvp.worldMatrix = XMMatrixMultiply(IslandTexture.wvp.worldMatrix, zero);
	IslandTexture.wvp.worldMatrix = XMMatrixIdentity();
	IslandTexture.wvp.worldMatrix = XMMatrixTranslation(0, -100, 0);
	//IslandModel.wvp.worldMatrix = XMMatrixMultiply(IslandModel.wvp.worldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));



	// Send it to Video Card
	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
	ID3D11Buffer* constantsRTT[] = { IslandTexture.constantBuffer, clipBuffer  };

	g_DeviceContext->Map(IslandTexture.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((WVP*)(gpuBuffer.pData)) = IslandTexture.wvp;
	g_DeviceContext->Unmap(IslandTexture.constantBuffer, 0);

	g_DeviceContext->Map(WATA.clipBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((XMVECTOR*)(gpuBuffer.pData)) = clipPlane;
	g_DeviceContext->Unmap(WATA.clipBuffer, 0);

	g_DeviceContext->Map(WATA.lightBuffer , 0, D3D11_MAP_WRITE_DISCARD, 0, & gpuBuffer);
	*((LBF*)(gpuBuffer.pData)) = lightColorDir;
	g_DeviceContext->Unmap(WATA.lightBuffer, 0);





	g_DeviceContext->VSSetConstantBuffers(0, 2, constantsRTT);

	g_DeviceContext->PSSetConstantBuffers(0, 1, &WATA.lightBuffer);




	//Draw
	g_DeviceContext->DrawIndexed(IslandTexture.IndexCount, 0, 0);


	g_DeviceContext->GenerateMips(WATA.TextureView_RTT_Refraction);



#pragma endregion
#pragma region SphereReflection

	g_DeviceContext->ClearDepthStencilView(WATA.zBufferRTT, D3D11_CLEAR_DEPTH, 1, 0); // clear it to Z exponential Far.

	ID3D11RenderTargetView* const RTTtarget2[] = { WATA.RTTRenderTV_Reflection};
	g_DeviceContext->OMSetRenderTargets(1, RTTtarget2, WATA.zBufferRTT);
	g_DeviceContext->RSSetViewports(1, &WATA.RTT_viewport);

	g_DeviceContext->ClearRenderTargetView(RTTtarget2[0], color);


	ID3D11Buffer* meshVBRTT2[] = { SphereReflect.VBuffer };
	UINT mesh_stridesRTT2[] = { sizeof(SimpleVertex) };
	UINT mesh_offsetsRTT2[] = { 0 };
	g_DeviceContext->OMSetDepthStencilState(g_zState, 0);
	g_DeviceContext->RSSetState(g_rasterStateCullNone);


	g_DeviceContext->IASetVertexBuffers(0, 1, meshVBRTT2, mesh_stridesRTT2, mesh_offsetsRTT2);
	g_DeviceContext->IASetIndexBuffer(SphereReflect.IBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_DeviceContext->VSSetShader(SphereReflect.VShader, 0, 0);

	g_DeviceContext->IASetInputLayout(SphereReflect.VLayout);


	ID3D11ShaderResourceView* texViews2[]{ Skybox.TextureView_SkyBox };



	g_DeviceContext->PSSetShader(SphereReflect.PShader, 0, 0);
	g_DeviceContext->PSSetShaderResources(0, 1, texViews2);
	g_DeviceContext->PSSetSamplers(0, 1, &WATA.wrapState);


	//Modify world Matrix before drawing
	//My_Matrices.time = timeX.TotalTimeExact();
	

	//SphereReflect.wvp.worldMatrix = XMMatrixMultiply(SphereReflect.wvp.worldMatrix, zero);
	//SphereReflect.wvp.worldMatrix = XMMatrixIdentity();

		SphereReflect.wvp.worldMatrix = XMMatrixMultiply(SphereReflect.wvp.worldMatrix, zeroSky);
	SphereReflect.wvp.worldMatrix = XMMatrixIdentity();

	XMMATRIX T2= XMMatrixTranslation(SphereReflect.wvp.cam.m128_f32[0], SphereReflect.wvp.cam.m128_f32[1], SphereReflect.wvp.cam.m128_f32[2]);

	T2= XMMatrixMultiply(XMMatrixScaling(1000.0f, 1000.0f, 1000.0f), T2);
	SphereReflect.wvp.worldMatrix = XMMatrixMultiply(SphereReflect.wvp.worldMatrix, T2);








	// Send it to Video Card
	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
	ID3D11Buffer* constantsRTT2[] = { SphereReflect.constantBuffer };

	g_DeviceContext->Map(SphereReflect.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((WVP*)(gpuBuffer.pData)) = SphereReflect.wvp;
	g_DeviceContext->Unmap(SphereReflect.constantBuffer, 0);

	g_DeviceContext->VSSetConstantBuffers(0, 1, constantsRTT2);

	g_DeviceContext->DrawIndexed(SphereReflect.IndexCount, 0, 0);


	g_DeviceContext->GenerateMips(SphereReflect.TextureView_RTT);
#pragma endregion

	g_DeviceContext->ClearDepthStencilView(g_zBuffer, D3D11_CLEAR_DEPTH, 1, 0); // clear it to Z exponential Far.

	g_DeviceContext->OMSetRenderTargets(1, tempRTV, g_zBuffer);
	
	camera_move(Skybox.wvp.viewMatrix);
//#pragma region Sphere
//	SphereReflect.wvp.viewMatrix = IslandModel.wvp.viewMatrix;
//
//
//	ID3D11Buffer* meshVBSphere[] = { SphereReflect.VBuffer };
//	UINT mesh_stridesSphere[] = { sizeof(SimpleVertex) };
//	UINT mesh_offsetsSphere[] = { 0 };
//
//	g_DeviceContext->IASetVertexBuffers(0, 1, meshVBSphere, mesh_stridesSphere, mesh_offsetsSphere);
//	g_DeviceContext->IASetIndexBuffer(SphereReflect.IBuffer, DXGI_FORMAT_R32_UINT, 0);
//	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	g_DeviceContext->VSSetShader(SphereReflect.VShader, 0, 0);
//
//	g_DeviceContext->IASetInputLayout(SphereReflect.VLayout);
//
//
//	ID3D11ShaderResourceView* texViewsNo[]{ Skybox.TextureView_SkyBox };
//
//
//
//	g_DeviceContext->PSSetShader(SphereReflect.PShader, 0, 0);
//	g_DeviceContext->PSSetShaderResources(0, 1, texViews2);
//	g_DeviceContext->PSSetSamplers(0, 1, &wrapState);
//
//
//	//Modify world Matrix before drawing
//	//My_Matrices.time = timeX.TotalTimeExact();
//
//
//	SphereReflect.wvp.worldMatrix = XMMatrixMultiply(SphereReflect.wvp.worldMatrix, zero);
//	SphereReflect.wvp.worldMatrix = XMMatrixIdentity();
//	SphereReflect.wvp.worldMatrix = XMMatrixTranslation(0, 10, 0);
//
//
//
//
//	SphereReflect.wvp.worldMatrix = XMMatrixMultiply(SphereReflect.wvp.worldMatrix, XMMatrixScaling(100.0f, 100.0f, 100.0f));
//
//
//
//	// Send it to Video Card
//	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
//	ID3D11Buffer* constantsSphere[] = { SphereReflect.constantBuffer };
//
//	g_DeviceContext->Map(SphereReflect.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
//	*((WVP*)(gpuBuffer.pData)) = SphereReflect.wvp;
//	g_DeviceContext->Unmap(SphereReflect.constantBuffer, 0);
//
//	g_DeviceContext->VSSetConstantBuffers(0, 1, constantsSphere);
//
//	//g_DeviceContext->DrawIndexed(SphereReflect.IndexCount, 0, 0);
//
//
//#pragma endregion


#pragma region Island
	ID3D11Buffer* meshVB[] = { IslandModel.VBuffer };
	UINT mesh_strides[] = { sizeof(SimpleVertex) };
	UINT mesh_offsets[] = { 0 };

	g_DeviceContext->IASetVertexBuffers(0, 1, meshVB, mesh_strides, mesh_offsets);
	g_DeviceContext->IASetIndexBuffer(IslandModel.IBuffer, DXGI_FORMAT_R32_UINT, 0);
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_DeviceContext->VSSetShader(IslandModel.VShader, 0, 0);

	g_DeviceContext->IASetInputLayout(IslandModel.VLayout);


	//ID3D11ShaderResourceView* texViews[]{ IslandModel.TextureView_Diffuse };



	g_DeviceContext->PSSetShader(IslandModel.PShader, 0, 0);
	g_DeviceContext->PSSetShaderResources(0, 1, texViews);
	g_DeviceContext->PSSetSamplers(0, 1, &wrapState);


	//Modify world Matrix before drawing
	//My_Matrices.time = timeX.TotalTimeExact();
	//XMMATRIX zero
	//{ 0,0,0,0,
	//  0,0,0,0,
	//  0,0,0,0,
	//  0,0,0,0 };

	IslandModel.wvp.worldMatrix = XMMatrixMultiply(IslandModel.wvp.worldMatrix, zero);
	IslandModel.wvp.worldMatrix = XMMatrixIdentity();
	IslandTexture.wvp.worldMatrix = XMMatrixTranslation(0, -100, 0);

	//IslandModel.wvp.worldMatrix = XMMatrixMultiply(IslandModel.wvp.worldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));



	// Send it to Video Card
	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
	ID3D11Buffer* constants[] = { IslandModel.constantBuffer };

	g_DeviceContext->Map(IslandModel.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((WVP*)(gpuBuffer.pData)) = IslandModel.wvp;
	g_DeviceContext->Unmap(IslandModel.constantBuffer, 0);
	g_DeviceContext->VSSetConstantBuffers(0, 1, constants);


	//Draw
	g_DeviceContext->DrawIndexed(IslandModel.IndexCount, 0, 0);
#pragma endregion
//#pragma region waterplane
//	ID3D11Buffer* meshVBW[] = { Water_Old.VBuffer };
//	UINT mesh_stridesW[] = { sizeof(SimpleVertex) };
//	UINT mesh_offsetsW[] = { 0 };
//
//	g_DeviceContext->IASetVertexBuffers(0, 1, meshVBW, mesh_stridesW, mesh_offsetsW);
//	g_DeviceContext->IASetIndexBuffer(Water_Old.IBuffer, DXGI_FORMAT_R32_UINT, 0);
//	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	g_DeviceContext->VSSetShader(WATA.VertexShader_Water, 0, 0);
//
//	g_DeviceContext->IASetInputLayout(WATA.VertexLayout);
//	
//
//	ID3D11ShaderResourceView* texViewsW[]{ Water_Old.TextureView_Diffuse,SphereReflect.TextureView_RTT, IslandTexture.TextureView_RTT };
//
//
//
//	g_DeviceContext->PSSetShader(Water_Old.PShader, 0, 0);
//	g_DeviceContext->PSSetShaderResources(0, 3, texViewsW);
//	g_DeviceContext->PSSetSamplers(0, 1, &wrapState);
//
//
//	//Modify world Matrix before drawing
//	//My_Matrices.time = timeX.TotalTimeExact();
//	//XMMATRIX zero
//	//{ 0,0,0,0,
//	//  0,0,0,0,
//	//  0,0,0,0,
//	//  0,0,0,0 };
//
//	Water_Old.wvp.worldMatrix = XMMatrixMultiply(Water_Old.wvp.worldMatrix, zero);
//	Water_Old.wvp.worldMatrix = XMMatrixIdentity();
//
//Water_Old.wvp.worldMatrix = XMMatrixMultiply(Water_Old.wvp.worldMatrix, XMMatrixScaling(300000, 300000, 300000));
//
//
//WBT.padding = { 0,0 };
//WBT.reflectRefractScale = 0.01f;
//
//	// Send it to Video Card
//	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
//	ID3D11Buffer* constantsW[] = { Water_Old.constantBuffer, g_reflectionBuffer };
//
//	g_DeviceContext->Map(Water_Old.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
//	*((WVP*)(gpuBuffer.pData)) = Water_Old.wvp;
//	g_DeviceContext->Unmap(Water_Old.constantBuffer, 0);
//
//	g_DeviceContext->Map(g_reflectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
//	*((ReflectionBufferType*)(gpuBuffer.pData)) = RBT;
//	g_DeviceContext->Unmap(g_reflectionBuffer, 0);
//	g_DeviceContext->VSSetConstantBuffers(0, 2, constantsW);
//
//	g_DeviceContext->Map(g_waterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
//	*((WaterBufferType*)(gpuBuffer.pData)) = WBT;
//	g_DeviceContext->Unmap(g_waterBuffer, 0);
//	g_DeviceContext->PSSetConstantBuffers(0, 1, &g_waterBuffer);
//
//		
//
//
//
//	//Draw
//	g_DeviceContext->DrawIndexed(Water_Old.IndexCount, 0, 0);
//	ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr};
//	g_DeviceContext->PSSetShaderResources(0, 3, null);
//
//#pragma endregion


	WATA.DrawWater(g_DeviceContext, Water_Old.VBuffer, Water_Old.IBuffer, sizeof(SimpleVertex), Water_Old.IndexCount, true);
#pragma region Skele
	static bool keypress = false;
	if (GetAsyncKeyState('P'))
	{
		keypress = false;

	}
	else if (GetAsyncKeyState('I'))
	{
		keypress = true;
	}
	static int counter = 0;
	static float elapsedTime = 0;
	elapsedTime += timeX.Delta();
	static float mod = (Anim.Animation_Clip.duration) * 0.000000000001f;
	if (keypress == false)
	{
		if (elapsedTime >= mod)
		{
			counter++;
			if (counter >= Anim.Animation_Clip.frames.size() - 1)
			{
				counter = 1;
			}
			elapsedTime = 0;
		}
	}
	else
	{
		if (GetAsyncKeyState('N'))
		{
			counter++;
			if (counter >= Anim.Animation_Clip.frames.size())
			{
				counter = 1;
			}
		}
		else if (GetAsyncKeyState('B'))
		{
			if (counter <= 1)
			{
				counter = Anim.Animation_Clip.frames.size() - 1;
			}
			counter--;
		}


	}
	for (int j = 0; j < Anim.Animation_Clip.frames[counter].jointsMatrix.size(); j++)
	{
		int parent = Anim.Animation_Clip.frames[counter].parents[j];
		if (parent != -1)
		{
			debug_renderer::draw_matrix(Anim.Animation_Clip.frames[counter].jointsMatrix[j]);
			debug_renderer::draw_to_parent(Anim.Animation_Clip.frames[counter].jointsMatrix[j], Anim.Animation_Clip.frames[counter].jointsMatrix[parent]);
		}

	}
	g_DeviceContext->UpdateSubresource(Anim.VBuffer, 0, NULL, get_line_verts(), 0, 0);

	ID3D11Buffer* tempVB[] = { Anim.VBuffer };
	UINT strides[] = { sizeof(colored_vertex) };
	UINT offsets[] = { 0 };

	g_DeviceContext->IASetVertexBuffers(0, 1, tempVB, strides, offsets);

	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	g_DeviceContext->VSSetShader(Anim.VShader, 0, 0);
	g_DeviceContext->PSSetShader(Anim.PShader, 0, 0);
	g_DeviceContext->IASetInputLayout(Anim.VLayout);

	Anim.wvp.worldMatrix = XMMatrixIdentity();
	Anim.wvp.worldMatrix = XMMatrixMultiply(Anim.wvp.worldMatrix, XMMatrixScaling(100.0f, 100.0f, 100.0f));

	Logger::log("HI");


	g_DeviceContext->Map(Anim.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
	*((WVP*)(gpuBuffer.pData)) = Anim.wvp;
	g_DeviceContext->Unmap(Anim.constantBuffer, 0);
	ID3D11Buffer* constants3[] = { Anim.constantBuffer };

	g_DeviceContext->VSSetConstantBuffers(0, 1, constants3);


	g_DeviceContext->Draw(get_line_vert_count(), 0);
	debug_renderer::clear_lines();

#pragma endregion





#pragma region Mage
		//	XMVECTOR Q1;
		//	XMVECTOR Q2;
		//
		//
		//	for(unsigned int matrixIndex = 0;matrixIndex < MageModel.Animation_Clip.frames[counter].jointsMatrix.size(); matrixIndex++ )
		//	{
		//		Q1 = XMQuaternionRotationMatrix(MageModel.Animation_Clip.frames[counter].jointsMatrix[matrixIndex]);
		//		Q2 = XMQuaternionRotationMatrix(MageModel.Animation_Clip.frames[counter + 1].jointsMatrix[matrixIndex]);
		//		
		//		  XMStoreFloat4x4 (&MageModel.wvp.AnimJoint[matrixIndex], XMMatrixRotationQuaternion(XMQuaternionSlerp(Q1, Q2, mod)) );
		//	}
		//	/*	for (unsigned int matrixIndex = 0; matrixIndex < MageModel.Animation_Clip.frames[counter].jointsMatrix.size(); matrixIndex++)
		//		{
		//			XMStoreFloat4x4 (&MageModel.wvp.AnimJoint[matrixIndex], MageModel.Animation_Clip.frames[counter].jointsMatrix[matrixIndex]);
		//		}*/
		//	
		//	ID3D11Buffer* meshVB2[] = { MageModel.VBuffer };
		//	UINT mesh_strides2[] = { sizeof(SimpleVertexAnim) };
		//	UINT mesh_offsets2[] = { 0 };
		//
		//	g_DeviceContext->IASetVertexBuffers(0, 1, meshVB2, mesh_strides2, mesh_offsets2);
		//	g_DeviceContext->IASetIndexBuffer(MageModel.IBuffer, DXGI_FORMAT_R32_UINT, 0);
		//	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//
		//	g_DeviceContext->VSSetShader(MageModel.VShader, 0, 0);
		//
		//	g_DeviceContext->IASetInputLayout(MageModel.VLayout);
		//
		//	ID3D11ShaderResourceView* texViews2[]{ MageModel.TextureView_Diffuse ,MageModel.TextureView_Emissive,MageModel.TextureView_Specular };
		//
		//
		//	g_DeviceContext->PSSetShader(MageModel.PShader, 0, 0);
		//	g_DeviceContext->PSSetShaderResources(0, 3, texViews2);
		//	g_DeviceContext->PSSetSamplers(0, 1, &wrapState);
		//
		//
		//	//Modify world Matrix before drawing
		//	//My_Matrices.time = timeX.TotalTimeExact();
		//	XMMATRIX zero2
		//	{ 0,0,0,0,
		//	  0,0,0,0,
		//	  0,0,0,0,
		//	  0,0,0,0 };
		//	//camera_move(MageModel.wvp.viewMatrix);
		//
		//	MageModel.wvp.cam = { MageModel.wvp.viewMatrix.r[3].m128_f32[0],MageModel.wvp.viewMatrix.r[3].m128_f32[1],MageModel.wvp.viewMatrix.r[3].m128_f32[2] };
		//
		//	//IslandModel.wvp.viewMatrix = (XMMatrixInverse(nullptr, IslandModel.wvp.viewMatrix));
		//
		//	MageModel.wvp.worldMatrix = XMMatrixMultiply(MageModel.wvp.worldMatrix, zero2);
		//	MageModel.wvp.worldMatrix = XMMatrixIdentity();
		//	MageModel.wvp.worldMatrix = XMMatrixMultiply(MageModel.wvp.worldMatrix, XMMatrixScaling(100.0f, 100.0f, 100.0f));
		//
		//
		//
		//	// Send it to Video Card
		//	//D3D11_MAPPED_SUBRESOURCE gpuBuffer;
		//
		//	g_DeviceContext->Map(MageModel.constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
		//	*((WVP*)(gpuBuffer.pData)) = MageModel.wvp;
		//	g_DeviceContext->Unmap(MageModel.constantBuffer, 0);
		//	ID3D11Buffer* constants2[] = { MageModel.constantBuffer };
		//
		//	g_DeviceContext->VSSetConstantBuffers(0, 1, constants2);
		//	g_DeviceContext->PSSetConstantBuffers(0, 1, constants2);
		//
		//
		//	//Draw
		//	g_DeviceContext->DrawIndexed(MageModel.IndexCount, 0, 0);
#pragma endregion




#pragma endregion

			// Present Backbuffer using Swapchain object
				// Framerate is currently unlocked, we suggest "MSI Afterburner" to track your current FPS and memory usage.

	g_Swapchain->Present(1, 0);// set first argument to 1 to enable vertical refresh sync with display
}


Water Renderer::InitializeObjects()
{
	SimpleMesh IslandMesh;
	std::fstream Ofile{ "ObjectData/Island.MObj", std::ios_base::in | std::ios_base::binary };

	if (Ofile.is_open())
	{



	}
	else
	{
		Process_OBJ("Assets/Low Poly Pirate LandscapesMAIN.obj", "ObjectData/Island.MObj");
	}
	Load_OBJ("ObjectData/Island.MObj", IslandMesh);
	D3D11_INPUT_ELEMENT_DESC meshInputEDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	vector<const wchar_t*> filePathWCharPrime;
	filePathWCharPrime.push_back(L"Assets/tex.dds");

	IslandModel = InitializeModelData(g_Device, IslandMesh.vertexList, IslandMesh.indicesList, MeshVertexShader, MeshPixelShader, sizeof(MeshVertexShader), sizeof(MeshPixelShader), meshInputEDesc, 3, filePathWCharPrime);
	IslandModel.wvp.viewMatrix = defaultView;

	IslandModel.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);


	IslandTexture = InitializeModelData(g_Device, IslandMesh.vertexList, IslandMesh.indicesList, RefractionVertexShader, RefractionPixelShader, sizeof(RefractionVertexShader), sizeof(RefractionPixelShader), meshInputEDesc, 3, filePathWCharPrime);
	IslandTexture.wvp.viewMatrix = defaultView;

	IslandTexture.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);

	IslandTexture.RenderToTexture(g_Device, g_width, g_height);

	vector<fbx_joint>skeleton;
	anim_clip clip;

	Anim_FBXSkeleton_InitLoad("Assets/AnimationAssets/MageAssets/Run.fbx", "ObjectData/Test.Anim", clip);

	Load_AnimSkeletonFBX("ObjectData/Test.Anim", skeleton);

	D3D11_INPUT_ELEMENT_DESC jointInputEDesc[]
	{

		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}

	};


	Anim = InitializeBoneData(g_Device, skeleton, LineVertexShader, LinePixelShader, sizeof(LineVertexShader), sizeof(LinePixelShader), jointInputEDesc, 2, clip);
	Anim.wvp.viewMatrix = defaultView;

	Anim.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);


#pragma region Skybox
	SimpleMesh Sky;

	FBX_InitLoad("Assets/Skybox.fbx", "ObjectData/Skybox.mFbx", "Assets/sky.fbm", "sky.materials");
	Load_FBX("ObjectData/Skybox.mFbx", Sky);
	vector<file_path_t> filePaths;
	std::fstream file{ "sky.materials", std::ios_base::in | std::ios_base::binary };

	assert(file.is_open());

	uint32_t count;
	file.read((char*)&count, sizeof(uint32_t));

	filePaths.resize(count);
	vector<const wchar_t*>filePathWChar;

	file.read((char*)filePaths.data(), sizeof(file_path_t) * count);
	if (filePaths.size() != 0)
	{
		string filePath = filePaths[0].data();
		wstring wide = wstring(filePath.begin(), filePath.end());

		filePathWChar.push_back(wide.c_str());
		if (filePaths.size() > 1)
		{
			filePath = filePaths[1].data();
			wstring wide2 = wstring(filePath.begin(), filePath.end());
			filePathWChar.push_back(wide2.c_str());

			filePath = filePaths[2].data();
			wstring wide3 = wstring(filePath.begin(), filePath.end());
			filePathWChar.push_back(wide3.c_str());
		}
	}
	D3D11_INPUT_ELEMENT_DESC skyInputEDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	Skybox = InitializeModelData(g_Device, Sky.vertexList, Sky.indicesList, SkyboxVertexShader, SkyboxPixelShader, sizeof(SkyboxVertexShader), sizeof(SkyboxPixelShader), skyInputEDesc, 3, filePathWChar);
	Skybox.wvp.viewMatrix = defaultView;
	
		CreateDDSTextureFromFile(g_Device, L"Assets/Skybox_RockyIslands.dds", (ID3D11Resource**)&Skybox.SkyCubeTexture, &Skybox.TextureView_SkyBox);
	Skybox.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);
	Skybox.RenderToTexture(g_Device, g_width, g_height);
#pragma endregion

#pragma region SphereReflect
	SimpleMesh Sphere = Sky;

	D3D11_INPUT_ELEMENT_DESC sphereInputEDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	SphereReflect = InitializeModelData(g_Device, Sphere.vertexList, Sphere.indicesList, SkyboxVertexShader, SkyboxPixelShader, sizeof(SkyboxVertexShader), sizeof(SkyboxPixelShader), sphereInputEDesc, 3, filePathWChar);
	SphereReflect.wvp.viewMatrix = defaultView;

	CreateDDSTextureFromFile(g_Device, L"Assets/marble01.dds", (ID3D11Resource**)&SphereReflect.diffuseTexture, &SphereReflect.TextureView_Diffuse);
	SphereReflect.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);
	SphereReflect.RenderToTexture(g_Device, g_width, g_height);
#pragma endregion

#pragma region Water
	SimpleMesh water;
	Water WATA;

	FBX_InitLoad("Assets/Water.fbx", "ObjectData/Water.mFbx", "Assets/water.fbm", "water.materials");
	Load_FBX("ObjectData/Water.mFbx", water);

	vector<const wchar_t*>filePathWCharWater;
	filePathWCharWater.push_back(L"Assets/normalWater.dds");
	D3D11_INPUT_ELEMENT_DESC waterInputEDesc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	Water_Old = InitializeModelData(g_Device, water.vertexList, water.indicesList, WaterVertexShader, WaterPixelShader, sizeof(WaterVertexShader), sizeof(WaterPixelShader), waterInputEDesc, 2, filePathWCharWater);
	WATA.WaterWVPCam.viewMatrix = Water_Old.wvp.viewMatrix = defaultView;

	WATA.InitializeBuffersandSamplerState(g_Device);
	WATA.InitializeShadersAndLayout(g_Device, true);
	WATA.RenderToTexture(g_Device, g_width, g_height, true);
	WATA.WaterWVPCam.projMatrix = Water_Old.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);
	WATA.SetNormalTexture(g_Device,L"Assets/normalWater.dds");
#pragma endregion

#pragma region Extra Buffers
	D3D11_BUFFER_DESC bDesc;
	//Load Mesh onto Card
	D3D11_SUBRESOURCE_DATA subData;
	ZeroMemory(&bDesc, sizeof(bDesc));
	ZeroMemory(&subData, sizeof(subData));

	bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//bDesc.ByteWidth = sizeof(GMATRIXF);//sizeof(XMFLOAT4X4);
	bDesc.ByteWidth = sizeof(XMVECTOR);
	bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bDesc.MiscFlags = 0;
	bDesc.StructureByteStride = 0;
	bDesc.Usage = D3D11_USAGE_DYNAMIC;

	subData.pSysMem = 0;

	g_Device->CreateBuffer(&bDesc, nullptr, &clipBuffer);

	bDesc.ByteWidth = sizeof(LBF);

	g_Device->CreateBuffer(&bDesc, nullptr, &lightBuffer);

	bDesc.ByteWidth = sizeof(ReflectionBufferType);

	g_Device->CreateBuffer(&bDesc, nullptr, &g_reflectionBuffer);

	bDesc.ByteWidth = sizeof(WaterBufferType);
	g_Device->CreateBuffer(&bDesc, nullptr, &g_waterBuffer);




#pragma endregion 



	//#pragma region MageInit
	//	SimpleMeshAnim Mage;
	//	std::fstream fbxfile{ "ObjectData/BattleMage.8mFbx", std::ios_base::in | std::ios_base::binary };
	//
	//	
	//	FBX_InitLoad("Assets/AnimationAssets/MageAssets/Run.fbx", "ObjectData/BattleMage.mFbx");
	//	
	//	Load_FBX("ObjectData/BattleMage.mFbx", Mage);
	//	vector<file_path_t> filePaths;
	//	std::fstream file{ "mage.materials", std::ios_base::in | std::ios_base::binary };
	//
	//	assert(file.is_open());
	//
	//	uint32_t count;
	//	file.read((char*)&count, sizeof(uint32_t));
	//
	//	filePaths.resize(count);
	//	vector<const wchar_t*>filePathWChar;
	//
	//	file.read((char*)filePaths.data(), sizeof(file_path_t) * count);
	//
	//	string filePath = filePaths[0].data();
	//	wstring wide = wstring(filePath.begin(), filePath.end());
	//	filePathWChar.push_back(wide.c_str());
	//
	//	filePath = filePaths[1].data();
	//	wstring wide2 = wstring(filePath.begin(), filePath.end());
	//	filePathWChar.push_back(wide2.c_str());
	//
	//	filePath = filePaths[2].data();
	//	wstring wide3 = wstring(filePath.begin(), filePath.end());
	//	filePathWChar.push_back(wide3.c_str());
	//
	//	D3D11_INPUT_ELEMENT_DESC animInputEDesc[]
	//	{
	//		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	//		{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	//		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	//		{"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32_SINT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	//		{"BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0}
	//
	//	};
	//
	//
	//	MageModel = InitializeAnimModelData(g_Device, Mage.vertexList, Mage.indicesList, skeleton, AnimVertexShader, AnimPixelShader, sizeof(AnimVertexShader), sizeof(AnimPixelShader), animInputEDesc, 5, filePathWChar, clip);
	//	MageModel.wvp.viewMatrix = defaultView;
	//
	//	MageModel.wvp.projMatrix = XMMatrixPerspectiveFovLH(3.14f / 2.0f, g_aspectRatio, 0.1f, 1000000);
	//#pragma endregion



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
	return WATA;

}
void Renderer::Release()
{
	this->~Renderer();


}


