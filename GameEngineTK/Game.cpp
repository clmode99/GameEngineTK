//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include <ctime>
#include <iterator>
#include <sstream>

extern void ExitGame();

using namespace std;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

//const int Game::SPHERE_NUM = 20;			// 球の数
const int Game::TEAPOT_NUM = 20;			// ティーポットの数
const int Game::GROUND_WIDTH_HEIGHT = 200;	// 床の幅と高さ
const int Game::GROUND_NUM = Game::GROUND_WIDTH_HEIGHT*Game::GROUND_WIDTH_HEIGHT;	// 床の数


Game::Game() :
    m_window(0),
    m_outputWidth(800),
    m_outputHeight(600),
    m_featureLevel(D3D_FEATURE_LEVEL_9_1)
{
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_window = window;
	m_outputWidth = std::max(width, 1);
	m_outputHeight = std::max(height, 1);

	CreateDevice();

	CreateResources();

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/

	// TODO:初期化処理はここ
	m_batch = make_unique<PrimitiveBatch<VertexPositionColor>>(m_d3dContext.Get());

	m_sprite_batch = make_unique<SpriteBatch>(m_d3dContext.Get());
	m_sprite_font = make_unique<SpriteFont>(m_d3dDevice.Get(), L"Recources/myfile.spritefont");

	m_keyboard = make_unique<Keyboard>();

	m_effect = make_unique<BasicEffect>(m_d3dDevice.Get());

	m_effect->SetProjection(XMMatrixOrthographicOffCenterRH(0,
		m_outputWidth, m_outputHeight, 0, 0, 1));
	m_effect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	m_d3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		m_inputLayout.GetAddressOf());

	m_states = make_unique<CommonStates>(m_d3dDevice.Get());

	//m_camera = make_unique<DebugCamera>(m_outputWidth, m_outputHeight);
	Vector3 camera_pos = m_head_pos;
	camera_pos += Vector3(0.0f, 0.0f, 5.0f);
	m_camera = make_unique<Camera>(camera_pos, Vector3::Zero, Vector3::Up,
		XMConvertToDegrees(60.0f), static_cast<float>(m_outputWidth) / m_outputHeight, 0.1f, 500.0f);

	m_factory = make_unique<EffectFactory>(m_d3dDevice.Get());
	m_factory->SetDirectory(L"Recources");		// テクスチャ(.dds)のパス設定
	m_ground = Model::CreateFromCMO(m_d3dDevice.Get(), L"Recources/Ground200m.cmo", *m_factory);
	m_skydome = Model::CreateFromCMO(m_d3dDevice.Get(), L"Recources/Skydome.cmo", *m_factory);
	//m_sphere = Model::CreateFromCMO(m_d3dDevice.Get(), L"Recources/Sphere.cmo", *m_factory);
	m_teapot = Model::CreateFromCMO(m_d3dDevice.Get(), L"Recources/Teapot.cmo", *m_factory);
	m_head = Model::CreateFromCMO(m_d3dDevice.Get(), L"Recources/Head.cmo", *m_factory);

	//m_sphere_world.resize(SPHERE_NUM);
	m_ground_world.resize(GROUND_NUM);
	m_teapot_world.resize(TEAPOT_NUM);
	m_teapot_trans.resize(TEAPOT_NUM);
	m_teapot_vec.resize(TEAPOT_NUM);

	{
		// 球の初期化
		//for (int i = 0; i < SPHERE_NUM; ++i)
		//{
		//	Matrix trans;
		//	Matrix rotate;

		//	const  float ANGLE = XM_2PI / (SPHERE_NUM / 2);		// １つごとの角度

		//	if (i < (SPHERE_NUM / 2))		// 内側の円
		//	{
		//		trans  = Matrix::CreateTranslation(Vector3(20.0f, 0.0f, 0.0f));
		//		rotate = Matrix::CreateRotationY(ANGLE * i);
		//	}
		//	else							// 外側の円
		//	{
		//		static auto cnt = 0;

		//		trans  = Matrix::CreateTranslation(Vector3(40.0f, 0.0f, 0.0f));
		//		rotate = Matrix::CreateRotationY(ANGLE * cnt);

		//		++cnt;
		//	}

		//	// 行列合成。順番大事！S(Scale)R(Rotation)T(Translation)の順番が一般的
		//	m_sphere_world[i] = trans * rotate;
		//}
	}

	// 床の初期化
	for (int i = 0; i < GROUND_WIDTH_HEIGHT; ++i)		// 一応高さ
	{
		for (int j = 0; j < GROUND_WIDTH_HEIGHT; ++j)	// 一応幅
		{
			Matrix sort = Matrix::CreateTranslation(Vector3(j, 0.0f, i));	// 並べる行列
			Matrix trans = Matrix::CreateTranslation(Vector3(-100.0f, -10.0f, -100.0f));

			m_ground_world[(i*GROUND_WIDTH_HEIGHT) + j] = sort * trans;
		}
	}

	// ティーポットの座標決定
	srand(static_cast<unsigned int>(time(nullptr)));

	for (int i = 0; i < TEAPOT_NUM; ++i)
	{
		Matrix trans;

		// 角度決定
		float angle_radian = XMConvertToRadians(rand() % 361);		// ０～３６０の間の角度を取得
		Vector3 vec(cosf(angle_radian), 0.0f, sinf(angle_radian));
		m_teapot_vec[i] = vec;

		// 距離決定
		float dir = static_cast<float>(rand() % 96);		// ０～９５の間の距離を取得
		m_teapot_vec[i] *= dir;

		m_teapot_trans[i] =  Matrix::CreateTranslation(m_teapot_vec[i]);

	}
	m_time_frame = 0;
	m_auto_move_time_frame = 0;

}

// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

	// TODO:更新処理
	m_camera->Update();

	// キーボード更新
	auto kb = m_keyboard->GetState();

	static float head_angle = 0.0f;

	// モデル移動
	if (kb.W)		// 前進
	{
		//Vector3 move_front(0.0f, 0.0f, -0.1f);
		Vector3 move_front2(cosf(XMConvertToRadians(90 - head_angle)), 0.0f, sinf(XMConvertToRadians(90 - head_angle)));
		move_front2.Normalize();
		move_front2 *= -0.1f;
		m_head_pos += move_front2;
	}
	if (kb.S)		// 後退
	{
		//Vector3 move_back(0.0f, 0.0f, 0.1f);
		Vector3 move_back2(cosf(XMConvertToRadians(90 - head_angle)), 0.0f, sinf(XMConvertToRadians(90 - head_angle)));
		move_back2.Normalize();
		move_back2 *= 0.1f;
		m_head_pos += move_back2;
	}

	if (kb.A)		// 左旋回
	{
		++head_angle;
	}
	if (kb.D)		// 右旋回
	{
		--head_angle;
	}
	Matrix head_trans        = Matrix::CreateTranslation(m_head_pos);
	m_head_world_rotate = Matrix::CreateRotationY(XMConvertToRadians(head_angle));
	m_head_world = m_head_world_rotate * head_trans;	// ワールド行列変換

	// ワールド行列を計算
	//Matrix double_scale = Matrix::CreateScale(2.0f);								// 拡大縮小
	//Matrix trans        = Matrix::CreateTranslation(Vector3(0.0f, 10.0f, 0.0f));	// 移動
	//Matrix rotate_z     = Matrix::CreateRotationZ(3.14f);							// 回転

	// 回転させる行列計算
	//for (int i = 0; i < SPHERE_NUM; ++i)
	//{
	//	int angle   = 1;
	//	int angle_r = -1;

	//	Matrix rotate;

	//	if (i < (SPHERE_NUM / 2))		// 内側の円
	//	{
	//		rotate = Matrix::CreateRotationY(XMConvertToRadians(angle));
	//	}
	//	else							// 外側の円
	//	{
	//		rotate = Matrix::CreateRotationY(XMConvertToRadians(angle_r));
	//	}

	//	m_sphere_world[i] *= rotate;

	//}

	// ティーポットの行列更新
	static float angle_radian = 0.0f;		// スケール
	angle_radian += 0.02f;
	float angle_scale = ((sinf(angle_radian) + 1.0f)*2.0f) + 1.0f;		//１～５倍間を行ったり来たり

	float time_step = (m_time_frame - m_auto_move_time_frame) / 600.0f;		// 10秒＝600フレーム

	// デバッグ
	wstringstream ss;
	ss << L"TimeStep:"<<time_step << endl;		// サイン角度表示

	m_wstr = ss.str();

	for (int i = 0; i < TEAPOT_NUM; ++i)
	{		
		// Y軸に回転
		Matrix rotate_y = Matrix::CreateRotationY(angle_radian);

		// スケーリング
		Matrix scale = Matrix::CreateScale(angle_scale);

		// 線形補間で移動
		Matrix trans;
		if (time_step < 1.0f)
			trans = Matrix::CreateTranslation(Vector3::Lerp(m_teapot_vec[i], Vector3::Zero, time_step));

		m_teapot_world[i] = scale * rotate_y * trans;
	}

	++m_time_frame;

	// カメラの設定
	const float CAMERA_DIS = 3.0f;
	Vector3 ref_pos = m_head_pos + Vector3(0, 1, 0);
	Vector3 camera_pos2(0.0f, 0.0f, CAMERA_DIS);

	Matrix rot = Matrix::CreateRotationY(XMConvertToRadians(head_angle));
	camera_pos2 = Vector3::TransformNormal(camera_pos2, rot);

	Vector3 camera_pos = ref_pos + camera_pos2;

	m_camera->SetCameraPos(camera_pos);
	m_camera->SetRefPos(ref_pos);

	m_camera->Update();
	m_view = m_camera->GetViewMatrix();
	m_proj = m_camera->GetProjectionMatrix();
}

// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();	// 深度バッファのクリア

	// TODO:描画処理
	m_d3dContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);		// Opaque＝不透明
	m_d3dContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	m_d3dContext->RSSetState(m_states->CullNone());

	//m_view = Matrix::CreateLookAt(Vector3(0.f, 0.f, 2.f),	// カメラ視点
	//		 Vector3(0, 0, 0),		// カメラ参照点
	//		 Vector3(0, 1, 0)		// 画面の上方向ベクトル
	//);

	m_world = Matrix::Identity;

	
	//Vector3 camera_pos(0.0f, 0.0f, 5.0f);	// カメラの位置
	//Vector3 ref_pos(0.0f, 0.0f, 0.0f);				// どこを見ているか
	//Vector3 up_vec(0.0f, 1.0f, 0.0f);				// カメラの上方向(長さ１の単位ベクトルを設定)
	//m_view = Matrix::CreateLookAt(camera_pos, ref_pos, up_vec);

	//m_view  = m_camera->GetCameraMatrix();
	
	//float fovY      = XMConvertToDegrees(60.0f);		// 縦方向にどこまで移すか
	//float aspect    = static_cast<float>(m_outputWidth) / m_outputHeight;
	//float near_clip = 0.1f;
	//float far_clip  = 1000.0f;

	//m_proj = Matrix::CreatePerspectiveFieldOfView(fovY, aspect, near_clip, far_clip);
	//m_proj  = Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f,	// 視野角
	//	float(m_outputWidth) / float(m_outputHeight),			// アスペクト比
	//	0.1f,			// nearクリップ
	//	500.f			// farクリップ
	//);

	m_effect->SetWorld(m_world);
	m_effect->SetView(m_view);
	m_effect->SetProjection(m_proj);

	m_effect->Apply(m_d3dContext.Get());
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	//m_ground ->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);		// 第６引数をtrueにするとワイヤー表示になる(デフォルトはfalse)
	m_skydome->Draw(m_d3dContext.Get(), *m_states, m_world, m_view, m_proj);

	//for (vector<Matrix>::iterator i = m_sphere_world.begin(); i != m_sphere_world.end(); ++i)
	//{
	//	m_sphere->Draw(m_d3dContext.Get(), *m_states, *i, m_view, m_proj);
	//}

	// forで回す単純な地面の描画(かなり重い)
	//for (vector<Matrix>::iterator i = m_ground_world.begin(); i != m_ground_world.end(); ++i)
	//{
	//	m_ground->Draw(m_d3dContext.Get(), *m_states, *i, m_view, m_proj);
	//}

	m_ground->Draw(m_d3dContext.Get(), *m_states, Matrix::Identity, m_view, m_proj);		// タイリングした地面を描画
	
	//for (vector<Matrix>::iterator i = m_teapot_world.begin(); i != m_teapot_world.end(); ++i)
	//{
	//	m_teapot->Draw(m_d3dContext.Get(), *m_states, *i, m_view, m_proj);
	//}

	m_head->Draw(m_d3dContext.Get(), *m_states, m_head_world, m_view, m_proj);

	m_sprite_batch->Begin();

	//m_sprite_font->DrawString(m_sprite_batch.get(), m_wstr.c_str(), XMFLOAT2(0, 0));

	m_sprite_batch->End();

    Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    // Clear the views.
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::CornflowerBlue);
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

    // Set the viewport.
    CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

