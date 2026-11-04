# ========================================================
# Programme transpilé depuis Algorithme : 01 _nombres_premiers
# Transpilateur Python avec NumPy et programmation structurée
# (Approche 1 pour Répéter..Jusqu'à : drapeau booléen)
# ========================================================

def est_premier(n):
    d = 0
    premier = False
    if (n < 2):
        return False
    premier = True
    d = 2
    while (((d * d) <= n) and premier):
        if ((n % d) == 0):
            premier = False
        d = (d + 1)
    return premier

i = 0

for i in range(2, 20 + 1):
    if est_premier(i):
        print(i, end=" ")
print("")
