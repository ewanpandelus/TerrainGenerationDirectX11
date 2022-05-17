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
#include <Audio.h>
#include "RayTriangleIntersection.h"
#include "RenderTexture.h"
#include "Terrain.h"
#include "PlaneShader.h"
#include "TreeShader.h"
#include "PostProcess.h"
#include "SkyboxEffect.h"
#include "PlacedObjects.h"
#include "PostProcessEffects.h"
#include "CoinShader.h"


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
    XMVECTOR RayCastDirectionOfMouse(SimpleMath::Vector3 terrainPos, float terrainScale, SimpleMath::Vector3 terrainOrientation);
    SimpleMath::Vector3 PositionOnTerrain(SimpleMath::Vector3 rayCast, SimpleMath::Vector3 currentPosition);
    bool CompareVectorsApproxEqual(SimpleMath::Vector3 v1, SimpleMath::Vector3 v2, float threshold);
    bool HandlePlaneInput();
    void RayCasting(ID3D11Device* device, SimpleMath::Vector3 currentPos);
    void SetStartLerpParameters();
    void LerpPositionAndRotation(SimpleMath::Vector3 expectedPosition, SimpleMath::Vector3 expectedRotation, float t);
    void PopulatePlacedObjectArrays();

    void GenerateTerrain(ID3D11Device* device);
    void RenderPlacedObjects(ID3D11DeviceContext* context);
    void RenderCollectables(ID3D11DeviceContext* context);

    void SetupTerrainParamsGUI();
    void SetupManualTerrainModificationGUI();
    void SetupExtraParametersGUI();
    void CreateSkyBoxEffect(ID3D11DeviceContext* context, ID3D11Device* device);
    void SetupSoundEffects();
    void DrawText();
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
    float                                                                  m_diffuseLight[3];
    float                                                                  m_ambientLight[3];



    //Cameras
    Camera																	m_Camera01;

    //Textures 
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        flatRock;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_groundTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_slopeRockTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_grassTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_snowTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_sandTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_rockNormalTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_waterTexture;

    //Skybox
    std::unique_ptr<DX::SkyboxEffect>                                       m_skyboxEffect;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_skyInputLayout;
    //Shaders
    Shader																	m_BasicShaderPair;
    CoinShader															    m_CoinShader;
    PlaneShader																m_PlaneShader;
    TreeShader																m_TreeShader;
    //Scene. 
    Terrain																	m_Terrain;
    ModelClass																m_BasicModel;
    ModelClass																m_PlaneModel;
    ModelClass																m_CoinModel;
    ModelClass																m_TreeModel;
    ModelClass																m_TreeModel2;
    ModelClass																m_TreeModel3;
    ModelClass																m_BasicModel3;

    //RenderTextures
    RenderTexture* m_FirstRenderPass;
    RECT																	m_fullscreenRect;
    RECT																	m_CameraViewRect;
    RECT		                                                            m_ScreenDimensions;



    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_coinEffect;
    std::unique_ptr<DirectX::SoundEffect>                                   m_popEffect;
    std::unique_ptr<DirectX::SoundEffect>                                   m_winEffect;
    std::unique_ptr<DirectX::SoundEffect>                                   m_song;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;




    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;


    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
    std::unique_ptr<BasicPostProcess>                                       m_postProcess;
    RayTriangleIntersection                                                 m_rayTriIntersect;
    PlacedObjects                                                           m_placedObjects;
    PostProcessEffects                                                      m_postProcessProperties;
    std::unique_ptr<DirectX::GeometricPrimitive>                            m_sky;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_cubemap;


    float                                                                   m_terrainLerpVal = 2;
    float                                                                   m_elapsedTime = 1;
    float                                                                   m_guiTimer = 0;
    float                                                                   m_volume = 1;
    float                                                                   m_planeSpeed = 1;
    int                                                                     m_screenWidth;
    int                                                                     m_screenHeight;


    bool                                                                    m_lerpingPosition = false;
    bool                                                                    m_playMode = false;
    bool                                                                    m_editTerrain = false;
    bool                                                                    m_smoothTerrainTransition = false;
    bool                                                                    m_hideGUI = false;
    bool                                                                    m_hoveringUI = false;
    bool                                                                    m_generatedLastFrame = false;
    bool                                                                    m_soundEffect = true;
    bool                                                                    m_wonGame = false;

    SimpleMath::Vector3                                                     m_positionBeforeLerp;
    SimpleMath::Vector3                                                     m_rotationBeforeLerp;

    SimpleMath::Vector3                                                     m_planeRotation;
    SimpleMath::Vector3                                                     m_planeTransform;
    std::vector<SimpleMath::Vector3>                                        m_positionsOnTerrain;
    std::vector<PlacedObjects::CoinObjectType>                              m_coins;
    SimpleMath::Vector3                                                     m_placedObjectPosition;




};