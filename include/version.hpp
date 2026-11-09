#pragma once

#include <string>
#include <sstream>

namespace algo {

// Système de versionnement sémantique (SemVer 2.0.0)
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 8;
constexpr int VERSION_PATCH = 1;
constexpr const char* VERSION_TAG = ""; // Ex: "-alpha", "-beta", "-rc1" ou "" pour release

// Version courte : ex. "1.0.0"
inline std::string getVersionString() {
    std::ostringstream oss;
    oss << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH;
    if (VERSION_TAG[0] != '\0') {
        oss << VERSION_TAG;
    }
    return oss.str();
}

// Identification du compilateur utilisé pour le build
inline std::string getCompilerInfo() {
#if defined(__clang__)
    return "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__) + "." + std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    return "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_VER);
#else
    return "Compilateur C++ generique";
#endif
}

// Identification de la plateforme et de l'architecture cible
inline std::string getTargetPlatformInfo() {
#if defined(_WIN64)
    return "Windows (x86_64)";
#elif defined(_WIN32)
    return "Windows (x86)";
#elif defined(__linux__)
    #if defined(__x86_64__)
        return "Linux (x86_64)";
    #elif defined(__aarch64__)
        return "Linux (aarch64)";
    #else
        return "Linux";
    #endif
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Inconnu";
#endif
}

// Informations détaillées et complètes sur la version
inline std::string getFullVersionInfo() {
    std::ostringstream oss;
    oss << "AlgorithmCompiler v" << getVersionString() << "\n"
        << "  Standard C++ : C++20 (ISO/IEC 14882:2020)\n"
        << "  Plateforme   : " << getTargetPlatformInfo() << "\n"
        << "  Compilateur  : " << getCompilerInfo() << "\n"
        << "  Date Build   : " << __DATE__ << " " << __TIME__ << "\n"
        << "  Conformite   : Standard algorithmique tunisien (Secondaire & Baccalaureat)\n"
        << "  Licence      : MIT License\n";
    return oss.str();
}

} // namespace algo
