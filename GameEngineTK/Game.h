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

/* TODO:�|�C���^�Ŏ��������ƂŃC���N���[�h�����炷 */
#include "DebugCamera.h"
#include "FollowCamera.h"
#include "Obj3D.h"

class Character;

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game
{
public:
    Game();
	~Game();

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
	std::unique_ptr<DirectX::CommonStates>     m_states;		// �ėp�X�e�[�g
	std::unique_ptr<DirectX::EffectFactory>      m_factory;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_inputLayout;

	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_proj;		// �ˉe�s��
	
	static const int GROUND_NUM;
	static const int GROUND_WIDTH_HEIGHT;
	Obj3D*           m_ground;			// �n��
	std::vector<DirectX::SimpleMath::Matrix>  m_ground_world;	// �n�ʗp�̃��[���h�s��

	Obj3D*        m_skydome;		// �V��

	std::unique_ptr<DirectX::SpriteBatch>  m_sprite_batch;
	std::unique_ptr<DirectX::SpriteFont>   m_sprite_font;
	std::wstring                           m_wstr;
	
	std::unique_ptr<DirectX::Keyboard> m_keyboard;		// �L�[�{�[�h

	// TODO:DebugCamera��FollowCamera���L�[�Ő؂�ւ�����悤�ɂ����GOOD
	FollowCamera* m_camera;

	static const int ENEMY_NUM;
	std::unique_ptr<Character> m_player;
	std::vector<std::unique_ptr<Character>> m_enemy;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ui;// UI
};