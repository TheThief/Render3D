#if __GNUC__
#define UNICODE
#endif

#define _WIN32_WINNT 0x0500
#define NOMINMAX
#include <Windows.h>

#if defined(_M_IX86) || defined(_M_X64)
#define SSE_VERSION 0
#endif

#include "renderer/threaded_renderer.h"
#include "math/scalarmath.h"
#include "array2d.h"
#include "math/colour.h"
#include "renderer/renderer.h"
#include "math/matrixmath.h"
#include "renderer/obj/objmesh.h"

#define STBI_ONLY_PNG 1
#include <stb/stb_image.h>

#include <float.h>
#include <limits.h>

//#include <iostream>
#include <memory>
#include <vector>

#include "resource.h"

#include "ForwardRender.h"
#include "DeferredRender.h"

//unsigned short ExtractDepthFunc(const float& p)
//{
//	return (unsigned short)(Clamp(Parametize(p, 0.8f, 1.0f), 0.0f, 1.0f) * USHRT_MAX);
//}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND hWnd = NULL;
BITMAPINFO* pBmi = NULL;

Array2d<ARGB> ColourBuffer(512, 512);
Array2d<float> DepthBuffer(512, 512);
std::unique_ptr<Renderer<float, ARGB>> Render3d;

Array2d<ARGB> DiffuseBuffer(512, 512);
Array2d<Vector3> WorldPositionBuffer(512, 512);
Array2d<Vector3> NormalBuffer(512, 512);
std::unique_ptr<Renderer<float, ARGB, Vector3, Vector3>> DeferredRender3d;

std::vector<TextureVertex> BoxVerts;
std::vector<byte> BoxIndices;
Array2d<ARGB> BoxTexture;

Matrix Projection;

deferred_point_light_mesh point_light_mesh;

renderer::objmesh model;

Array2d<ARGB> spot_texture;

Vector3 Location;
Vector3 Rotation;
Vector3 LightPosition;
float LightIntensity;
float LightRadius;

bool bDepthPrepass = false;
bool bOverdrawTest = false;
bool bDeferredShading = false;

bool draw_model = true;

enum class OutputView
{
	Result,
	DepthBuffer,
	DiffuseBuffer,
	NormalBuffer,
};
OutputView ViewType = OutputView::Result;

