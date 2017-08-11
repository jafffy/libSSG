#include <cstdio>

#include <Windows.h>
#include <d3d11.h>
#include <DXGI.h>
#include <DXGIType.h>
#include <dxgi1_3.h>
#include <d2d1_3.h>

#include "ssg.h"
#include "core\\ssg_color4.h"
#include "ssg_timer.h"
#include "ssg_dbg.h"

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
D3D_DRIVER_TYPE g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pImmediateContext = nullptr;
IDXGISwapChain1* g_pSwapChain = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
IDXGIDevice2* g_pDXGIDevice = nullptr;
IDXGIAdapter * g_pDXGIAdapter = nullptr;
IDXGIFactory2 * g_pIDXGIFactory = nullptr;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void Render();

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	{
		if (FAILED(InitWindow(hInstance, nCmdShow))) {
			return 0;
		}

		if (FAILED(InitDevice())) {
			CleanupDevice();
			return 0;
		}

		app_init();

		ssg_timer oneFrameTimer;
		oneFrameTimer.start();

		MSG msg = { 0 };

		while (WM_QUIT != msg.message) {
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				float deltaTime = oneFrameTimer.elapsedTime();
				oneFrameTimer.reset();

				app_update(deltaTime);
				app_render();

				Render();
			}
		}

		app_destroy();

		CleanupDevice();
	}

	_CrtDumpMemoryLeaks();

	return 0;
}


HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	static const LPCWSTR kClassName = L"SimpleSceneGraph";
	static const LPCWSTR kWindowTitle = app_get_window_title();

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = kClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	g_hInst = hInstance;
	RECT rc = { 0, 0, app_get_window_width(), app_get_window_height() };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(kClassName, kWindowTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
		NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC1 sd = { 0 };
	sd.Width = width;
	sd.Height = height;
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.Stereo = false;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL; // Where is DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL?
	sd.Flags = 0;
	sd.Scaling = DXGI_SCALING_STRETCH;
	sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

	for (UINT driverTypeIndex = 0;
		driverTypeIndex < numDriverTypes;
		++driverTypeIndex) {
		g_driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, 0, createDeviceFlags,
			featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
			&g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice2), (void**)&g_pDXGIDevice);
	if (FAILED(hr))
		return hr;

	hr = g_pDXGIDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&g_pDXGIAdapter);
	if (FAILED(hr))
		return hr;

	hr = g_pDXGIAdapter->GetParent(__uuidof(IDXGIFactory2), (void **)&g_pIDXGIFactory);
	if (FAILED(hr))
		return hr;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = { 0 };
	fullscreenDesc.RefreshRate.Denominator = 60;
	fullscreenDesc.RefreshRate.Numerator = 1;
	fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	fullscreenDesc.Windowed = TRUE;

	hr = g_pIDXGIFactory->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd,
		&sd, &fullscreenDesc, nullptr, &g_pSwapChain);
	if (FAILED(hr))
		return hr;

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
		&g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	return S_OK;
}

void Render()
{
	const auto clear_color = app_get_clear_color();
	float ClearColor[4] = { clear_color.r, clear_color.g, clear_color.b, clear_color.a };
	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
	g_pSwapChain->Present(0, 0);
}

void CleanupDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	if (g_pRenderTargetView) g_pRenderTargetView->Release();
	if (g_pSwapChain) g_pSwapChain->Release();
	if (g_pImmediateContext) g_pImmediateContext->Release();
	if (g_pd3dDevice) g_pd3dDevice->Release();
	if (g_pDXGIDevice) g_pDXGIDevice->Release();
	if (g_pDXGIAdapter) g_pDXGIAdapter->Release();
	if (g_pIDXGIFactory) g_pIDXGIFactory->Release();
}