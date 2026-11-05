Algorithme test_fichiers_acces_direct
Début
    Ouvrir("f_direct.tmp", f, "wb")
    v ← 10
    Ecrire(f, v)
    v ← 20
    Ecrire(f, v)
    v ← 30
    Ecrire(f, v)
    Fermer(f)

    Ouvrir("f_direct.tmp", f, "rb")
    Ecrire("Taille fichier =", Taille_Fichier(f))
    Ecrire("Position initiale =", Position_Fichier(f))

    Aller_A(f, 1)
    Ecrire("Position après Aller_A(1) =", Position_Fichier(f))
    Lire(f, val)
    Ecrire("Valeur lue à pos 1 =", val)

    Aller_A(f, 0)
    Lire(f, val)
    Ecrire("Valeur lue à pos 0 =", val)

    Fermer(f)
Fin

TDO
Objet : Type/Nature
f : Fichier de entier
v, val : entier
