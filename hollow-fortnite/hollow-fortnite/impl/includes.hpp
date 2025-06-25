#pragma once
#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
#include <fstream>
#include <winternl.h>
#include <cstdint>
#include <functional>
#include <DbgHelp.h>
#include <thread>
#include <dwmapi.h>
#include <d3d9types.h>
#include <mutex>
#include <map>
#include <array>
#include <utility>
#include <random>
#include <algorithm>
#include <numbers>
#include <cassert>
#include <limits>
#include <cmath>

// frameworks
#include <dependencies/imgui/imgui.h>
#include <dependencies/imgui/imgui_internal.h>
#include <dependencies/imgui/backend/imgui_impl_dx11.h>
#include <dependencies/imgui/backend/imgui_impl_win32.h>
#include <d3d11.h>

// depdendencies
#include <dependencies/g_vars.hpp>

//cipher
#include <dependencies/skcrypt/skcrypter.hpp>

// kernel driver communication