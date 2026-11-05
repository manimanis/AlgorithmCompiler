Algorithme test_jump_chaining
Début
    cpt ← 0
    Pour i de 1 à 10 Faire
        Si i = 2 Alors
            // Rien
        Sinon Si i = 4 Alors
            // Rien
        Sinon Si i = 6 Alors
            // Rien
        Sinon
            cpt ← cpt + i
        Fin Si
    Fin Pour
    Ecrire("Somme filtrée =", cpt)
Fin

TDO
Objet : Type/Nature
cpt, i : entier