void Init();
bool Update();
void Render();

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = 0;
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = TEXT("Render3D");
	wcex.hIconSm = 0;

	RegisterClassEx(&wcex);

	RECT WindowRect = {0,0, 512,512};
	constexpr auto WindowStyle = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
	AdjustWindowRect(&WindowRect, WindowStyle, true);
	hWnd = CreateWindow(TEXT("Render3D"), TEXT("Render3D Demo"), WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top, NULL, NULL, hInstance, NULL);

	BITMAPV5HEADER bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bV5Size = sizeof(bmi);
	bmi.bV5Width = 512;
	bmi.bV5Height = -512;
	bmi.bV5Planes = 1;
	bmi.bV5BitCount = 32;
	bmi.bV5Compression = BI_BITFIELDS;
	bmi.bV5RedMask   = 0x00FF0000;
	bmi.bV5GreenMask = 0x0000FF00;
	bmi.bV5BlueMask  = 0x000000FF;
	bmi.bV5AlphaMask = 0xFF000000;
	pBmi = (BITMAPINFO*)&bmi;

	Init();

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	do
	{
		while(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) && (msg.message!=WM_QUIT))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(msg.message==WM_QUIT)
		{
			break;
		}

		if(!Update())
			WaitMessage();
	}
	while (msg.message!=WM_QUIT);

	Render3d = nullptr;

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		{
			if (Render3d)
			{
				Render3d->Finalize();
			}

			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hWnd, &ps);

			int Width = ps.rcPaint.right - ps.rcPaint.left;
			int Height = ps.rcPaint.bottom - ps.rcPaint.top;

			switch (ViewType)
			{
			case OutputView::Result:
				SetDIBitsToDevice(hdc,
					ps.rcPaint.left, ps.rcPaint.top,
					Width, Height,
					ps.rcPaint.left, 512 - ps.rcPaint.bottom,
					0, 512, (void*)ColourBuffer.GetDataPointer(), pBmi, DIB_RGB_COLORS);
				break;
			case OutputView::DepthBuffer:
				{
					Array2d<ARGB> Temp(Width, Height);

					for (unsigned int y : NumericRange<unsigned int>(0, Height))
					{
						auto* DepthLine = DepthBuffer.GetScanlineDataPointer(y + ps.rcPaint.top) + ps.rcPaint.left;
						auto* TempLine  = Temp.GetScanlineDataPointer(y);
						for (unsigned int x : NumericRange<unsigned int>(0, Width))
						{
							float Depth = DepthLine[x];
							TempLine[x] = ARGB((byte)(Depth * 256), (byte)(Depth * 256), (byte)(Depth * 256));
						}
					}
					BITMAPV5HEADER bmi;
					ZeroMemory(&bmi, sizeof(bmi));
					bmi.bV5Size = sizeof(bmi);
					bmi.bV5Width = Width;
					bmi.bV5Height = -Height;
					bmi.bV5Planes = 1;
					bmi.bV5BitCount = 32;
					bmi.bV5Compression = BI_BITFIELDS;
					bmi.bV5RedMask   = 0x00FF0000;
					bmi.bV5GreenMask = 0x0000FF00;
					bmi.bV5BlueMask  = 0x000000FF;
					bmi.bV5AlphaMask = 0xFF000000;

					SetDIBitsToDevice(hdc,
						ps.rcPaint.left, ps.rcPaint.top,
						Width, Height,
						0, 0,
						0, Height, (void*)Temp.GetDataPointer(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
				}
				break;
			case OutputView::DiffuseBuffer:
				SetDIBitsToDevice(hdc,
					ps.rcPaint.left, ps.rcPaint.top,
					Width, Height,
					ps.rcPaint.left, 512 - ps.rcPaint.bottom,
					0, 512, (void*)DiffuseBuffer.GetDataPointer(), pBmi, DIB_RGB_COLORS);
				break;
			case OutputView::NormalBuffer:
				{
					Array2d<ARGB> Temp(Width, Height);

					for (unsigned int y : NumericRange<unsigned int>(0, Height))
					{
						auto* NormalLine = NormalBuffer.GetScanlineDataPointer(y + ps.rcPaint.top) + ps.rcPaint.left;
						auto* TempLine = Temp.GetScanlineDataPointer(y);
						for (unsigned int x : NumericRange<unsigned int>(0, Width))
						{
							Vector4 Normal = NormalLine[x];
							TempLine[x] = ARGB((byte)(Normal.x * 127 + 128), (byte)(Normal.y * 127 + 128), (byte)(Normal.z * 127 + 128));
						}
					}

					BITMAPV5HEADER bmi;
					ZeroMemory(&bmi, sizeof(bmi));
					bmi.bV5Size = sizeof(bmi);
					bmi.bV5Width = Width;
					bmi.bV5Height = -Height;
					bmi.bV5Planes = 1;
					bmi.bV5BitCount = 32;
					bmi.bV5Compression = BI_BITFIELDS;
					bmi.bV5RedMask   = 0x00FF0000;
					bmi.bV5GreenMask = 0x0000FF00;
					bmi.bV5BlueMask  = 0x000000FF;
					bmi.bV5AlphaMask = 0xFF000000;

					SetDIBitsToDevice(hdc,
						ps.rcPaint.left, ps.rcPaint.top,
						Width, Height,
						0, 0,
						0, Height, (void*)Temp.GetDataPointer(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS);
			}
				break;
			default:
				break;
			}

			EndPaint(hWnd, &ps);
		}
		break;
	//case WM_LBUTTONUP:
	//	{
	//		int iy = HIWORD(lParam);
	//		int ix = LOWORD(lParam);
	//	}
	//	break;
	//case WM_RBUTTONUP:
	//	{
	//	}
	//	break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_FILE_EXIT:
				{
					PostQuitMessage(0);
				}
				break;
			case ID_OPTIONS_DEPTHPREPASS:
				{
					bDepthPrepass = !bDepthPrepass;
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DEPTHPREPASS, bDepthPrepass ? MF_CHECKED : MF_UNCHECKED);
				}
				break;
			case ID_OPTIONS_OVERDRAWTEST:
				{
					bOverdrawTest = !bOverdrawTest;
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OVERDRAWTEST, bOverdrawTest ? MF_CHECKED : MF_UNCHECKED);
				}
				break;
			case ID_OPTIONS_DEFERREDSHADING:
				{
					bDeferredShading = !bDeferredShading;
					CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DEFERREDSHADING, bDeferredShading ? MF_CHECKED : MF_UNCHECKED);
				}
				break;
			case ID_OPTIONS_DRAWMODEL:
			{
				draw_model = !draw_model;
				CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_DRAWMODEL, draw_model ? MF_CHECKED : MF_UNCHECKED);
			}
			break;
			case ID_VIEW_RESULT:
				{
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_RESULT + (UINT)ViewType, MF_UNCHECKED);
					ViewType = OutputView::Result;
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_RESULT, MF_CHECKED);
				}
				break;
			case ID_VIEW_DEPTHBUFFER:
				{
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_RESULT + (UINT)ViewType, MF_UNCHECKED);
					ViewType = OutputView::DepthBuffer;
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_DEPTHBUFFER, MF_CHECKED);
				}
				break;
			case ID_VIEW_DIFFUSEBUFFER:
				{
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_RESULT + (UINT)ViewType, MF_UNCHECKED);
					ViewType = OutputView::DiffuseBuffer;
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_DIFFUSEBUFFER, MF_CHECKED);
				}
				break;
			case ID_VIEW_NORMALSBUFFER:
				{
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_RESULT + (UINT)ViewType, MF_UNCHECKED);
					ViewType = OutputView::NormalBuffer;
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_NORMALSBUFFER, MF_CHECKED);
				}
				break;
			}
		}
		break;
	case WM_KEYDOWN:
		{
			int repeats = (lParam & 0x0f);
			switch(wParam)
			{
			case VK_LEFT:
				{
					Rotation.y -= 2 * repeats;
				}
				break;
			case VK_RIGHT:
				{
					Rotation.y += 2 * repeats;
				}
				break;
			case VK_UP:
				{
					Location += ( Vector4(0, 0, 0.05f) * Matrix::ConstructYRotation(-Rotation.y * (float)M_PI_2 / 90) ).xyz() * (float)repeats;
				}
				break;
			case VK_DOWN:
				{
					Location -= ( Vector4(0, 0, 0.05f) * Matrix::ConstructYRotation(-Rotation.y * (float)M_PI_2 / 90) ).xyz() * (float)repeats;
				}
				break;
			case VK_ADD:
				{
					LightPosition.y -= 0.05f * repeats;
				}
				break;
			case VK_SUBTRACT:
				{
					LightPosition.y += 0.05f * repeats;
				}
				break;
			}
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void Init()
{
	// front
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1, -1), Vector4( 0,  0,  1, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1, -1), Vector4( 0,  0,  1, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1, -1), Vector4( 0,  0,  1, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1, -1), Vector4( 0,  0,  1, 0), Vector2(1,1)));
	BoxIndices.push_back( 0); BoxIndices.push_back( 1); BoxIndices.push_back( 2);
	BoxIndices.push_back( 2); BoxIndices.push_back( 1); BoxIndices.push_back( 3);

	// back
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1,  1), Vector4( 0,  0, -1, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1,  1), Vector4( 0,  0, -1, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1,  1), Vector4( 0,  0, -1, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1,  1), Vector4( 0,  0, -1, 0), Vector2(1,1)));
	BoxIndices.push_back( 4); BoxIndices.push_back( 5); BoxIndices.push_back( 6);
	BoxIndices.push_back( 6); BoxIndices.push_back( 5); BoxIndices.push_back( 7);

	// bottom
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1, -1), Vector4( 0,  1,  0, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1,  1), Vector4( 0,  1,  0, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1, -1), Vector4( 0,  1,  0, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1,  1), Vector4( 0,  1,  0, 0), Vector2(1,1)));
	BoxIndices.push_back( 8); BoxIndices.push_back( 9); BoxIndices.push_back(10);
	BoxIndices.push_back(10); BoxIndices.push_back( 9); BoxIndices.push_back(11);

	// top
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1, -1), Vector4( 0, -1,  0, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1,  1), Vector4( 0, -1,  0, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1, -1), Vector4( 0, -1,  0, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1,  1), Vector4( 0, -1,  0, 0), Vector2(1,1)));
	BoxIndices.push_back(12); BoxIndices.push_back(13); BoxIndices.push_back(14);
	BoxIndices.push_back(14); BoxIndices.push_back(13); BoxIndices.push_back(15);

	// left
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1, -1), Vector4( 1,  0,  0, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1, -1), Vector4( 1,  0,  0, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4(-1, -1,  1), Vector4( 1,  0,  0, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4(-1,  1,  1), Vector4( 1,  0,  0, 0), Vector2(1,1)));
	BoxIndices.push_back(16); BoxIndices.push_back(17); BoxIndices.push_back(18);
	BoxIndices.push_back(18); BoxIndices.push_back(17); BoxIndices.push_back(19);

	// right
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1,  1), Vector4(-1,  0,  0, 0), Vector2(0,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1,  1), Vector4(-1,  0,  0, 0), Vector2(0,1)));
	BoxVerts.push_back(TextureVertex(Vector4( 1, -1, -1), Vector4(-1,  0,  0, 0), Vector2(1,0)));
	BoxVerts.push_back(TextureVertex(Vector4( 1,  1, -1), Vector4(-1,  0,  0, 0), Vector2(1,1)));
	BoxIndices.push_back(20); BoxIndices.push_back(21); BoxIndices.push_back(22);
	BoxIndices.push_back(22); BoxIndices.push_back(21); BoxIndices.push_back(23);

	{
		stbi_set_flip_vertically_on_load(true);

		using image_ptr = std::unique_ptr<uint8_t[], decltype(&stbi_image_free)>;
		int width, height, bpp;
		image_ptr stbi_boxgrid = image_ptr(stbi_load("demo_app/BoxGrid.png", &width, &height, &bpp, 4), &stbi_image_free);
		if (!stbi_boxgrid)
			throw "BoxGrid.png not found";
		BoxTexture = Array2d<ARGB>(width, height);
		std::transform((ARGB*)stbi_boxgrid.get(), ((ARGB*)stbi_boxgrid.get()) + width * height, BoxTexture.GetDataPointer(),
			[](ARGB colour) { return ARGB(colour.B, colour.G, colour.R, colour.A); });
	}

	model = renderer::load_obj(L"demo_app/spot/spot.obj");

	// hack - todo
	{
		stbi_set_flip_vertically_on_load(true);

		using image_ptr = std::unique_ptr<uint8_t[], decltype(&stbi_image_free)>;
		int width, height, bpp;
		image_ptr stbi_spot = image_ptr(stbi_load("demo_app/spot/spot_texture.png", &width, &height, &bpp, 4), &stbi_image_free);
		if (!stbi_spot)
			throw "spot_texture.png not found";
		spot_texture = Array2d<ARGB>(width, height);
		std::transform((ARGB*)stbi_spot.get(), ((ARGB*)stbi_spot.get()) + width * height, spot_texture.GetDataPointer(),
			[](ARGB colour) { return ARGB(colour.B, colour.G, colour.R, colour.A); });
	}

	Location = Vector3(-0.1f, 0, -0.5f);
	Rotation = Vector3(0, 10, 0);
	LightPosition = Vector3(0, 0.3f, 0);
	LightIntensity = 0.8f;
	//LightRadius = sqrt(255 * LightIntensity);
	LightRadius = 1.8f;

	Projection = Matrix::ConstructPerspective_LH() * Matrix::ConstructScale(Vector3(256, -256, 1)) * Matrix::ConstructTranslation(Vector3(256, 256, 0));

	Render3d = std::make_unique<Renderer<float, ARGB>>(DepthBuffer, ColourBuffer);
	DeferredRender3d = std::make_unique<Renderer<float, ARGB, Vector3, Vector3>>(DepthBuffer, DiffuseBuffer, WorldPositionBuffer, NormalBuffer);
}

