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

	//setup imgui.  its up here cos we need the window handle too
	//pulled from imgui directx11 example
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);		//tie to our window
	ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 1920;
	m_fullscreenRect.bottom = 1080;

	m_CameraViewRect.left = 0;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 1920;
	m_CameraViewRect.bottom = 1080;

	//setup light
	m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setPosition(2.0f, 10.0f, 1.0f);
	m_Light.setDirection(-1.0f, -1.0f, 0.0f);


    m_Position.SetPosition(5.f, 5.f, 0.f);
    m_Position.SetRotation(19.6834f, 222.013f, 0.0f);

    m_Camera01.setPosition(Vector3(0.f, 0.5f, -10.f));





	//setup camera
	// m_Camera01.setPosition(Vector3(0.0f, 1.5f, 4.0f));
	// m_Camera01.setRotation(Vector3(180.0f, 0.0f, 0.0f));	//orientation is -90 becuase zero will be looking up at the sky straight up.




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

    HandleMovementInput(10.f);

	if (m_gameInputCommands.generate)
	{
        m_Terrain.Flatten(device);
	}

    if (m_gameInputCommands.smooth)
    {
        m_Terrain.Smooth(device);
    }
	m_Camera01.Update();	//camera update.
    m_view = m_Camera01.getCameraMatrix();
    m_world = Matrix::Identity;


	m_Terrain.Update();		//terrain update.  doesnt do anything at the moment.

    m_Water.Frame();
    m_Skyplane.Frame();

    RenderRefractionToTexture();
    RenderReflectionToTexture();
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


    m_world = Matrix::Identity;

    context->RSSetState(m_states->CullNone());
    context->OMSetDepthStencilState(m_states->DepthNone(), 1);

    Matrix cameraTranslation = Matrix::CreateTranslation(m_Camera01.getPosition());
    //Matrix skyscale = Matrix::CreateScale(5.f);
    m_world = m_world  * cameraTranslation;
    m_SkyDomeShader.EnableShader(context);
    m_SkyDomeShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, m_SkyDome.GetApexColor(), m_SkyDome.GetCenterColor());
    m_SkyDome.Render(context);

    context->RSSetState(m_states->CullCounterClockwise());

    float blendFactor[4];
    // Turn on additive alpha blending to blend clouds with sky dome
    blendFactor[0] = 0.0f;
    blendFactor[1] = 0.0f;
    blendFactor[2] = 0.0f;
    blendFactor[3] = 0.0f;
    context->OMSetBlendState(m_states->Additive(), blendFactor, 0xffffffff);


    m_SkyplaneShader.EnableShader(context);
    m_SkyplaneShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, m_Skyplane.GetCloudTexture(),
        m_Skyplane.GetPerturbTexture(), m_Skyplane.GetTranslation(), m_Skyplane.GetScale(), m_Skyplane.GetBrightness());

    m_Skyplane.Render(context);

    context->OMSetBlendState(m_states->Opaque(), blendFactor, 0xffffffff);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 1);

    //TERRAIN//

    //prepare transform for floor object.
    m_world = Matrix::Identity; //set world back to identity
    //Matrix newPosition3 = Matrix::CreateTranslation(0.0f, -0.6f, 0.0f);
    Matrix newScale = Matrix::CreateScale(0.1);		//scale the terrain down a little.
    m_world = m_world * newScale; //*newPosition3;

    //setup and draw cube
    m_TerrainShader.EnableShader(context);
    m_TerrainShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTexture.Get(), m_slopeTexture.Get(), m_rockTexture.Get());
    m_Terrain.Render(context);



    //WATER//

    m_world = Matrix::Identity;

    Matrix waterTranslation = Matrix::CreateTranslation(6.4f, m_Water.GetWaterHeight(), 6.4f);

    m_world = m_world * waterTranslation;
    m_WaterShader.EnableShader(context);
    m_Camera01.RenderReflection(m_Water.GetWaterHeight());
    m_WaterShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Camera01.GetReflectionViewMatrix(),
        m_RefractionTexture->getShaderResourceView(), m_ReflectionTexture->getShaderResourceView(), m_Water.GetTexture(), m_Camera01.getPosition(), m_Water.GetNormalMapTiling(), m_Water.GetWaterTranslation(),
        m_Water.GetReflectRefractScale(), m_Water.GetRefractionTint(), &m_Light, m_Water.GetSpecularShininess());


    m_Water.Render(context);

    m_world = Matrix::Identity;

    //render our GUI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


    // Show the new frame.
    m_deviceResources->Present();
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

