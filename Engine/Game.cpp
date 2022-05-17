//
//
// Game.cpp
//

#include "pch.h"
#include "Game.h"
#include "PlacedObjects.h"

//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace ImGui;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
   
    
    m_input.Initialise(window);
    m_placedObjects.Initialise();
    m_deviceResources->SetWindow(window, width, height);
    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();
    
    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();
    CD3D11_TEXTURE2D_DESC sceneDesc(
        DXGI_FORMAT_R16G16B16A16_FLOAT, width, height,
        1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

    //setup imgui.  its up here cos we need the window handle too
    //pulled from imgui directx11 example
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(window);		//tie to our window
    ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx
    GetClientRect(window, &m_ScreenDimensions);
    m_fullscreenRect.left = 0;
    m_fullscreenRect.top = 0;
    m_fullscreenRect.right = 1920;
    m_fullscreenRect.bottom = 1080;

    m_CameraViewRect.left = 0;
    m_CameraViewRect.top = 0;
    m_CameraViewRect.right = 1920;
    m_CameraViewRect.bottom = 1080;
    m_diffuseLight[0] = 0.5f;
    m_diffuseLight[1] = 0.5f;
    m_diffuseLight[2] = 0.5f;

    m_ambientLight[0] = 0.4f;
    m_ambientLight[1] = 0.4f;
    m_ambientLight[2] = 0.4f;

    //setup light
    m_Light.setAmbientColour(m_ambientLight[0], m_ambientLight[1], m_ambientLight[2], 1.0f);
    m_Light.setDiffuseColour(m_diffuseLight[0], m_diffuseLight[1], m_diffuseLight[2], 1.0f);
    m_Light.setPosition(2.0f, 1.0f, 1.0f);
    m_Light.setDirection(-0.8f, -1.0f, 0.0f);

    //setup camera
    m_Camera01.setPosition(Vector3(-.5f * 10, (m_Terrain.GetCameraYPos() * 0.1f * 10) + 5 * 10, 6.5f * 10));
    m_Camera01.setRotation(Vector3(90.5f, 89.545f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 


    SetupSoundEffects();

    //m_effect2 = m_waveBank->CreateInstance(10);

   
   // m_effect2->Play();

}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    //take in input
    m_input.Update();								//update the hardware
    m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game

    //Update all game objects
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    //Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif


}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
  
    auto device = m_deviceResources->GetD3DDevice();
    m_audEngine->SetMasterVolume(m_volume);
    Vector3 currentPosition = m_Camera01.getPosition();
    Vector3 inFront = m_Camera01.getForward();
    m_Light.setAmbientColour(m_ambientLight[0], m_ambientLight[1], m_ambientLight[2], 1.0f);
    m_Light.setDiffuseColour(m_diffuseLight[0], m_diffuseLight[1], m_diffuseLight[2], 1.0f);
    m_elapsedTime += m_timer.GetElapsedSeconds() * (1 - m_elapsedTime) + 0.002f;
    m_guiTimer += m_timer.GetElapsedSeconds();
    if (!m_wonGame) {
        m_wonGame = m_placedObjects.DecreaseCoinScale();
        if (m_wonGame && m_soundEffect) { m_winEffect->Play(); }
    }

    if (m_terrainLerpVal < 1.0)
    {
        m_Terrain.LerpTerrainHeight(device, m_terrainLerpVal);
        m_terrainLerpVal += (0.06f * (1 - m_terrainLerpVal)) + 0.02f;
    }
    if(!m_hoveringUI||m_hideGUI) RayCasting(device, currentPosition);
    
    if (m_lerpingPosition)
    {
        if (m_playMode)
        {
            LerpPositionAndRotation(SimpleMath::Vector3(50, (m_Terrain.GetCameraYPos() + 5), 50),
                Vector3(90.0f, 89.5f, 0.0f), m_elapsedTime);
        }
        else
        {
            LerpPositionAndRotation(SimpleMath::Vector3(-.5f * 10, (m_Terrain.GetCameraYPos() * 0.1f * 10) + 5 * 10, 6.5f * 10),
                Vector3(90.5f, 89.545f, 0.0f), m_elapsedTime);
        }
    }
    if (m_gameInputCommands.p && m_elapsedTime > 0.5)
    {
        m_playMode = !m_playMode;
        SetStartLerpParameters();
    }

    if (m_playMode && !m_lerpingPosition)
    {
        HandlePlaneInput();
        m_planeTransform = m_Camera01.getPosition() + inFront * 3;
        m_Camera01.setPosition(currentPosition + (inFront * 0.2f*m_planeSpeed));
    }


    GenerateTerrain(device);

    if (m_gameInputCommands.h && m_guiTimer>0.2)
    {
        m_hideGUI = !m_hideGUI;
        m_guiTimer = 0;
    }
 
    m_view = m_Camera01.getCameraMatrix();
    m_world = Matrix::Identity;
 

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif


    if (m_input.Quit())
    {
        ExitGame();
    }
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();


 

    //Set Rendering states. 
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullNone());

    m_BasicShaderPair.EnableShader(context);
    SimpleMath::Matrix terrainPosition = SimpleMath::Matrix::CreateTranslation(0.0, -0.6, 0.0);
    m_world = m_world * terrainPosition;
    m_BasicShaderPair.SetShaderParametersTerrain(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTex.Get(), m_groundTex.Get(),
        m_slopeRockTex.Get(), m_snowTex.Get(), m_waterTexture.Get(), m_sandTex.Get(), m_timer.GetTotalSeconds(), m_Terrain, m_wonGame);
    m_Terrain.Render(context);
    

    if (m_playMode) RenderCollectables(context);
    m_TreeShader.EnableShader(context);
    RenderPlacedObjects(context);

    SimpleMath::Matrix scale = SimpleMath::Matrix::CreateScale(0.5);
    SimpleMath::Matrix currentObjectPosition = SimpleMath::Matrix::CreateTranslation(m_planeTransform);
    SimpleMath::Matrix planeRotation = SimpleMath::Matrix::CreateFromYawPitchRoll(m_Camera01.getRotation().y, m_Camera01.getRotation().x - 90, m_Camera01.getRotation().z);
    m_PlaneShader.EnableShader(context);
    m_world = scale * planeRotation * currentObjectPosition;
    m_PlaneShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), m_waterTexture.Get());
    m_PlaneModel.Render(context);

    RenderTexturePass1();
 
    m_skyboxEffect->SetView(m_view);
    m_sky->Draw(m_skyboxEffect.get(), m_skyInputLayout.Get());

    if (m_postProcessProperties.GetPostProcess()) {
        m_postProcess->SetSourceTexture(m_FirstRenderPass->getShaderResourceView());
        m_postProcess->SetBloomBlurParameters(1, m_postProcessProperties.GetBloomBlurStength(), m_postProcessProperties.GetBloomBrightness());
        m_postProcess->SetEffect(BasicPostProcess::BloomBlur);
        m_postProcess->Process(context);
    }
    else if (!m_hideGUI)
    {
        DrawText();
    }
    m_Camera01.Update(); //Late update so plane is not jittery

    // Show the new frame.
    m_deviceResources->Present();
}
void Game::RenderTexturePass1()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();
    m_FirstRenderPass->setRenderTarget(context);
    m_FirstRenderPass->clearRenderTarget(context, 0.0f, 0.0f, 1.0f, 1.0f);




    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullClockwise());



    m_TreeShader.EnableShader(context);
    RenderPlacedObjects(context);
    if (m_playMode) RenderCollectables(context);




    m_world = SimpleMath::Matrix::Identity;
    SimpleMath::Matrix scale = SimpleMath::Matrix::CreateScale(0.5);
    SimpleMath::Matrix currentObjectPosition = SimpleMath::Matrix::CreateTranslation(m_planeTransform);
    SimpleMath::Matrix planeRotation = SimpleMath::Matrix::CreateFromYawPitchRoll(m_Camera01.getRotation().y, m_Camera01.getRotation().x - 90, m_Camera01.getRotation().z);
    m_PlaneShader.EnableShader(context);
    m_world = scale * planeRotation * currentObjectPosition;
    m_PlaneShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), m_waterTexture.Get());
    m_PlaneModel.Render(context);



    m_world = SimpleMath::Matrix::Identity;
    m_BasicShaderPair.EnableShader(context);
    SimpleMath::Matrix terrainPosition = SimpleMath::Matrix::CreateTranslation(0.0, -0.6, 0.0);
    m_world = m_world * terrainPosition;
    m_BasicShaderPair.SetShaderParametersTerrain(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTex.Get(), m_groundTex.Get(),
        m_slopeRockTex.Get(), m_snowTex.Get(), m_waterTexture.Get(), m_sandTex.Get(), m_timer.GetTotalSeconds(), m_Terrain, m_wonGame);
    m_Terrain.Render(context);
 
    if (m_postProcessProperties.GetPostProcess() == true) {
        m_skyboxEffect->SetView(m_view);
        m_sky->Draw(m_skyboxEffect.get(), m_skyInputLayout.Get());
    }
    if (!m_hideGUI) 
    {
        SetupGUI();
        DrawText();
    }
 
  
    // Reset the render target back to the original back buffer and not the render to texture anymore.	
    context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;

}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
    m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);


    m_normalMapEffect = std::make_unique<NormalMapEffect>(device);
    //setup our terrain
    m_Terrain.Initialize(device, 128, 128);
    //setup our test model
    m_BasicModel.InitializeSphere(device);
    m_PlaneModel.InitializeModel(device, "plane.obj");
    m_CoinModel.InitializeModel(device, "coin.obj");
    m_TreeModel.InitializeModel(device, "tree.obj");
    m_TreeModel2.InitializeModel(device, "tree2.obj");
    m_TreeModel3.InitializeModel(device, "tree3.obj");

    m_BasicModel3.InitializeBox(device, 10.0f, 0.1f, 10.0f);	//box includes dimensions

    //load and set up our Vertex and Pixel Shaders
    m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_CoinShader.InitStandard(device, L"coin_vs.cso", L"coin_ps.cso");
    m_PlaneShader.InitStandard(device, L"plane_vs.cso", L"plane_ps.cso");
    m_TreeShader.InitStandard(device, L"tree_vs.cso", L"tree_ps.cso");

    //load Textures

    CreateDDSTextureFromFile(device, L"ground.dds", nullptr, m_groundTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"grass.dds", nullptr, m_grassTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"snow.dds", nullptr, m_snowTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"sand.dds", nullptr, m_sandTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"rock.dds", nullptr, m_slopeRockTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"water.dds", nullptr, m_waterTexture.ReleaseAndGetAddressOf());

    //Initialise 
    m_FirstRenderPass = new RenderTexture(device, 1920, 1080, 1, 2);
    m_postProcess = std::make_unique<BasicPostProcess>(device);
    CreateSkyBoxEffect(context, device);

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        160.0f
    );
    m_skyboxEffect->SetProjection(m_projection);
}
void Game::GenerateTerrain(ID3D11Device* device)
{

    if (m_gameInputCommands.generate)
    {
        m_placedObjects.ClearObjectPositions();
        if (m_smoothTerrainTransition)
        {
            m_Terrain.GenerateHeightMapLerped(device);
            m_terrainLerpVal = 0;
        }
        else { m_Terrain.GenerateHeightMap(device); }


        if (!m_playMode)
        {
            SetStartLerpParameters();

        }
        else
        {
            PopulatePlacedObjectArrays();
            m_wonGame = false;
        }


        if (!m_generatedLastFrame && m_soundEffect) {
            m_popEffect->Play();
        }
        m_generatedLastFrame = true;
    }
    else { m_generatedLastFrame = false; }
}
void Game::SetupGUI()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    m_hoveringUI = io.WantCaptureMouse;
    SetupTerrainParamsGUI();
    SetupManualTerrainModificationGUI();
    SetupExtraParametersGUI();
    if (ImGui::IsMouseReleased(0))
    {
        m_Terrain.TerrainTypeTicked();
    }

}
bool Game::HandlePlaneInput() {

    Vector3 currentPosition = m_Camera01.getPosition();
    Vector3 rotation = m_Camera01.getRotation();
    if (m_gameInputCommands.left && rotation.z > -1)
    {

        rotation.z = rotation.z -= m_Camera01.getRotationSpeed()/1.2;
    }
    if (m_gameInputCommands.right && rotation.z < 1)
    {
        rotation.z = rotation.z += m_Camera01.getRotationSpeed()/1.2;
    }
    if (m_gameInputCommands.forward && rotation.x < 91)
    {
        rotation.x = rotation.x += m_Camera01.getRotationSpeed() / 3;

    }
    if (m_gameInputCommands.back && rotation.x > 89)
    {
        rotation.x = rotation.x -= m_Camera01.getRotationSpeed() / 3;
    }
    rotation.y = rotation.y -= rotation.z * (m_Camera01.getRotationSpeed()*m_planeSpeed)/2.7;
    m_Camera01.setRotation(rotation);
    m_Camera01.setPosition(currentPosition);
    return true;
}

