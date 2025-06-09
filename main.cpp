#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <windows.h>
#include "json.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

using json = nlohmann::json;

const std::string FILE_NAME = "vault.enc";

// AES 256-bit key and IV (for simplicity, static; you can improve with key derivation)
const unsigned char KEY[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1};
const unsigned char IV[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5};

// -------- Encryption / Decryption --------
std::string encrypt(const std::string &plaintext)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    std::string ciphertext;
    ciphertext.resize(plaintext.size() + EVP_MAX_BLOCK_LENGTH);

    int len, ciphertext_len;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, KEY, IV);
    EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]), &len,
                      reinterpret_cast<const unsigned char *>(plaintext.data()), plaintext.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]) + len, &len);
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

std::string decrypt(const std::string &ciphertext)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    std::string plaintext;
    plaintext.resize(ciphertext.size());

    int len, plaintext_len;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, KEY, IV);
    EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]), &len,
                      reinterpret_cast<const unsigned char *>(ciphertext.data()), ciphertext.size());
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]) + len, &len) <= 0)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("Decryption failed");
    }

    plaintext_len += len;
    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}

// -------- Load & Save JSON --------
json load_data()
{
    std::ifstream file(FILE_NAME, std::ios::binary);
    if (!file)
        return json::object();

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string encrypted = ss.str();

    try
    {
        std::string decrypted = decrypt(encrypted);
        return json::parse(decrypted);
    }
    catch (...)
    {
        std::cerr << "Failed to decrypt or parse data.\n";
        return json::object();
    }
}

void save_data(const json &data)
{
    std::ofstream file(FILE_NAME, std::ios::binary);
    std::string plaintext = data.dump();
    std::string encrypted = encrypt(plaintext);
    file.write(encrypted.data(), encrypted.size());
}

// -------- GUI Application --------
int main()
{
    if (!glfwInit())
        return -1;
    GLFWwindow *window = glfwCreateWindow(640, 480, "Password Manager", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // App State
    json data = load_data();
    static char website[128] = "";
    static char username[128] = "";
    static char password[128] = "";
    static std::string search_result = "";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);
        ImGui::Begin("Password Manager");

        ImGui::InputText("Website", website, IM_ARRAYSIZE(website));
        ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
        ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

        if (ImGui::Button("Add Entry"))
        {
            if (data.contains(website) && data[website].is_array())
            {
                // Already an array — add to it
                data[website].push_back({{"username", username}, {"password", password}});
            }
            else if (data.contains(website) && data[website].is_object())
            {
                // Old object — convert to array
                json temp = data[website];
                data[website] = json::array();
                data[website].push_back(temp);
                data[website].push_back({{"username", username}, {"password", password}});
            }
            else
            {
                // New website — create array
                data[website] = json::array();
                data[website].push_back({{"username", username}, {"password", password}});
            }
        }

        ImGui::Separator();

        static char search[128] = "";
        ImGui::InputText("Search Website", search, IM_ARRAYSIZE(search));
        if (ImGui::Button("Find"))
        {
            if (data.contains(search) && data[search].is_array())
            {
                search_result = "";
                int i = 1;
                for (const auto &entry : data[search])
                {
                    search_result += "Entry " + std::to_string(i++) + ":\n";
                    search_result += "  Username: " + entry["username"].get<std::string>() + "\n";
                    search_result += "  Password: " + entry["password"].get<std::string>() + "\n\n";
                }
            }
            else if (data.contains(search) && data[search].is_object())
            {
                search_result = "Username: " + data[search]["username"].get<std::string>() + "\n";
                search_result += "Password: " + data[search]["password"].get<std::string>();
            }
            else
            {
                search_result = "No data found.";
            }
        }

        if (!search_result.empty())
        {
            ImGui::TextWrapped("%s", search_result.c_str());
        }

        ImGui::Separator();

        static char delete_website[128] = "";
        ImGui::InputText("Website to Delete From", delete_website, IM_ARRAYSIZE(delete_website));

        static int delete_index = 0;
        ImGui::InputInt("Entry Index to Delete (0-based)", &delete_index);

        if (ImGui::Button("Delete Entry"))
        {
            if (data.contains(delete_website) && data[delete_website].is_array())
            {
                if (delete_index >= 0 && delete_index < data[delete_website].size())
                {
                    data[delete_website].erase(data[delete_website].begin() + delete_index);
                    if (data[delete_website].empty())
                    {
                        data.erase(delete_website); // Remove the website key if no entries remain
                    }
                    save_data(data);
                }
            }
        }

        static bool show_all_websites = false;
        static std::vector<std::string> all_websites;

        if (ImGui::Button("Show All Websites"))
        {
            all_websites.clear();
            for (auto &[key, value] : data.items())
            {
                all_websites.push_back(key);
            }
            show_all_websites = true;
        }

        if (show_all_websites)
        {
            ImGui::Separator();
            ImGui::Text("Stored Websites:");
            for (const auto &site : all_websites)
            {
                ImGui::BulletText("%s", site.c_str());
            }
            if (ImGui::CollapsingHeader("Stored Entries"))
            {
                for (auto &[site, entries] : data.items())
                {
                    ImGui::Text("%s", site.c_str());
                    if (entries.is_array())
                    {
                        for (size_t i = 0; i < entries.size(); ++i)
                        {
                            std::string entry = "  [" + std::to_string(i) + "] " + "Username: " + entries[i]["username"].get<std::string>() + ", Password: " + entries[i]["password"].get<std::string>();
                            ImGui::BulletText("%s", entry.c_str());
                        }
                    }
                }
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
