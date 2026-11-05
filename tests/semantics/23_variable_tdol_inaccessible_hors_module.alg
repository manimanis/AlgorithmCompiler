Procédure ModuleA()
Début
    var_localeA ← 42
Fin

TDOL
Objet : Type/Nature
var_localeA : entier

Procédure ModuleB()
Début
    // Tentative illégale d'accéder à la variable locale de ModuleA
    Ecrire(var_localeA)
Fin

TDOL
Objet : Type/Nature
var_localeB : entier

Algorithme test_inaccessible
Début
    ModuleA()
    ModuleB()
Fin

TDO
Objet : Type/Nature
