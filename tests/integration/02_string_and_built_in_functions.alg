Algorithme 02_string_and_built_in_functions
Début
    a ← "La vie est comme une bicyclette, il faut avancer pour ne pas perdre l'équilibre"
    b ← long(a)	
    c ← b mod 16
    d ← pos("ne", a)
    e ← pos("il", a)
    f ← pos("pour", a)
    g ← sous_chaine(a, f, b) + 
        sous_chaine(a, e - 1, f)
    h ← racine(f)
    i ← Long(ConvCh(f)) = 1
    j ← Valeur(ConvCh(c) + ConvCh(d) +
            "." + ConvCh(b + c))
    k ← Arrondi(j * 10)
    l ← Effacer(a, 31, b)
    m ← Effacer(l, 3, 21)
    Ecrire(a)
    Ecrire(b)
    Ecrire(c)
    Ecrire(d)
    Ecrire(e)
    Ecrire(f)
    Ecrire(g)
    Ecrire(h)
    Ecrire(i)
    Ecrire(j)
    Ecrire(k)
    Ecrire(l)
    Ecrire(m)
Fin

TDO
Objet : Type/Nature
a, g, l, m : chaîne
b, c, d, e, f, k : entier
h, j : réel
i : booléen
