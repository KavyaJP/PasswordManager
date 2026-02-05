# Password Manager GUI in C++

A simple password manager with a graphical user interface using **ImGui**, **GLFW**, **OpenGL**, and **OpenSSL** for encryption.  
Stores encrypted password data locally in a file.

---

## Clone the repository

To get a local copy of this project, run:

```bash
git clone https://github.com/KavyaJP/PasswordManager.git
cd PasswordManager
```

---

## Features

- Add and view saved passwords by website
- Passwords are stored encrypted using AES-256-CBC (OpenSSL)
- Cross-platform GUI with ImGui + GLFW + OpenGL3
- Uses JSON for data storage format (`nlohmann/json`)

---

## Project Structure

```
/Project-Folder
├── app.ico
├── icon.o
├── imconfig.h
├── imgui_demo.cpp
├── imgui_draw.cpp
├── imgui_impl_glfw.cpp
├── imgui_impl_glfw.h
├── imgui_impl_opengl3_loader.h
├── imgui_impl_opengl3.cpp
├── imgui_impl_opengl3.h
├── imgui_internal.h
├── imgui_tables.cpp
├── imgui_widgets.cpp
├── imgui.cpp
├── imgui.h
├── imstb_rectpack.h
├── imstb_textedit.h
├── imstb_truetype.h
├── json.hpp
├── main.cpp -> Main Password Manager Program
└── resource.rc
```

---

## Prerequisites

- [MSYS2](https://www.msys2.org/) environment with `mingw-w64-x86_64` toolchain
- Install required libraries via MSYS2:

```bash
pacman -Syu
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw mingw-w64-x86_64-openssl
```

- `windres` (comes with MSYS2)

---

## Build Instructions

1. Compile the resource.rc file into an object file using this command (For adding an icon to the built exe, Optional), the icon.o is already present, only recompile if the icon.o is corrupted or if you want to change the icon, in which case put the icon in the same folder as the repository and name it as app.ico

```bash
windres resource.rc -o icon.o
```

2. Open the MSYS2 MinGW64 shell and navigate to your project directory, then compile with:

```bash
g++ main.cpp icon.o imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp imgui_demo.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp -I. -L/mingw64/lib -lglfw3 -lopengl32 -lgdi32 -luser32 -lssl -lcrypto -lws2_32 -o PasswordManager.exe -std=c++17 -mwindows
```

### Note
- If you haven't created icon.o file using command 1 then remove icon.o from the command
- If you are not on windows then remove -mwindows from the end of command in step 2, -mwindows is a windows specific flag that makes it so that CMD doesn't open when you start the PasswordManager.exe by double clicking.

---

## Run

Execute the compiled binary:

```bash
./password_gui.exe
```

---

## Notes

- This project uses a fixed AES-256 key and IV for simplicity.
- For production, consider using a secure password-based key derivation function (PBKDF2, Argon2, etc.) and safer key management.
- All password data is stored encrypted in vault.enc in the program folder.

---

## License

MIT License

---

## Acknowledgements

- [Dear ImGui](https://github.com/ocornut/imgui) — Immediate mode GUI library
- [GLFW](https://www.glfw.org/) — Windowing and input
- [OpenSSL](https://www.openssl.org/) — Cryptography
- [nlohmann/json](https://github.com/nlohmann/json) — JSON for Modern C++

---

### Feel Free to Contribute
