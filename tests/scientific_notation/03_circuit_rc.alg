Procédure Saisir_Donnees(@E: réel, @C: réel, @R: réel)
Début
  Répéter
    Ecrire("Tension du générateur :") ; Lire(E)
  Jusqu'à 0 < E ≤ 50
  Répéter
    Ecrire("Capacité du condensateur :") ; Lire(C)
  Jusqu'à 10 ≤ C ≤ 2700000
  Répéter
    Ecrire("Résistance du résistor :") ; Lire(R)
  Jusqu'à 10 ≤ R ≤ 1000000
Fin

Fonction Calcul_Tau(R: réel, C: réel):réel
Début
  Retourner R * C * 1e-9
Fin

Procédure Afficher_Etat(uc: réel, E: réel)
Début
  Si uc = 0 Alors
    Ecrire("Condensateur déchargé")
  Sinon Si uc ≥ 0.99*E Alors
    Ecrire("Régime permanent")
  Sinon
    Ecrire("Régime transitoire")
  Fin Si
Fin

Procédure Courbe_Charge(E: réel, tau: réel)
Début
  Pour k de 0 à 5 Faire
    t ← 0.5 * k * tau
    uc ← E * (1 - exp(-t / tau))
    Ecrire("A l'instant t =", t, "s, la tension vaut :", uc, "V")
  Fin Pour
Fin

TDOL
Objet : Type/Nature
t, uc : réel

Fonction Tension_Decharge(t: réel, E: réel, tau: réel):réel
Début
  Retourner E * exp(-t / tau)
Fin

Fonction Energie(C: réel, uc: réel):réel
Début
  Retourner 0.5 * (C * 1e-9) * uc * uc
Fin

Procédure Suivi_Decharge(E: réel, tau: réel, C: réel)
Début
  t ← 0
  Répéter
    uc ← Tension_Decharge(t, E, tau)
    w ← Energie(C, uc)
    Ecrire("t =", t, "s - uc =", uc, "V - w =", w, "J")
    t ← t + 0.5 * tau
  Jusqu'à uc ≤ 0.01*E
  Ecrire("Durée totale de décharge =", t, "s")
Fin

TDO
Objet : Type/Nature
t, uc, w : réel
Tension_Decharge, Energie : Fonction

Algorithme Etude_Circuit_RC
Début
  Saisir_Donnees(E, C, R)
  tau ← Calcul_Tau(R, C)
  Courbe_Charge(E, tau)
  Suivi_Decharge(E, tau, C)
Fin

TDOG
Objet : Type/Nature
E, C, R, tau : réel
Saisir_Donnees, Courbe_Charge, Suivi_Decharge : Procédure
Calcul_Tau : Fonction
