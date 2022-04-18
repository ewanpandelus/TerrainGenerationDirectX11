//
//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


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
    m_fullscreenRect.right = 800;
    m_fullscreenRect.bottom = 600;

    m_CameraViewRect.left = 500;
    m_CameraViewRect.top = 0;
    m_CameraViewRect.right = 800;
    m_CameraViewRect.bottom = 240;

    //setup light
    m_Light.setAmbientColour(0.4f, 0.4f, 0.6f, 1.0f);
    m_Light.setDiffuseColour(0.6f, 0.6f, 0.6f, 1.0f);
    m_Light.setPosition(2.0f, 1.0f, 1.0f);
    m_Light.setDirection(-0.8f, -1.0f, 0.0f);

    //setup camera
    m_Camera01.setPosition(Vector3(0.0, 0.0f, 5.0f));
    m_Camera01.setRotation(Vector3(90.0f, 89.5f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up. 


#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
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
    //this is hacky,  i dont like this here.  
    auto device = m_deviceResources->GetD3DDevice();
     m_elapsedTime += timer.GetElapsedSeconds();
    XMVECTOR rayCast;
    if (m_gameInputCommands.leftMouse) {
        rayCast = MousePicking(SimpleMath::Vector3(0.0f, -0.6f, 0.0f),0.1, SimpleMath::Vector3(0.0f, 0.0f, 0.0f));
    }

    if (m_gameInputCommands.p&&m_elapsedTime>1) 
    { 
      m_playMode = !m_playMode; 
      m_elapsedTime = 0;
    }


    if (m_playMode)
    {
        float verticalDifference = 0.0001;
        bool moved = false;
        //note that currently.  Delta-time is not considered in the game object movement. 
        Vector3 currentPosition = m_Camera01.getPosition();
        Vector3 inFrontCurrentPos = m_Camera01.getPosition() + m_Camera01.getForward() * 0.01f;
        Box* box = m_Terrain.GetBoxAtPosition(inFrontCurrentPos.x * 10, inFrontCurrentPos.z * 10);
        DirectX::SimpleMath::Vector3 pos1;
        if (box) {
            pos1 = m_CameraMovement.UpdateCameraMovement(m_Camera01, m_rayTriIntersect, box);
            verticalDifference = currentPosition.y - pos1.y;
            pos1.y = currentPosition.y - (verticalDifference * m_CameraSmoothMovement);
        }



        if (m_gameInputCommands.left)
        {
            Vector3 rotation = m_Camera01.getRotation();
            rotation.y = rotation.y += m_Camera01.getRotationSpeed();
            m_Camera01.setRotation(rotation);
        }
        if (m_gameInputCommands.right)
        {
            Vector3 rotation = m_Camera01.getRotation();
            rotation.y = rotation.y -= m_Camera01.getRotationSpeed();
            m_Camera01.setRotation(rotation);
        }
        if (m_gameInputCommands.forward)
        {
            currentPosition = m_Camera01.getPosition(); //get the position
            currentPosition += (m_Camera01.getForward() * m_Camera01.getMoveSpeed());//*(1 / (1 - abs(verticalDifference))) / 3;
            moved = true;

        }
        if (m_gameInputCommands.back)
        {
            currentPosition = m_Camera01.getPosition(); //get the position
            currentPosition -= (m_Camera01.getForward() * m_Camera01.getMoveSpeed());// *(1 / (1 - abs(verticalDifference))) / 3;
            moved = true;
        }
        if (moved) {
            currentPosition.y = pos1.y;
        }
        m_Camera01.setPosition(currentPosition);


      
    }
    else 
    {
        Vector3 currentPos = m_Camera01.getPosition();
        if (currentPos != SimpleMath::Vector3(-2.5f, m_Terrain.GetCameraYPos(), 5.0f)) {

            SimpleMath::Vector3 difference = SimpleMath::Vector3(-2.0f, (m_Terrain.GetCameraYPos()*0.1f)+4, 5.0f) - currentPos;
            m_Camera01.setPosition(m_Camera01.getPosition() + difference * 0.03f);
            // m_Camera01.setPosition(Vector3(-2.5f, 4.0f, 5.0f));
            m_Camera01.setRotation(Vector3(90.0f, 89.5f, 0.0f));
        }

    }
    if (m_gameInputCommands.generate)
    {
        m_Terrain.GenerateHeightMap(device);
    }

   

    m_Terrain.Update();		//terrain update.  doesnt do anything at the moment. 


    //m_Camera01.setPosition(position);
    m_Camera01.Update();

    m_postProcess = std::make_unique<BasicPostProcess>(device);
    m_view = m_Camera01.getCameraMatrix();
    m_world = Matrix::Identity;

    /*create our UI*/
    SetupGUI();

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

    // Draw Text to the screen
    m_sprites->Begin();
    m_font->DrawString(m_sprites.get(), L"Procedural Methods", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();

    //Set Rendering states. 
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullClockwise());
    //context->RSSetState(m_states->Wireframe());
    RenderTexturePass1();

   
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix newPosition3 = SimpleMath::Matrix::CreateTranslation(0.0f, -0.6f, 0.0f);




    SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.1);		//scale the terrain down a little. 
    m_world = m_world * newScale * newPosition3;
    m_BasicShaderPair.EnableShader(context);
    //m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, flatRock.Get());
    // m_BasicModel3.Render(context);
        //setup and draw cube
    m_BasicShaderPair.EnableShader(context);
    //m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());
    m_BasicShaderPair.SetShaderParametersTerrain(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTex.Get(), m_groundTex.Get(),
        m_slopeRockTex.Get(), m_snowTex.Get(), m_waterTexture.Get(), m_sandTex.Get(), m_timer.GetTotalSeconds(), m_Terrain);

    m_Terrain.Render(context);
    m_OceanShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), m_waterTexture.Get());
    m_OceanShader.EnableShader(context);
   // m_Ocean.Render(context);
    //render our GUI

    
    //prepare transform for floor object. 

    if (m_postProcessProperties.GetPostProcess()) {
        m_postProcess->SetSourceTexture(m_FirstRenderPass->getShaderResourceView());
        m_postProcess->SetBloomBlurParameters(1,m_postProcessProperties.GetBloomBlurStength(),m_postProcessProperties.GetBloomBrightness());
        m_postProcess->SetEffect(BasicPostProcess::BloomBlur);
        m_postProcess->Process(context);
    }
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    

    /*m_sprites->Begin();
    m_sprites->Draw(m_FirstRenderPass->getShaderResourceView(), m_fullscreenRect);
    m_sprites->End();*/

   

    // Set RSSetViewports to the rectangle you want to fill with the texture.

    // If the viewport is not the full render target, be sure to call RSSetScissors as well.



    // Show the new frame.
    m_deviceResources->Present();
}
void Game::RenderTexturePass1()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();
    // Set the render target to be the render to texture.
    m_FirstRenderPass->setRenderTarget(context);
    // Clear the render to texture.
    m_FirstRenderPass->clearRenderTarget(context, 0.0f, 0.0f, 1.0f, 1.0f);

    m_sprites->Begin();
    m_font->DrawString(m_sprites.get(), L"Procedural Methods", XMFLOAT2(10, 10), Colors::Yellow);
    m_sprites->End();

    //Set Rendering states. 
    context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
    context->RSSetState(m_states->CullClockwise());
    //context->RSSetState(m_states->Wireframe());

    //prepare transform for floor object. 
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix newPosition3 = SimpleMath::Matrix::CreateTranslation(0.0f, -0.6f, 0.0f);




    SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(0.1);		//scale the terrain down a little. 
    m_world = m_world * newScale * newPosition3;
    m_BasicShaderPair.EnableShader(context);
    //m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());
    m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, flatRock.Get());
    // m_BasicModel3.Render(context);
     //setup and draw cube
    m_BasicShaderPair.EnableShader(context);
    //m_BasicShaderPair.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_texture1.Get());
    m_BasicShaderPair.SetShaderParametersTerrain(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTex.Get(), m_groundTex.Get(),
        m_slopeRockTex.Get(), m_snowTex.Get(), m_waterTexture.Get(), m_sandTex.Get(), m_timer.GetTotalSeconds(), m_Terrain);
    m_Terrain.Render(context);
    m_OceanShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(), m_waterTexture.Get());
    m_OceanShader.EnableShader(context);
    //m_Ocean.Render(context);
    //render our GUI
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


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
    m_Ocean.Initialize(device, 144, 144);
    //setup our test model
    m_BasicModel.InitializeSphere(device);
    m_BasicModel2.InitializeModel(device, "drone.obj");
    m_BasicModel3.InitializeBox(device, 10.0f, 0.1f, 10.0f);	//box includes dimensions

    //load and set up our Vertex and Pixel Shaders
    m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_OceanShader.InitStandard(device, L"ocean_vs.cso", L"ocean_ps.cso");

    //load Textures

    CreateDDSTextureFromFile(device, L"ground.dds", nullptr, m_groundTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"grass.dds", nullptr, m_grassTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"snow.dds", nullptr, m_snowTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"sand.dds", nullptr, m_sandTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"rock.dds", nullptr, m_slopeRockTex.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"water2.dds", nullptr, m_waterTexture.ReleaseAndGetAddressOf());

    //Initialise 
    m_FirstRenderPass = new RenderTexture(device, 1920, 1000, 1, 2);
    
    
    
    
    
    

  

}
void Game::CreateNormalMapEffect(ID3D11DeviceContext* context, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTexture) {
    auto time = static_cast<float>(m_timer.GetTotalSeconds());
    SimpleMath::Matrix zRotation = Matrix::CreateRotationX(time);
    m_normalMapEffect->SetWorld(zRotation * SimpleMath::Matrix::CreateTranslation(5.0f, 0.5f, 2.0f));

    m_normalMapEffect->SetView(m_view);
    m_normalMapEffect->SetProjection(m_projection);
    m_normalMapEffect->SetNormalTexture(normalTexture.Get());
    m_normalMapEffect->SetAlpha(0.5f);
    m_normalMapEffect->SetAmbientLightColor(Colors::Crimson);
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
        100.0f
    );
}