SimpleMath::Vector3 Game::PositionOnTerrain(SimpleMath::Vector3 rayCast, SimpleMath::Vector3 currentPosition) {
    for each (Triangle tri in m_Terrain.GetTriangleArray())
    {
        Vector3 rayDest = (currentPosition)+SimpleMath::Vector3(rayCast) * 1000;
        if (m_rayTriIntersect.Intersects(currentPosition, rayDest, tri.trianglePositions[0], tri.trianglePositions[1], tri.trianglePositions[2], 1))
        {
            return tri.trianglePositions[2];
        }


    }
    return Vector3(-10, -10, -10);
}
void Game::SetStartLerpParameters() 
{
    m_lerpingPosition = true;
    m_positionBeforeLerp = m_Camera01.getPosition();
    m_rotationBeforeLerp = m_Camera01.getRotation();
    m_elapsedTime = 0;
    if (m_playMode) {
        PopulatePlacedObjectArrays();
        m_wonGame = false;
    }
}

XMVECTOR Game::RayCastDirectionOfMouse(SimpleMath::Vector3 terrainPos, float terrainScale, SimpleMath::Vector3 terrainOrientation)
{
    const XMVECTOR nearSource = XMVectorSet(m_input.GetMouseX(), m_input.GetMouseY(), 0.0f, 1.0f);
    const XMVECTOR farSource = XMVectorSet(m_input.GetMouseX(), m_input.GetMouseY(), 1.0f, 1.0f);
    const XMVECTORF32 scale = { terrainScale,		terrainScale,		terrainScale };
    const XMVECTORF32 translate = { terrainPos.x,		terrainPos.y,	terrainPos.z };
    XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(terrainOrientation.y * 3.1415 / 180, terrainOrientation.x * 3.1415 / 180,
        terrainOrientation.z * 3.1415 / 180);
    XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);
    XMVECTOR  nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, 1920, 1080, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
    XMVECTOR  farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, 1920, 1080, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
    return farPoint - nearPoint;
}
bool Game::CompareVectorsApproxEqual(SimpleMath::Vector3 v1, SimpleMath::Vector3 v2, float threshold) {
    bool equal = true;
    equal = equal && (abs(v1.x - v2.x) < threshold);
    equal = equal && (abs(v1.y - v2.y) < threshold);
    return equal && (abs(v1.z - v2.z) < threshold);
}
void Game::RenderPlacedObjects(ID3D11DeviceContext* context) {
    std::vector<PlacedObjects::PlacedObjectType> placedObjects = m_placedObjects.GetObjectPositions();
    SimpleMath::Matrix currentObjPosition = SimpleMath::Matrix::CreateTranslation(0, 0, 0);
    for (int i = 0;i < placedObjects.size();i++) {
        m_world = SimpleMath::Matrix::Identity; //set world back to identity
        currentObjPosition = SimpleMath::Matrix::CreateTranslation(placedObjects[i].position);
        m_world = m_world  * currentObjPosition;
    
        if(placedObjects[i].type ==0)
        {
            float* treeColour = m_placedObjects.GetTreeColour();
            m_TreeShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, DirectX::SimpleMath::Vector3(treeColour[0]
                , treeColour[1], treeColour[2]), m_waterTexture.Get());
            m_TreeModel.Render(context);
        }
        else if (placedObjects[i].type ==1) {
            float* treeColour = m_placedObjects.GetTree1Colour();
            m_TreeShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, DirectX::SimpleMath::Vector3(treeColour[0]
                , treeColour[1], treeColour[2]), m_waterTexture.Get());
            m_TreeModel2.Render(context);
        }
        else  {
            float* treeColour = m_placedObjects.GetTree2Colour();
            m_TreeShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, DirectX::SimpleMath::Vector3(treeColour[0]
                , treeColour[1], treeColour[2]), m_waterTexture.Get());
            m_TreeModel3.Render(context);
        }
      
    }

}
void Game::RenderCollectables(ID3D11DeviceContext* context) {
    m_CoinShader.EnableShader(context);
    m_coins = m_placedObjects.GetCoins();
    SimpleMath::Matrix coinLocalRotation = SimpleMath::Matrix::CreateRotationY((m_timer.GetTotalSeconds() / 1.5));
    SimpleMath::Matrix coinScale = SimpleMath::Matrix::CreateScale(0.4);
    SimpleMath::Matrix currentObjectPosition = SimpleMath::Matrix::CreateTranslation(0, 0, 0);
    for (int i = 0; i < m_coins.size(); i++) {
        m_world = SimpleMath::Matrix::Identity; 
        if (CompareVectorsApproxEqual(m_planeTransform, m_coins[i].position, 1) && m_coins[i].collected== false) {
            m_placedObjects.AssignCollected(i);
            if(m_soundEffect) m_coinEffect->Play();
        }
        coinScale = SimpleMath::Matrix::CreateScale(m_coins[i].scale);
        currentObjectPosition = SimpleMath::Matrix::CreateTranslation(m_coins[i].position);
        m_world = m_world * coinScale * coinLocalRotation * currentObjectPosition;
        m_CoinShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), m_waterTexture.Get());
        m_CoinModel.Render(context);
    }
}
void Game::PopulatePlacedObjectArrays()
{
    m_placedObjects.ClearCoinPositions();
 
    m_placedObjects.AddToCoinPositions(m_Terrain.GetCameraYPos());

}
void Game::RayCasting(ID3D11Device* device, SimpleMath::Vector3 currentPos) {
    if ((m_gameInputCommands.leftMouse || m_gameInputCommands.rightMouse) &&
        (m_editTerrain || (*m_placedObjects.GetPlaceTrees()==true))) {

        SimpleMath::Vector3 rayCast = RayCastDirectionOfMouse(SimpleMath::Vector3(0.0f, -0.6f, 0.0f), 1, SimpleMath::Vector3(0.0f, 0.0f, 0.0f));
        rayCast.Normalize();
        Vector3 positionOnTerrain = PositionOnTerrain(rayCast, currentPos);
        if (m_editTerrain)
        {
            int editTerrainDirection = m_gameInputCommands.leftMouse ? 1 : -1;
            if (positionOnTerrain.x != -10) m_Terrain.ManipulateTerrain(positionOnTerrain.x, positionOnTerrain.z, device, editTerrainDirection);
        }
        if (*m_placedObjects.GetPlaceTrees()==true) {
            m_placedObjects.AddToObjectPositions(positionOnTerrain + Vector3(0, 0.23, 0));
        }

    }
}
void Game::LerpPositionAndRotation(SimpleMath::Vector3 expectedPosition, SimpleMath::Vector3 expectedRotation, float t)
{
    SimpleMath::Vector3 difference = expectedPosition - m_positionBeforeLerp;
    SimpleMath::Vector3 differenceInRotation = expectedRotation - m_rotationBeforeLerp;
    m_Camera01.setPosition(m_positionBeforeLerp + (difference * t));
    m_Camera01.setRotation(m_rotationBeforeLerp + (differenceInRotation * t));
    if (t >= 1) {
        m_lerpingPosition = false;
    }
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
    m_batch.reset();
    m_testmodel.reset();
    m_batchInputLayout.Reset();
    m_sky.reset();
    m_skyboxEffect.reset();
    m_skyInputLayout.Reset();
}


