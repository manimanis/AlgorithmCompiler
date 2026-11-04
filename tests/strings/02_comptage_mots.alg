// Test: Comptage de mots dans une chaîne
Fonction compter_mots(phrase: chaîne): entier
Var nb, i: entier
    dans_mot: booléen
Début
    nb ← 0
    dans_mot ← Faux
    Pour i de 0 à Long(phrase) - 1 Faire
        Si phrase[i] ≠ " " Et phrase[i] ≠ "\t" Alors
            Si Non dans_mot Alors
                nb ← nb + 1
                dans_mot ← Vrai
            Fin Si
        Sinon
            dans_mot ← Faux
        Fin Si
    Fin Pour
    Retourner nb
Fin

TDOL
Objet : Type/Nature
nb, i : entier
dans_mot : booléen

Algorithme 02_comptage_mots
Début
    Ecrire(compter_mots("Bonjour le monde"))
    Ecrire(compter_mots("   Algorithmique   en C++  moderne  "))
    Ecrire(compter_mots("Test"))
Fin