bool Game::HandleMovementInput(float frameTime)
{
    bool keyDown, result;
    float posX, posY, posZ, rotX, rotY, rotZ;


    // Set the frame time for calculating the updated position.
    m_Position.SetFrameTime(frameTime);

    // Handle the input.
    keyDown = m_gameInputCommands.left;
    m_Position.TurnLeft(keyDown);

    keyDown = m_gameInputCommands.right;
    m_Position.TurnRight(keyDown);

    keyDown = m_gameInputCommands.forward;
    m_Position.MoveForward(keyDown);

    keyDown = m_gameInputCommands.back;
    m_Position.MoveBackward(keyDown);

    keyDown = m_gameInputCommands.up;
    m_Position.MoveUpward(keyDown);

    keyDown = m_gameInputCommands.down;
    m_Position.MoveDownward(keyDown);

    keyDown = m_gameInputCommands.rotUp;
    m_Position.LookUpward(keyDown);

    keyDown = m_gameInputCommands.rotDown;
    m_Position.LookDownward(keyDown);

    // Get the view point position/rotation.
    m_Position.GetPosition(posX, posY, posZ);
    m_Position.GetRotation(rotX, rotY, rotZ);

    // Set the position of the camera.
    m_Camera01.setPosition(Vector3(posX, posY, posZ));
    m_Camera01.setRotation(Vector3(rotX, rotY, rotZ));
    return true;
}

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

	//setup our terrain
	m_Terrain.Initialize(device, 128, 128);
    m_Water.Initialize(device, L"waternormal.dds", 0.1f, 6.3f);
    //m_SkyDome->Initialize(device);
	//setup our test model
    m_SkyDome.Initialize(device);
    m_Skyplane.Initialize(device, L"cloud001.dds", L"perturb001.dds");
	m_BasicModel.InitializeSphere(device, 1);
	m_BasicModel2.InitializeModel(device,"drone.obj");
	m_BasicModel3.InitializeBox(device, 10.0f, 10.f, 3.0f);	//box includes dimensions
	m_BasicModel4.InitializeBox(device, 10.0f, 10.f, 3.0f);	//box includes dimensions

	//load and set up our Vertex and Pixel Shaders
	m_BasicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
	m_TerrainShader.InitStandard(device, L"terrain_vs.cso", L"terrain_ps.cso");
	m_SkyDomeShader.InitStandard(device, L"skydome_vs.cso", L"skydome_ps.cso");
    m_ReflectionShader.InitStandard(device, L"reflection_vs.cso", L"reflection_ps.cso");
    m_WaterShader.InitStandard(device, L"water_vs.cso", L"water_ps.cso");
    m_SkyplaneShader.InitStandard(device, L"skyplane_vs.cso", L"skyplane_ps.cso");
	//load Textures
	CreateDDSTextureFromFile(device, L"seafloor.dds",		nullptr,	m_texture1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"EvilDrone_Diff.dds", nullptr,	m_texture2.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"grass.dds", nullptr,	m_grassTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"slope.dds", nullptr,	m_slopeTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"rock.dds", nullptr,	m_rockTexture.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"waternormal.dds", nullptr,	m_waterTexture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"normal.dds", nullptr, m_normalTexture.ReleaseAndGetAddressOf());
	//Initialise Render to texture
	m_FirstRenderPass = new RenderTexture(device, 800, 600, 1, 2);	//for our rendering, We dont use the last two properties. but.  they cant be zero and they cant be the same.
    m_RefractionTexture = new RenderTexture(device, 1920, 1080, 1, 2);
    m_ReflectionTexture = new RenderTexture(device, 1920, 1080, 1, 2);
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
    //m_projection = Matrix::CreatePerspectiveFieldOfView(
    //    fovAngleY,
    //    aspectRatio,
    //    0.01f,
    //    100.0f
    //);
    m_projection = DirectX::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, 0.01f, 100.0f);
}

void Game::SetupGUI()
{

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Sin Wave Parameters");
    if (ImGui::Button("Generate Sin Wave"))
        m_Terrain.GenerateHeightMap(m_deviceResources->GetD3DDevice());
		ImGui::SliderFloat("Wave Amplitude",	m_Terrain.GetAmplitude(), 0.0f, 10.0f);
		ImGui::SliderFloat("Wavelength",		m_Terrain.GetWavelength(), 0.0f, 1.0f);
	ImGui::End();

    ImGui::Begin("Faulting");
    if (ImGui::Button("Generate Faulting"))
        m_Terrain.GenerateFaulting(m_deviceResources->GetD3DDevice());
    if (ImGui::Button("Flatten (spacebar)"))
        m_Terrain.Flatten(m_deviceResources->GetD3DDevice());
    ImGui::End();

    ImGui::Begin("Perlin Noise");
    if (ImGui::Button("Generate Perlin Noise"))
        m_Terrain.GeneratePerlinNoise(
            m_deviceResources->GetD3DDevice(),
            *m_Terrain.GetSeed(),
            *m_Terrain.GetNoiseSize(),
            *m_Terrain.GetPersistence(),
            *m_Terrain.GetOctaves());
    ImGui::SliderInt("Seed", m_Terrain.GetSeed(), 0, 10);
    ImGui::SliderFloat("Noise Size", m_Terrain.GetNoiseSize(), 0.0f, 1.0f);
    ImGui::SliderFloat("Persistence", m_Terrain.GetPersistence(), 0.0f, 1.0f);
    ImGui::SliderInt("Octaves", m_Terrain.GetOctaves(), 0, 8);

    ImGui::End();
}

void Game::RenderRefractionToTexture()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();

    Vector4 clipPlane;

    // Setup a clipping plane based on the height of the water to clip everything above it to create a refraction.
    clipPlane = Vector4(0.0f, -1.0f, 0.0f, m_Water.GetWaterHeight() + 0.1f);

    // Set the render target to be the refraction render to texture.
    m_RefractionTexture->setRenderTarget(context);

    // Clear the refraction render to texture.
    m_RefractionTexture->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

    // Generate the view matrix based on the camera's position.
    m_Camera01.Update();

    // Render the terrain using the reflection shader and the refraction clip plane to produce the refraction effect.
    m_ReflectionShader.EnableShader(context);
    m_ReflectionShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_grassTexture.Get(), m_grassTexture.Get(), clipPlane, 2.0f);
    m_Terrain.Render(context);

    context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
    context->RSSetViewports(1, &m_deviceResources->GetScreenViewport());

    return;
}

void Game::RenderReflectionToTexture()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTargetView = m_deviceResources->GetRenderTargetView();
    auto depthTargetView = m_deviceResources->GetDepthStencilView();
    Vector4 clipPlane;
    Matrix reflectionViewMatrix, worldMatrix, projectionMatrix;
    Vector3 cameraPosition;


    // Setup a clipping plane based on the height of the water to clip everything below it.
    clipPlane = Vector4(0.0f, 1.0f, 0.0f, -m_Water.GetWaterHeight());

    // Set the render target to be the reflection render to texture.
    m_ReflectionTexture->setRenderTarget(context);

    // Clear the reflection render to texture.
    m_ReflectionTexture->clearRenderTarget(context, 0.0f, 0.0f, 0.0f, 1.0f);

    // Use the camera to render the reflection and create a reflection view matrix.
    m_Camera01.RenderReflection(m_Water.GetWaterHeight());

    // Get the camera reflection view matrix instead of the normal view matrix.
    reflectionViewMatrix = m_Camera01.GetReflectionViewMatrix();

    // Get the position of the camera.
    cameraPosition = m_Camera01.getPosition();

    // Invert the Y coordinate of the camera around the water plane height for the reflected camera position.
    cameraPosition.y = -cameraPosition.y + (m_Water.GetWaterHeight() * 2.0f);

    worldMatrix = Matrix::Identity;
    // Translate the sky dome and sky plane to be centered around the reflected camera position.
    Matrix translation = Matrix::CreateTranslation(cameraPosition);

    worldMatrix = worldMatrix * translation;
    // Turn off back face culling and the Z buffer.
    context->RSSetState(m_states->CullNone());
    context->OMSetDepthStencilState(m_states->DepthNone(), 1);


    // Render the sky dome using the reflection view matrix.
    m_SkyDomeShader.EnableShader(context);
    m_SkyDomeShader.SetShaderParameters(context, &worldMatrix, &reflectionViewMatrix, &m_projection, m_SkyDome.GetApexColor(), m_SkyDome.GetCenterColor());
    m_SkyDome.Render(context);


    // Enable back face culling.
    context->RSSetState(m_states->CullCounterClockwise());

    float blendFactor[4];
    // Turn on additive alpha blending to blend clouds with sky dome
    blendFactor[0] = 0.0f;
    blendFactor[1] = 0.0f;
    blendFactor[2] = 0.0f;
    blendFactor[3] = 0.0f;
    context->OMSetBlendState(m_states->Additive(), blendFactor, 0xffffffff);

    // Render the sky plane using the sky plane shader.
    m_SkyplaneShader.EnableShader(context);
    m_SkyplaneShader.SetShaderParameters(context, &worldMatrix, &reflectionViewMatrix, &m_projection, m_Skyplane.GetCloudTexture(),
        m_Skyplane.GetPerturbTexture(), m_Skyplane.GetTranslation(), m_Skyplane.GetScale(), m_Skyplane.GetBrightness());

        m_Skyplane.Render(context);

    // Turn off blending and enable the Z buffer again.
    //m_Direct3D->TurnOffAlphaBlending();
    context->OMSetBlendState(m_states->Opaque(), blendFactor, 0xffffffff);
    context->OMSetDepthStencilState(m_states->DepthDefault(), 1);

    // Reset the world matrix.
    //m_Direct3D->GetWorldMatrix(worldMatrix);
    worldMatrix = Matrix::Identity;
    Matrix newScale = Matrix::CreateScale(0.1);
    worldMatrix = worldMatrix * newScale;
    // Render the terrain using the reflection view matrix and reflection clip plane.
    m_ReflectionShader.EnableShader(context);
    m_ReflectionShader.SetShaderParameters(context, &worldMatrix, &reflectionViewMatrix, &m_projection, &m_Light, m_grassTexture.Get(), m_rockTexture.Get(), clipPlane, 2.0f);
    m_Terrain.Render(context);

    context->OMSetRenderTargets(1, &renderTargetView, depthTargetView);
    context->RSSetViewports(1, &m_deviceResources->GetScreenViewport());
    return;
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
