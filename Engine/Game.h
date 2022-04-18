//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "Shader.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include "RayTriangleIntersection.h"
#include "RenderTexture.h"
#include "Terrain.h"
#include "CameraMovement.h"
#include "Collision.h"
#include "MouseRay.h"
#include "Ocean.h"
#include "WaterShader.h"
#include "PostProcess.h"
#include "PostProcessEffects.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);
    void RenderTexturePass1();
    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize(int& width, int& height) const;

private:

    struct MatrixBufferType
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    void Update(DX::StepTimer const& timer);
    void Render();
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void SetupGUI();
    void CreateNormalMapEffect(ID3D11DeviceContext* context, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTexture);
    XMVECTOR MousePicking(SimpleMath::Vector3 terrainPos, float terrainScale, SimpleMath::Vector3 terrainOrientation);
    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    //input manager. 
    Input									m_input;
    InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

    // Scene Objects
    std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;
    std::unique_ptr<DirectX::NormalMapEffect>                               m_normalMapEffect;
    //lights
    Light																	m_Light;

    //Cameras
    Camera																	m_Camera01;

    //textures 
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        flatRock;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_groundTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_slopeRockTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_grassTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_snowTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_sandTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_rockNormalTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_waterTexture;

    //Shaders
    Shader																	m_BasicShaderPair;
    WaterShader																	m_OceanShader;
    //Scene. 
    Terrain																	m_Terrain;
    Ocean																	m_Ocean;
    ModelClass																m_BasicModel;
    ModelClass																m_BasicModel2;
    ModelClass																m_BasicModel3;

    //RenderTextures
    RenderTexture* m_FirstRenderPass;
    RECT																	m_fullscreenRect;
    RECT																	m_CameraViewRect;
    RECT		                                                            m_ScreenDimensions;


#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif


#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
    std::unique_ptr<BasicPostProcess>                                       m_postProcess;
    RayTriangleIntersection                                                 m_rayTriIntersect;
    MouseRay                                                                m_mouseRay;
    CameraMovement                                                          m_CameraMovement;
    PostProcessEffects                                                      m_postProcessProperties;
    Collision                                                               m_Collision;
    
    
    float                                                                   m_CameraSmoothMovement = 0.03;
    int                                                                     m_screenWidth;
    int                                                                     m_screenHeight;
    float                                                                   m_elapsedTime = 0;
    bool                                                                    m_playMode = true;
    float                                                                   xChange;
    float                                                                   yChange;
};