void Game::SetupGUI()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Terrain Parameters");
    ImGui::SliderFloat("Amplitude", m_Terrain.GetAmplitude(), 1.0f, 10.0f);
    ImGui::SliderFloat("Frequency", m_Terrain.GetFrequency(), 0.0f, 2.0f);
    ImGui::SliderFloat("Lacunarity", m_Terrain.GetLacunarity(), 0.0f, 1.0f);
    ImGui::SliderFloat("Persistance", m_Terrain.GetPersistance(), 0.0f, 2.5f);
    ImGui::SliderFloat("Octaves", m_Terrain.GetOctaves(), 1.0f, 10.0f);
    ImGui::SliderFloat("Terrain Y Position", m_Terrain.SetTerrainHeightPosition(), -2.0f, 10.0f);
    ImGui::Checkbox("Use Worley Noise Heightmap", m_Terrain.GetWorleyNoise());
    ImGui::Checkbox("Use Ridge Noise Heightmap", m_Terrain.GetRidgeNoise());
    ImGui::Checkbox("Use Perlin Noise Heightmap", m_Terrain.GetFBMNoise());
    ImGui::Checkbox("Choose Terrain Colours", m_Terrain.SetColourTerrain());

    if (ImGui::IsMouseReleased(0)) 
    {
        m_Terrain.TerrainTypeTicked();
    }
    if (m_Terrain.GetColourTerrain()) {
        ImGui::ColorEdit3("Bottom Terrain Colour", m_Terrain.SetBottomTerrainColorImGUI());
        ImGui::ColorEdit3("Slope Terrain Colour", m_Terrain.SetSecondTerrainColourImGUI());
        ImGui::ColorEdit3("Sand Colour", m_Terrain.SetThirdTerrainColorImGUI());
        ImGui::ColorEdit3("Snow Colour", m_Terrain.SetTopTerrainColorImGUI());
    }
    ImGui::End();

    ImGui::Begin("Post Process Parameters");
    ImGui::Checkbox("PostProcessOn", m_postProcessProperties.SetPostProcessImGUI());
    ImGui::SliderFloat("Bloom Brightness", m_postProcessProperties.SetBloomBrightness(), 0.1f, 3.0f);
    ImGui::SliderFloat("Bloom Blur Strength", m_postProcessProperties.SetBloomBlurRadius(), 0.1f, 3.0f);


    ImGui::End();
   
}
 XMVECTOR Game::MousePicking(SimpleMath::Vector3 terrainPos,  float terrainScale, SimpleMath::Vector3 terrainOrientation)
{
 
    
        const XMVECTOR nearSource = XMVectorSet(m_input.GetMouseX(), m_input.GetMouseY(), 0.0f, 1.0f);
        const XMVECTOR farSource = XMVectorSet(m_input.GetMouseX(), m_input.GetMouseY(), 1.0f, 1.0f);

        const XMVECTORF32 scale = {terrainScale,		terrainScale,		terrainScale };
        const XMVECTORF32 translate = { terrainPos.x,		terrainPos.y,	terrainPos.z };
        XMVECTOR rotate = Quaternion::CreateFromYawPitchRoll(terrainOrientation.y * 3.1415 / 180, terrainOrientation.x * 3.1415 / 180,
            terrainOrientation.z * 3.1415 / 180);


        XMMATRIX local = m_world * XMMatrixTransformation(g_XMZero, Quaternion::Identity, scale, g_XMZero, rotate, translate);
        DirectX::SimpleMath::Vector3 nearPoint = XMVector3Unproject(nearSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);
        DirectX::SimpleMath::Vector3 farPoint = XMVector3Unproject(farSource, 0.0f, 0.0f, m_ScreenDimensions.right, m_ScreenDimensions.bottom, m_deviceResources->GetScreenViewport().MinDepth, m_deviceResources->GetScreenViewport().MaxDepth, m_projection, m_view, local);


        return farPoint - nearPoint;
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
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion