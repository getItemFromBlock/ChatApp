#include "Core/App.hpp"

#include <iostream>
#include <ImGUI/imgui.h>
#include <ImGUI-FileBrowser/imfilebrowser.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_opengl3.h>
#include <ImGUI/imgui_stdlib.hpp>

#include "Resources/Texture.hpp"
#include "Core/Log.hpp"

#include <chrono>

using namespace Core;

Core::App::App()
{
}

Core::App::~App()
{
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	if (windowIcon)
	{
		if (windowIcon->pixels)	delete[] windowIcon->pixels;
		delete windowIcon;
	}
}

int App::Init()
{
	if (!network.isValid) return -1;
	glfwInit();
	/*
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	*/

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(1200, 900, "Chat Application", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	windowIcon = Resources::Texture::ReadIcon("Resources/Icon_48.png");
	if (windowIcon) glfwSetWindowIcon(window, 1, windowIcon);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	GLint flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Core::Log::glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	int c;
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &c);
	Resources::Texture::SetUnitLimit(c);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	textures = std::make_unique<Resources::TextureManager>();
	users = std::make_unique<Chat::UserManager>(*textures.get());

	selfID = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	selfUser = users->GetOrCreateUser(selfID);
	selfUser->userName = "User";

	std::filesystem::path p = std::filesystem::current_path().append("Saved");
	if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p))
	{
		std::filesystem::create_directory(p);
	}

	fileDialog = std::make_unique<ImGui::FileBrowser>();
	fileDialog->SetTitle("Image Selection");
	fileDialog->SetTypeFilters({ ".png", ".jpg", ".jpeg", ".webp" });

	return 0;
}

void App::Run()
{
	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::BeginMainMenuBar();
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				glfwSetWindowShouldClose(window, true);
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Options"))
		{
			ImGui::MenuItem("User Profile", nullptr, &userSettings);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (userSettings)
		{
			ImGui::Begin("User Settings", &userSettings, ImGuiWindowFlags_NoCollapse);
			ImGui::InputText("User Name", &selfUser->userName);
			ImGui::Image((ImTextureID)selfUser->userTex->GetTextureID(), ImVec2(100, 100));
			if (ImGui::IsItemClicked())
			{
				fileDialog->Open();
			}
			ImGui::ColorEdit3("Color", &selfUser->userColor.x);
			ImGui::End();
		}

		fileDialog->Display();

		if (fileDialog->HasSelected())
		{
			std::filesystem::path texPath = fileDialog->GetSelected();
			//std::filesystem::path dest = std::filesystem::path(cachePath).append(texPath.filename().string());
			//std::filesystem::copy_file(texPath, dest);
			std::string path = texPath.string();
			Resources::Texture* tex = textures->GetOrCreateTexture(path);
			lastError = Resources::Texture::TryLoad(path.c_str(), tex, Maths::Vec2(16, 16), Maths::Vec2(256, 256), 0x40000);
			if (lastError == TextureError::NONE)
			{
				selfUser->userTex = tex;
			}
			else
			{
				ImGui::OpenPopup("Texture Error");
			}
			fileDialog->ClearSelected();
		}

		if (ImGui::BeginPopupModal("Texture Error", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::TextUnformatted("Error reading file:");
			switch (lastError)
			{
			case TextureError::NONE:
				ImGui::CloseCurrentPopup();
				break;
			case TextureError::NO_FILE:
				ImGui::TextUnformatted("Image Not Found!");
				break;
			case TextureError::FILE_TOO_BIG:
				ImGui::TextUnformatted("Image file is too big!\nMaximum is 262144 (256k) bytes");
				break;
			case TextureError::IMG_TOO_SMALL:
				ImGui::TextUnformatted("Image resolution is too small!\nMinimum is 16/16 pixels");
				break;
			case TextureError::IMG_TOO_BIG:
				ImGui::TextUnformatted("Image resolution is too big!\nMaximum is 256/256 pixels");
				break;
			case TextureError::IMG_INVALID:
				ImGui::TextUnformatted("Image file is invalid or corrupted!");
				ImGui::TextUnformatted(Resources::Texture::GetSTBIError());
				break;
			case TextureError::OTHER:
				break;
				ImGui::TextUnformatted("Could not read Image file!");
			default:
				ImGui::CloseCurrentPopup();
				break;
			}
			if (ImGui::Button("Close"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (manager.get())
		{
			manager->Update();
			manager->Render();
		}
		else
		{
			if (ImGui::Begin("Chat Selection"))
			{
				if (ImGui::Button("Start new Chat"))
				{
					manager = std::make_unique<Chat::ServerChatManager>(users.get(), textures.get(), selfID, fileDialog.get());
				}
				if (ImGui::Button("Join Chat") && !manager.get())
				{
					manager = std::make_unique<Chat::ClientChatManager>(users.get(), textures.get(), selfID, fileDialog.get());
				}
				ImGui::End();
			}
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(window);
	}
}