const LARGE_INTEGER tickFrequency = []
{
	LARGE_INTEGER _Frequency;
	QueryPerformanceFrequency(&_Frequency);
	return _Frequency;
}();

bool Update()
{
	static LARGE_INTEGER startTicks = { 0 };
	if (startTicks.QuadPart == 0)
	{
		QueryPerformanceCounter(&startTicks);
	}

	Render();

	InvalidateRect(hWnd, NULL, false);

	LARGE_INTEGER endTicks;
	QueryPerformanceCounter(&endTicks);

	int msFrameTime = (int)(((endTicks.QuadPart - startTicks.QuadPart) * 1000) / tickFrequency.QuadPart);
	int usFrameTime = (int)(((endTicks.QuadPart - startTicks.QuadPart) * 1000000) / tickFrequency.QuadPart) % 1000;
	int fps = (int)(tickFrequency.QuadPart / (endTicks.QuadPart - startTicks.QuadPart));

	wchar_t sNewTitle[128];
	swprintf(sNewTitle, 128, L"Render3D Demo: %d.%03dms %d FPS", msFrameTime, usFrameTime, fps);
	SetWindowText(hWnd, sNewTitle);

	startTicks = endTicks;

	return false;
}

void Render()
{
	Render3d->Finalize();

	Render3d->Fill(FLT_MAX, ARGB(255, 0, 255));

	VertexGlobals VGlobs;
	VGlobs.ViewProjection = Matrix::ConstructTranslation(-Location) * Matrix::ConstructYRotation(Rotation.y * (float)M_PI_2 / 90) * Projection;

	std::vector<TextureVertexInterpolants> ProcessedVerts_Outer;
	std::vector<TextureVertexInterpolants> ProcessedVerts_Inner;
	{
		VGlobs.ObjectTransform = Matrix();
		ProcessedVerts_Outer = ProcessToNew1d<std::vector<TextureVertexInterpolants>>([&](auto&& v) { return TextureVertexVertexFunc(VGlobs, v); }, BoxVerts);

		if (bOverdrawTest)
		{
			VGlobs.ObjectTransform = Matrix::ConstructScale(Vector3(0.9f, 0.9f, 0.9f));
			ProcessedVerts_Inner = ProcessToNew1d<std::vector<TextureVertexInterpolants>>([&](auto&& v) { return TextureVertexVertexFunc(VGlobs, v); }, BoxVerts);
		}
	}

	std::vector<TextureVertexInterpolants> processed_verts_obj;
	if (draw_model)
	{
		VertexGlobals VGlobs_spot = VGlobs;
		VGlobs_spot.ObjectTransform = Matrix::ConstructScale(0.5f) * Matrix::ConstructTranslation({ 0, -0.5f, 0.5f });

		processed_verts_obj = ProcessToNew1d<std::vector<TextureVertexInterpolants>>(
			[&](Vector3 position, Vector3 normal, Vector2 tex_coord) { return TextureVertexVertexFunc(VGlobs_spot, TextureVertex(position, normal, tex_coord)); },
			model.elements[0].mesh.positions, model.elements[0].mesh.normals, model.elements[0].mesh.tex_coords);
	}

	if (bDeferredShading)
	{
		Fill2d(DiffuseBuffer, ARGB(255, 0, 255));
		Fill2d(WorldPositionBuffer, Vector3(FLT_MAX, FLT_MAX, FLT_MAX));

		PixelGlobals_Deferred_InitialPass PGlobs1 = { BoxTexture };
		PixelGlobals_Deferred_InitialPass PGlobs1_spot = { spot_texture };

		if (bDepthPrepass)
		{
			DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs1, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs1, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs1_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}

			DeferredRender3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}
		}
		else
		{
			DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				DeferredRender3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_Deferred_InitialPass>(PGlobs1_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}
		}

		VertexGlobals VGlobs_DeferredLight;
		VGlobs_DeferredLight.ViewProjection = Matrix::ConstructTranslation(-Location) * Matrix::ConstructYRotation(Rotation.y * (float)M_PI_2 / 90) * Projection;

		VGlobs_DeferredLight.ObjectTransform = Matrix::ConstructScale(Vector3(LightRadius)) /* * Matrix::ConstructTranslation(Vector3(0, 0, LightRadius))*/ * Matrix::ConstructYRotation(-Rotation.y * (float)M_PI_2 / 90) * Matrix::ConstructTranslation(LightPosition);
		const auto ProcessedVerts_Light = ProcessToNew1d<std::vector<DeferredLightVertexInterpolants>>([&](auto&& v) { return DeferredLightVertexVertexFunc(VGlobs_DeferredLight, v); }, point_light_mesh.positions);

		DeferredRender3d->Finalize();

		PixelGlobals_Deferred_LightingPass PGlobs2 = { DiffuseBuffer, WorldPositionBuffer, NormalBuffer, LightPosition, LightIntensity };
		Render3d->DrawIndexedTriList<DepthComparator_Deferred_Light, DeferredLightVertexPixelFunc_Deferred_LightingPass>(PGlobs2, ProcessedVerts_Light, point_light_mesh.indices);
	}
	else
	{
		PixelGlobals PGlobs = { BoxTexture, LightPosition, LightIntensity };

		if (bDepthPrepass)
		{
			Render3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				Render3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				PixelGlobals PGlobs_spot = { spot_texture, LightPosition, LightIntensity };
				Render3d->DrawIndexedTriList<DepthComparator_Less_Set>(PGlobs_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}

			Render3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_ColourPass>(PGlobs, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				Render3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_ColourPass>(PGlobs, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				PixelGlobals PGlobs_spot = { spot_texture, LightPosition, LightIntensity };
				Render3d->DrawIndexedTriList<DepthComparator_Equal_NoSet, TextureVertexPixelFunc_ColourPass>(PGlobs_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}
		}
		else
		{
			Render3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_ColourPass>(PGlobs, ProcessedVerts_Outer, BoxIndices);
			if (bOverdrawTest)
				Render3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_ColourPass>(PGlobs, ProcessedVerts_Inner, BoxIndices);
			if (draw_model)
			{
				PixelGlobals PGlobs_spot = { spot_texture, LightPosition, LightIntensity };
				Render3d->DrawIndexedTriList<DepthComparator_Less_Set, TextureVertexPixelFunc_ColourPass>(PGlobs_spot, processed_verts_obj, model.elements[0].mesh.indices);
			}
		}
	}
}
