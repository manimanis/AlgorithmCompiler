#include "../include/bytecode.hpp"
#include "../include/vm.hpp"
#include "../include/version.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    algo::initConsoleUtf8();
    std::string self = algo::getSelfExecutablePath();
    if (algo::isStandaloneExecutable(self)) {
        return algo::runStandaloneExecutable(self, argc, argv);
    }
    std::cerr << "AlgorithmCompiler Runtime Stub v" << algo::getVersionString() << "\n";
    std::cerr << "Ce binaire sert de modèle autonome pour l'option '--build-exe' de algo.\n";
    std::cerr << "Utilisation : algo <fichier.alg> --build-exe <mon_programme.exe>\n";
    return 1;
}
