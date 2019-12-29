#include "LabApp.hpp"

#include "../Extensions/ImGui/imgui_impl_win32.hpp"
#include "../Extensions/SpdLog/SinkStorage.hpp"

#include "../Core/Logging.hpp"

#include "Managers/Scene/SceneManager.hpp"
#include "Managers/Asset/AssetManager.hpp"
#include "Managers/UI/UIManager.hpp"
#include "RuntimeSharing.hpp"

#include <chrono>

#define __DIRECTX12__MODE__
#ifndef __DIRECTX12__MODE__
#define __VULKAN__MODE__
#endif

using Time = std::chrono::high_resolution_clock;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

LRTR::LabApp::LabApp(const std::string& name, size_t width, size_t height)
	: mName(name), mWidth(width), mHeight(height), mHwnd(nullptr), mExisted(false)
{
	mRuntimeSharing = std::make_shared<RuntimeSharing>(this);
	
	initializeLogComponents();
	
	const auto hInstance = GetModuleHandle(nullptr);
	const auto class_name = this->name();

	WNDCLASS appInfo;

	appInfo.style = CS_DBLCLKS;
	appInfo.lpfnWndProc = DefaultWindowProc;
	appInfo.cbClsExtra = 0;
	appInfo.cbWndExtra = 0;
	appInfo.hInstance = hInstance;
	appInfo.hIcon = static_cast<HICON>(LoadImage(nullptr, "", IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
	appInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
	appInfo.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	appInfo.lpszMenuName = nullptr;
	appInfo.lpszClassName = &class_name[0];

	RegisterClass(&appInfo);

	RECT rect;

	rect.top = 0;
	rect.left = 0;
	rect.right = static_cast<UINT>(this->width());
	rect.bottom = static_cast<UINT>(this->height());

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	mHwnd = CreateWindow(&class_name[0], &class_name[0], WS_OVERLAPPEDWINDOW ^
		WS_SIZEBOX ^ WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, hInstance, nullptr);

	mExisted = true;

	LRTR_DEBUG_INFO("Initialize LRTRApp with [{0}, {1}].", mWidth, mHeight);
	
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(mHwnd);

	initializeCodeRedComponents();
	initializeManagerComponents();
}

LRTR::LabApp::~LabApp()
{
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	mCommandQueue->waitIdle();
}

void LRTR::LabApp::show() const
{
	ShowWindow(mHwnd, SW_SHOW);
}

void LRTR::LabApp::hide() const
{
	ShowWindow(mHwnd, SW_HIDE);
}

void LRTR::LabApp::runLoop()
{
	auto currentTime = Time::now();

	while (mExisted == true) {
		MSG message;

		message.hwnd = mHwnd;

		while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);

			processMessage(this, message);

			if (message.message == WM_QUIT) mExisted = false;
		}

		if (mExisted == false) break;

		auto duration = std::chrono::duration_cast<
			std::chrono::duration<float>>(Time::now() - currentTime);

		currentTime = Time::now();

		ImGui_ImplWin32_NewFrame();

		update(duration.count());
		render(duration.count());
	}
}

void LRTR::LabApp::initializeLogComponents()
{
	const auto sink = std::make_shared<SinkStorageSingleThread>();

	//initialize spd-log interface
	spdlog::default_logger()->sinks().push_back(sink);
	CodeRed::DebugReport::listeners().push_back(sink);
}

void LRTR::LabApp::initializeCodeRedComponents()
{
	//initialize Code-Red interface
	LRTR_DEBUG_INFO("Initialize Code Red Components.");

	initializeDevice();
	initializeCommand();
	initializeSwapChain();

	LRTR_DEBUG_INFO("Finish intialize Code Red Components.");
}

void LRTR::LabApp::initializeManagerComponents()
{
	//initialize Layers
	LRTR_DEBUG_INFO("Initialize Managers.");

	initializeAssetManager();
	initializeSceneManager();
	initializeUIManager();

	LRTR_DEBUG_INFO("Finish initialize Managers.");
}

void LRTR::LabApp::update(float delta)
{
	mSceneManager->update(delta);
	mUIManager->update(delta);
}

void LRTR::LabApp::render(float delta)
{
	mCommandQueue->waitIdle();
	mCommandAllocator->reset();

	auto commandLists = mSceneManager->render(delta);

	commandLists.push_back(mUIManager->render(mFrameBuffers[mCurrentFrameIndex], delta));

	mCommandQueue->execute(commandLists);
	
	mSwapChain->present();
	
	mCurrentFrameIndex = (mCurrentFrameIndex + 1) % mSwapChain->bufferCount();
}

void LRTR::LabApp::initializeDevice()
{
#ifdef __DIRECTX12__MODE__
	auto systemInfo = std::make_shared<CodeRed::DirectX12SystemInfo>();
	auto adapters = systemInfo->selectDisplayAdapter();

	mDevice = std::make_shared<CodeRed::DirectX12LogicalDevice>(adapters[0]);
#else
	auto systemInfo = std::make_shared<CodeRed::VulkanSystemInfo>();
	auto adapters = systemInfo->selectDisplayAdapter();

	mDevice = std::make_shared<CodeRed::VulkanLogicalDevice>(adapters[0]);
#endif
}

void LRTR::LabApp::initializeCommand()
{
	mCommandAllocator = mDevice->createCommandAllocator();
	mCommandQueue = mDevice->createCommandQueue();
}

void LRTR::LabApp::initializeSwapChain()
{
	mSwapChain = mDevice->createSwapChain(
		mCommandQueue,
		{ mWidth, mHeight, mHwnd },
		CodeRed::PixelFormat::BlueGreenRedAlpha8BitUnknown,
		2
	);

	for (size_t index = 0; index < mSwapChain->bufferCount(); index++) {
		mFrameBuffers.push_back(
			mDevice->createFrameBuffer(
				mSwapChain->buffer(index)
			)
		);
	}

	mRenderPass = mDevice->createRenderPass(
		CodeRed::Attachment::RenderTarget(mSwapChain->format())
	);

	LRTR_DEBUG_INFO("Initialize Swap Chain with [{0}, {1}].",
		mSwapChain->width(), mSwapChain->height());
}

void LRTR::LabApp::initializeSceneManager()
{
	LRTR_DEBUG_INFO("Initialize Scene Manager.");

	mSceneManager = std::make_shared<SceneManager>(
		mRuntimeSharing,
		mDevice);
}

void LRTR::LabApp::initializeAssetManager()
{
	LRTR_DEBUG_INFO("Initialize Asset Manager.");

	mAssetManager = std::make_shared<AssetManager>(
		mRuntimeSharing);
}

void LRTR::LabApp::initializeUIManager()
{
	LRTR_DEBUG_INFO("Initialize UI Manager.");

	ImGui::StyleColorsLight();
	
	mUIManager = std::make_shared<UIManager>(
		mRuntimeSharing,
		mDevice,
		mRenderPass,
		mCommandAllocator,
		mCommandQueue,
		width(), height());
}

void LRTR::LabApp::processMessage(LabApp* app, const MSG& message)
{

}
