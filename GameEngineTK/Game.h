//
// Game.h
//

#pragma once

#include "StepTimer.h"

#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Effects.h>
#include <CommonStates.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Model.h>

#include <vector>

#include "DebugCamera.h"
#include "FollowCamera.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:

    Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();
    void Present();

    void CreateDevice();
    void CreateResources();

    void OnDeviceLost();

    // Device resources.
    HWND                                            m_window;
    int                                             m_outputWidth;
    int                                             m_outputHeight;

    D3D_FEATURE_LEVEL                               m_featureLevel;
    Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11Device1>           m_d3dDevice1;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    m_d3dContext1;

    Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;
    Microsoft::WRL::ComPtr<IDXGISwapChain1>         m_swapChain1;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	std::unique_ptr<DirectX::BasicEffect>      m_effect;
	std::unique_ptr<DirectX::CommonStates>     m_states;		// 汎用ステート
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;

	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;		// 射影行列

	//std::unique_ptr<DebugCamera>            m_camera;

	std::unique_ptr<DirectX::EffectFactory>   m_factory;		// エフェクトファクトリー(モデル描画に必要)
	
	static const int GROUND_NUM;
	static const int GROUND_WIDTH_HEIGHT;
	std::unique_ptr<DirectX::Model>           m_ground;			// 地面
	std::vector<DirectX::SimpleMath::Matrix>  m_ground_world;	// 地面用のワールド行列

	std::unique_ptr<DirectX::Model>         m_skydome;		// 天球

	//static const int SPHERE_NUM;
	//std::unique_ptr<DirectX::Model>           m_sphere;			// 球
	//std::vector<DirectX::SimpleMath::Matrix>  m_sphere_world;		// 球用のワールド行列

	static const int TEAPOT_NUM;
	std::unique_ptr<DirectX::Model> m_teapot;					// ティーポット
	std::vector<DirectX::SimpleMath::Matrix>  m_teapot_world;	// 球用のワールド行列
	std::vector<DirectX::SimpleMath::Matrix>  m_teapot_trans;	// 球用の移動行列
	std::vector<DirectX::SimpleMath::Vector3> m_teapot_vec;	// 球用の移動行列(Vector3版)

	std::unique_ptr<DirectX::Model> m_head;		// ロボあたま
	DirectX::SimpleMath::Vector3    m_head_pos;
	DirectX::SimpleMath::Matrix     m_head_world;
	DirectX::SimpleMath::Matrix     m_head_world_rotate;

	std::unique_ptr<DirectX::SpriteBatch>  m_sprite_batch;
	std::unique_ptr<DirectX::SpriteFont>   m_sprite_font;
	std::wstring                           m_wstr;
	
	std::unique_ptr<DirectX::Keyboard> m_keyboard;		// キーボード

	int m_time_frame;
	int m_auto_move_time_frame;

	// TODO:DebugCameraとFollowCameraをキーで切り替えられるようにするとGOOD
	std::unique_ptr<FollowCamera> m_camera;

};