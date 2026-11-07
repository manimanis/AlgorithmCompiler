// Test: Signatures et TDNT respectifs
Procédure AfficherNotes(t: TabNotes, n: entier)
Début
    Pour i de 0 à n - 1 Faire
        Ecrire(t[i])
    Fin Pour
Fin

TDOL
Objet : Type/Nature
i : entier

Fonction EstMajeur(e: Etudiant): booléen
Début
    Retourner e.age >= 18
Fin

Algorithme TestTdntRespectif
Début
    notes[0] ← 15.5
    notes[1] ← 18.0
    AfficherNotes(notes, 2)
    e.age ← 20
    Si EstMajeur(e) Alors
        Ecrire("Admis")
    Fin Si
Fin

TDNT
TabNotes = Tableau de 10 réel
Etudiant = Enregistrement
    age : entier
Fin
MatriceNotes = Tableau de 5 lignes * 5 colonnes de entier

TDO
Objet : Type/Nature
notes : TabNotes
e : Etudiant