void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
void Game::SetupTerrainParamsGUI()
{
    ImGui::SetNextWindowSize(ImVec2(450, 320));
    ImGui::Begin("Procedural Terrain Parameters");
    ImGui::SliderFloat("Amplitude", m_Terrain.GetAmplitude(), 0.0f, 10.0f);
    ImGui::SliderFloat("Frequency", m_Terrain.GetFrequency(), 0.0f, 2.0f);
    ImGui::SliderFloat("Lacunarity", m_Terrain.GetLacunarity(), 0.0f, 1.0f);
    ImGui::SliderFloat("Persistance", m_Terrain.GetPersistance(), 0.0f, 2.5f);
    ImGui::SliderInt("Octaves", m_Terrain.GetOctaves(), 1.0f, 10.0f);
    ImGui::SliderFloat("Terrain Y Position", m_Terrain.SetTerrainHeightPosition(), -10.0f, 10.0f);
    ImGui::InputInt("Seed", m_Terrain.SetSeed(), 0, 10000);
    ImGui::Checkbox("Use Worley Noise Heightmap", m_Terrain.GetWorleyNoise());
    ImGui::Checkbox("Use Ridge Noise Heightmap", m_Terrain.GetRidgeNoise());
    ImGui::Checkbox("Use Perlin Noise Heightmap", m_Terrain.GetFBMNoise());
    if (*m_Terrain.GetFBMNoise() == true) {
        ImGui::Checkbox("Terrace Perlin Heightmap", m_Terrain.GetTerraced());
        ImGui::InputInt("Terrace Level", m_Terrain.SetTerraceVal(), 1, 30);
    }
    else {
        *m_Terrain.GetTerraced() = false;
    }
    ImGui::Checkbox("Invert Terrain", m_Terrain.GetInverseHeightMap());
    ImGui::SliderFloat("Flatten Terrain Percentage", m_Terrain.SetFlattenPercentage(),0,100);
    ImGui::Checkbox("Smooth Transition", &m_smoothTerrainTransition);
    ImGui::End();
}
void Game::SetupManualTerrainModificationGUI()
{
  
    ImGui::Begin("Manually Modify Terrain");
    ImGui::Checkbox("Modify Terrain with Mouse", &m_editTerrain);
    int guiX = 260;
    int guiY = 120;
    ImGui::Checkbox("Choose Terrain Colours", m_Terrain.SetColourTerrain());
    if (m_Terrain.GetColourTerrain()) {
        guiX += 140;
        guiY += 100;
        ImGui::Checkbox("Overwrite Terrain Texture Colours", m_Terrain.GetOverwritesColour());
        ImGui::ColorEdit3("Water", m_Terrain.SetWaterColour());
        ImGui::ColorEdit3("Sand", m_Terrain.SetSandColour());
        ImGui::ColorEdit3("Grass", m_Terrain.SetGrassColour());
        ImGui::ColorEdit3("Mellow Rock", m_Terrain.SetMellowSlopeColour());
        ImGui::ColorEdit3("Steep Rock", m_Terrain.SetSteepSlopeColour());
        ImGui::ColorEdit3("Snow", m_Terrain.SetSnowColour());
    }
    ImGui::Checkbox("Place Trees", m_placedObjects.GetPlaceTrees());
    if (*m_placedObjects.GetPlaceTrees() == true)
    {
        guiX = 550;
        guiY += 100;
        ImGui::SliderInt("Tree Variation", m_placedObjects.SetCurrentTreeSelected(), 0, 2);
        ImGui::ColorEdit3("Tree Variation 1 Colour", m_placedObjects.GetTreeColour());
        ImGui::ColorEdit3("Tree Variation 2 Colour", m_placedObjects.GetTree1Colour());
        ImGui::ColorEdit3("Tree Variation 3 Colour", m_placedObjects.GetTree2Colour());
    }
    ImGui::SetWindowSize("Manually Modify Terrain", ImVec2(guiX, guiY));
    ImGui::End();
}
void Game::SetupExtraParametersGUI()
{
    ImGui::SetNextWindowSize(ImVec2(500, 210));
    ImGui::Begin("Extra Parameters");
    ImGui::Checkbox("Post Process On", m_postProcessProperties.SetPostProcessImGUI());
    ImGui::SliderFloat("Bloom Brightness", m_postProcessProperties.SetBloomBrightness(), 0.1f, 3.0f);
    ImGui::SliderFloat("Bloom Blur Strength", m_postProcessProperties.SetBloomBlurRadius(), 0.1f, 3.0f);
    ImGui::SliderFloat("Sound Volume",&m_volume, 0, 1);
    ImGui::Checkbox("Sound Effects", &m_soundEffect);
    ImGui::ColorEdit3("Diffuse Light Colour", m_diffuseLight);
    ImGui::ColorEdit3("Ambient Light Colour", m_ambientLight);
    ImGui::SliderFloat("Plane Speed", &m_planeSpeed, 0.01f, 1.5f);
    ImGui::End();
}
void Game::DrawText() {
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    m_sprites->Begin();
    m_font->DrawString(m_sprites.get(), L"Press SPACE to generate terrain", XMFLOAT2(10, 10), Colors::OrangeRed);
    m_font->DrawString(m_sprites.get(), L"Press P to alternate view & play mode", XMFLOAT2(10, 40), Colors::OrangeRed);
    m_font->DrawString(m_sprites.get(), L"Press H to hide GUI", XMFLOAT2(10, 70), Colors::OrangeRed);
    m_sprites->End();
}
void Game::SetupSoundEffects()
{
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_coinEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"coin.wav");
    m_popEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"pop.wav");
    m_song = std::make_unique<SoundEffect>(m_audEngine.get(), L"song.wav");
    m_winEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"success.wav");
   
    m_effect1 = m_song->CreateInstance();
    m_effect1->Play(true);
    m_audEngine->SetMasterVolume(0.2f);
}
void Game::CreateSkyBoxEffect(ID3D11DeviceContext* context, ID3D11Device* device) {
    m_sky = GeometricPrimitive::CreateGeoSphere(context, 2.f, 3, false /*invert for being inside the shape*/);

    m_skyboxEffect = std::make_unique<DX::SkyboxEffect>(device);
    m_sky->CreateInputLayout(m_skyboxEffect.get(),
        m_skyInputLayout.ReleaseAndGetAddressOf());
    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"sunsetcube1024.dds",
            nullptr, m_cubemap.ReleaseAndGetAddressOf()));

    m_skyboxEffect->SetTexture(m_cubemap.Get());
}
#pragma endregion