// Presents the back buffer contents to the screen.
void Game::Present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr = m_swapChain->Present(1, 0);

    // If the device was reset we must completely reinitialize the renderer.
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        OnDeviceLost();
    }
    else
    {
        DX::ThrowIfFailed(hr);
    }
}

// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = std::max(width, 1);
    m_outputHeight = std::max(height, 1);

    CreateResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}

// These are the resources that depend on the device.
void Game::CreateDevice()
{
    UINT creationFlags = 0;

#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    static const D3D_FEATURE_LEVEL featureLevels [] =
    {
        // TODO: Modify for supported Direct3D feature levels (see code below related to 11.1 fallback handling).
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    // Create the DX11 API device object, and get a corresponding context.
    HRESULT hr = D3D11CreateDevice(
        nullptr,                                // specify nullptr to use the default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        creationFlags,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        m_d3dDevice.ReleaseAndGetAddressOf(),   // returns the Direct3D device created
        &m_featureLevel,                        // returns feature level of device created
        m_d3dContext.ReleaseAndGetAddressOf()   // returns the device immediate context
        );

    if (hr == E_INVALIDARG)
    {
        // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it.
        hr = D3D11CreateDevice(nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            creationFlags,
            &featureLevels[1],
            _countof(featureLevels) - 1,
            D3D11_SDK_VERSION,
            m_d3dDevice.ReleaseAndGetAddressOf(),
            &m_featureLevel,
            m_d3dContext.ReleaseAndGetAddressOf()
            );
    }

    DX::ThrowIfFailed(hr);

#ifndef NDEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(m_d3dDevice.As(&d3dDebug)))
    {
        ComPtr<ID3D11InfoQueue> d3dInfoQueue;
        if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
        {
#ifdef _DEBUG
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif
            D3D11_MESSAGE_ID hide [] =
            {
                D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
                // TODO: Add more message IDs here as needed.
            };
            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
        }
    }
#endif

    // DirectX 11.1 if present
    if (SUCCEEDED(m_d3dDevice.As(&m_d3dDevice1)))
        (void)m_d3dContext.As(&m_d3dContext1);

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateResources()
{
    // Clear the previous window size specific context.
    ID3D11RenderTargetView* nullViews [] = { nullptr };
    m_d3dContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    UINT backBufferHeight = static_cast<UINT>(m_outputHeight);
    DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
    DXGI_FORMAT depthBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    UINT backBufferCount = 2;

    // If the swap chain already exists, resize it, otherwise create one.
    if (m_swapChain)
    {
        HRESULT hr = m_swapChain->ResizeBuffers(backBufferCount, backBufferWidth, backBufferHeight, backBufferFormat, 0);

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            // If the device was removed for any reason, a new device and swap chain will need to be created.
            OnDeviceLost();

            // Everything is set up now. Do not continue execution of this method. OnDeviceLost will reenter this method 
            // and correctly set up the new device.
            return;
        }
        else
        {
            DX::ThrowIfFailed(hr);
        }
    }
    else
    {
        // First, retrieve the underlying DXGI Device from the D3D Device.
        ComPtr<IDXGIDevice1> dxgiDevice;
        DX::ThrowIfFailed(m_d3dDevice.As(&dxgiDevice));

        // Identify the physical adapter (GPU or card) this device is running on.
        ComPtr<IDXGIAdapter> dxgiAdapter;
        DX::ThrowIfFailed(dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf()));

        // And obtain the factory object that created it.
        ComPtr<IDXGIFactory1> dxgiFactory;
        DX::ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf())));

        ComPtr<IDXGIFactory2> dxgiFactory2;
        if (SUCCEEDED(dxgiFactory.As(&dxgiFactory2)))
        {
            // DirectX 11.1 or later

            // Create a descriptor for the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
            swapChainDesc.Width = backBufferWidth;
            swapChainDesc.Height = backBufferHeight;
            swapChainDesc.Format = backBufferFormat;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = backBufferCount;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
            fsSwapChainDesc.Windowed = TRUE;

            // Create a SwapChain from a Win32 window.
            DX::ThrowIfFailed(dxgiFactory2->CreateSwapChainForHwnd(
                m_d3dDevice.Get(),
                m_window,
                &swapChainDesc,
                &fsSwapChainDesc,
                nullptr,
                m_swapChain1.ReleaseAndGetAddressOf()
                ));

            DX::ThrowIfFailed(m_swapChain1.As(&m_swapChain));
        }
        else
        {
            DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
            swapChainDesc.BufferCount = backBufferCount;
            swapChainDesc.BufferDesc.Width = backBufferWidth;
            swapChainDesc.BufferDesc.Height = backBufferHeight;
            swapChainDesc.BufferDesc.Format = backBufferFormat;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.OutputWindow = m_window;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.Windowed = TRUE;

            DX::ThrowIfFailed(dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &swapChainDesc, m_swapChain.ReleaseAndGetAddressOf()));
        }

        // This template does not support exclusive fullscreen mode and prevents DXGI from responding to the ALT+ENTER shortcut.
        DX::ThrowIfFailed(dxgiFactory->MakeWindowAssociation(m_window, DXGI_MWA_NO_ALT_ENTER));
    }

    // Obtain the backbuffer for this window which will be the final 3D rendertarget.
    ComPtr<ID3D11Texture2D> backBuffer;
    DX::ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())));

    // Create a view interface on the rendertarget to use on bind.
    DX::ThrowIfFailed(m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.ReleaseAndGetAddressOf()));

    // Allocate a 2-D surface as the depth/stencil buffer and
    // create a DepthStencil view on this surface to use on bind.
    CD3D11_TEXTURE2D_DESC depthStencilDesc(depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    DX::ThrowIfFailed(m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf()));

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
    DX::ThrowIfFailed(m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    // TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    m_depthStencilView.Reset();
    m_renderTargetView.Reset();
    m_swapChain1.Reset();
    m_swapChain.Reset();
    m_d3dContext1.Reset();
    m_d3dContext.Reset();
    m_d3dDevice1.Reset();
    m_d3dDevice.Reset();

    CreateDevice();

    CreateResources();
}