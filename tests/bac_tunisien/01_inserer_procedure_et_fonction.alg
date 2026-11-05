Algorithme test_inserer
Début
    ch ← ""
    ch ← Inserer("bonjour", ch, 0)
    ch ← Inserer(" monde", ch, 7)
    Ecrire("ch =", ch)

    res ← Inserer("!!", ch, Long(ch))
    Ecrire("res =", res)
    Ecrire("ch apres =", ch)
Fin

TDO
Objet : Type/Nature
ch, res : chaine
