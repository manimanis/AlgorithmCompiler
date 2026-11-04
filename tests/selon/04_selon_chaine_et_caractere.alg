Algorithme test_selon_caractere
Début
  c <- "B"
  Selon c
    "A" .. "Z" : Ecrire("Majuscule")
    "a" .. "z" : Ecrire("Minuscule")
    "0" .. "9" : Ecrire("Chiffre")
    Sinon
      Ecrire("Special")
  Fin Selon

  c <- "m"
  Selon c
    'A' .. 'Z' : Ecrire("Majuscule")
    'a' .. 'z' : Ecrire("Minuscule")
    '0' .. '9' : Ecrire("Chiffre")
    Sinon
      Ecrire("Special")
  Fin Selon

  c <- "#"
  Selon c
    'A' .. 'Z' : Ecrire("Majuscule")
    'a' .. 'z' : Ecrire("Minuscule")
    '0' .. '9' : Ecrire("Chiffre")
    Sinon
      Ecrire("Special")
  Fin Selon
Fin

TDO
Objet : Type/Nature
c : Chaine
