# Suite de Tests pour l'Interpréteur d'Algorithmes

Ce dossier contient une suite complète et extensible de tests pour l'interpréteur d'algorithmes en C++.

## Structure d'un Cas de Test

Chaque test est autonome et repose sur une convention de nommage simple :

1. **`<nom>.alg`** *(obligatoire)* : Le code source de l'algorithme à exécuter.
2. **`<nom>.in`** *(optionnel)* : Les entrées standards (lignes successives injectées pour les instructions `Lire(...)`).
3. **`<nom>.out`** *(optionnel)* : La sortie standard attendue produite par `Ecrire(...)`. Le test réussit si la sortie de l'algorithme correspond exactement.
4. **`<nom>.err`** *(optionnel)* : Pour les tests de robustesse ou négatifs devant échouer. Le test réussit si une exception est levée et contient le texte spécifié.

### Directives d'en-tête (dans le fichier `.alg`)

Vous pouvez configurer l'exécution directement via des commentaires d'en-tête :
```alg
// @max-iter: 50        -> Fixe la limite maximale d'itérations
// @timeout: 50         -> Fixe le timeout d'exécution en millisecondes
// @seed: 42            -> Fixe la graine initiale du générateur PRNG
```

## Comment ajouter un nouveau test ?

Pour ajouter un nouveau test, il suffit de créer un nouveau fichier `.alg` dans n'importe quel sous-dossier de `tests/` (ou un nouveau sous-dossier), accompagné de son fichier `.out` (ou `.err`) et `.in` si nécessaire.

Exemple :
- `tests/arithmetic/04_factorielle.alg`
- `tests/arithmetic/04_factorielle.in` (si saisie au clavier)
- `tests/arithmetic/04_factorielle.out` (résultat attendu)

Le test runner `run_tests.exe` détecte et exécute automatiquement tout nouveau test sans aucune modification de code C++.

---

## Sommaire des Tests (223 algorithmes)

- [arithmetic](#arithmetic) (3 algorithmes) - Nombres premiers, somme des chiffres, nombres parfaits
- [arrays](#arrays) (4 algorithmes) - Déclarations, accès et affectations de tableaux
- [basics](#basics) (10 algorithmes) - Types primitifs, commentaires, déclarations multiples
- [control-flow](#control-flow) (9 algorithmes) - Si/Sinon, Pour (croissant/décroissant/pas), TantQue, Répéter
- [converter](#converter) (1 algorithmes)
- [expressions](#expressions) (15 algorithmes) - Opérations arithmétiques, comparaisons, booléens, appartenance ∈
- [graine](#graine) (2 algorithmes) - Graine et nombres pseudo-aléatoires déterministes
- [input](#input) (4 algorithmes) - Saisies clavier simples, multiples, tableaux
- [integration](#integration) (5 algorithmes) - Programmes complets
- [integration2](#integration2) (4 algorithmes) - PGCD/PPCM, chaînes, recherche dicho, cryptage
- [integration3](#integration3) (3 algorithmes) - Gestion de drones, tri complet, révision
- [integration4](#integration4) (6 algorithmes) - Manipulations avancées et algorithmes d'examen
- [output](#output) (8 algorithmes) - Écritures avec séparateurs et paramètres de fin
- [pointers](#pointers) (9 algorithmes) - Pointeurs, allocation dynamique Allouer/Liberer, listes, piles, files, arbres, graphes
- [procedures](#procedures) (9 algorithmes) - Sous-programmes, portée locale, fonctions prédéfinies
- [recursion](#recursion) (3 algorithmes) - Fibonacci récursif, Tours de Hanoï, PGCD récursif
- [robustness](#robustness) (26 algorithmes) - Détection de boucles infinies, divisions par zéro, hors bornes, timeouts
- [strings](#strings) (2 algorithmes) - Palindromes, comptage de mots
- [var-optional](#var-optional) (5 algorithmes) - Déclarations sans mot-clé Var

---

### arrays

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_array_type_declaration.alg`](./arrays/01_array_type_declaration.alg) | Array type declaration | 9 | 5 |
| [`02_array_type_declaration_with_short_name.alg`](./arrays/02_array_type_declaration_with_short_name.alg) | Array type declaration with short name | 18 | 5 |
| [`03_array_access_and_assignment.alg`](./arrays/03_array_access_and_assignment.alg) | Array access and assignment | 27 | 7 |
| [`04_array_default_values.alg`](./arrays/04_array_default_values.alg) | Array default values | 40 | 5 |
### basics

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_empty_program_execution.alg`](./basics/01_empty_program_execution.alg) | Empty program execution | 16 | 1 |
| [`02_variable_declaration_integer.alg`](./basics/02_variable_declaration_integer.alg) | Variable declaration - integer | 21 | 5 |
| [`03_variable_declaration_real.alg`](./basics/03_variable_declaration_real.alg) | Variable declaration - real | 31 | 5 |
| [`04_variable_declaration_boolean.alg`](./basics/04_variable_declaration_boolean.alg) | Variable declaration - boolean | 41 | 5 |
| [`05_variable_declaration_string.alg`](./basics/05_variable_declaration_string.alg) | Variable declaration - string | 51 | 5 |
| [`06_multiple_writes.alg`](./basics/06_multiple_writes.alg) | Multiple writes | 61 | 4 |
| [`07_write_with_mixed_types.alg`](./basics/07_write_with_mixed_types.alg) | Write with mixed types | 70 | 3 |
| [`08_single_line_comment.alg`](./basics/08_single_line_comment.alg) | Single-line comment // | 77 | 4 |
| [`09_multi_line_comment.alg`](./basics/09_multi_line_comment.alg) | Multi-line comment /* */ | 85 | 5 |
| [`10_var_declaration_with_continuation_lines.alg`](./basics/10_var_declaration_with_continuation_lines.alg) | Var declaration with continuation lines | 94 | 9 |
### control-flow

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_if_statement_true_condition.alg`](./control-flow/01_if_statement_true_condition.alg) | If statement - true condition | 9 | 9 |
| [`02_if_statement_false_condition.alg`](./control-flow/02_if_statement_false_condition.alg) | If statement - false condition | 23 | 9 |
| [`03_if_else_statement.alg`](./control-flow/03_if_else_statement.alg) | If-else statement | 37 | 11 |
| [`04_for_loop_ascending.alg`](./control-flow/04_for_loop_ascending.alg) | For loop - ascending | 55 | 8 |
| [`05_for_loop_descending.alg`](./control-flow/05_for_loop_descending.alg) | For loop - descending | 68 | 8 |
| [`06_for_loop_with_step.alg`](./control-flow/06_for_loop_with_step.alg) | For loop - with step | 81 | 8 |
| [`07_for_loop_with_negative_step.alg`](./control-flow/07_for_loop_with_negative_step.alg) | For loop - with negative step | 94 | 8 |
| [`08_while_loop.alg`](./control-flow/08_while_loop.alg) | While loop | 109 | 10 |
| [`09_repeat_loop.alg`](./control-flow/09_repeat_loop.alg) | Repeat loop | 126 | 10 |
### converter

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_converter_repeat_assignment_uses_target_names.alg`](./converter/01_converter_repeat_assignment_uses_target_names.alg) | Converter repeat assignment uses target names | 21 | 10 |
### expressions

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_addition.alg`](./expressions/01_addition.alg) | Addition | 9 | 5 |
| [`02_subtraction.alg`](./expressions/02_subtraction.alg) | Subtraction | 19 | 5 |
| [`03_multiplication.alg`](./expressions/03_multiplication.alg) | Multiplication | 29 | 5 |
| [`04_division.alg`](./expressions/04_division.alg) | Division | 39 | 5 |
| [`05_integer_division_div.alg`](./expressions/05_integer_division_div.alg) | Integer division (div) | 49 | 5 |
| [`06_modulo_mod.alg`](./expressions/06_modulo_mod.alg) | Modulo (mod) | 59 | 5 |
| [`07_unary_minus.alg`](./expressions/07_unary_minus.alg) | Unary minus | 69 | 5 |
| [`08_equality_comparison.alg`](./expressions/08_equality_comparison.alg) | Equality comparison | 81 | 7 |
| [`09_inequality_comparison.alg`](./expressions/09_inequality_comparison.alg) | Inequality comparison | 95 | 7 |
| [`10_less_than_comparison.alg`](./expressions/10_less_than_comparison.alg) | Less than comparison | 109 | 7 |
| [`11_greater_than_comparison.alg`](./expressions/11_greater_than_comparison.alg) | Greater than comparison | 123 | 7 |
| [`12_appartenance_avec.alg`](./expressions/12_appartenance_avec.alg) | Appartenance avec ∈ | 137 | 10 |
| [`13_logical_and_et.alg`](./expressions/13_logical_and_et.alg) | Logical AND (Et) | 153 | 11 |
| [`14_logical_or_ou.alg`](./expressions/14_logical_or_ou.alg) | Logical OR (Ou) | 175 | 11 |
| [`15_logical_not_non.alg`](./expressions/15_logical_not_non.alg) | Logical NOT (Non) | 197 | 7 |
### graine

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_should_produce_identical_random_numbers_when_seeded_wit.alg`](./graine/01_should_produce_identical_random_numbers_when_seeded_wit.alg) | should produce identical random numbers when seeded with same value | 24 | 5 |
| [`02_should_convert_graine_to_seed_in_python_converter.alg`](./graine/02_should_convert_graine_to_seed_in_python_converter.alg) | should convert graine to seed in Python converter | 37 | 4 |
### input

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_input_with_prompt.alg`](./input/01_input_with_prompt.alg) | Input with prompt | 9 | 5 |
| [`02_input_multiple_values.alg`](./input/02_input_multiple_values.alg) | Input multiple values | 27 | 7 |
| [`03_input_with_array_access.alg`](./input/03_input_with_array_access.alg) | Input with array access | 49 | 8 |
| [`04_input_with_array_access_and_procedure_call.alg`](./input/04_input_with_array_access_and_procedure_call.alg) | Input with array access and procedure call | 73 | 28 |
### integration

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_arithmetic_and_operator_precedence.alg`](./integration/01_arithmetic_and_operator_precedence.alg) | Arithmetic and operator precedence | 9 | 13 |
| [`02_string_and_built_in_functions.alg`](./integration/02_string_and_built_in_functions.alg) | String and built-in functions | 37 | 18 |
| [`03_integer_vs_real_division.alg`](./integration/03_integer_vs_real_division.alg) | Integer vs real division | 73 | 13 |
| [`04_binary_conversion_and_gray_code.alg`](./integration/04_binary_conversion_and_gray_code.alg) | Binary conversion and Gray code | 101 | 24 |
| [`05_array_sorting_with_procedures.alg`](./integration/05_array_sorting_with_procedures.alg) | Array sorting with procedures | 140 | 47 |
### integration2

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_calcul_du_pgcd_et_ppcm.alg`](./integration2/01_calcul_du_pgcd_et_ppcm.alg) | Calcul du PGCD et PPCM | 9 | 22 |
| [`02_fonctions_sur_les_chaines.alg`](./integration2/02_fonctions_sur_les_chaines.alg) | Fonctions sur les Chaines | 38 | 34 |
| [`03_recherche_dichotomique.alg`](./integration2/03_recherche_dichotomique.alg) | Recherche dichotomique | 79 | 46 |
| [`04_programme_de_cryptage.alg`](./integration2/04_programme_de_cryptage.alg) | Programme de cryptage | 136 | 114 |
### integration3

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_algorithme_drones.alg`](./integration3/01_algorithme_drones.alg) | Algorithme Drones | 134 | 98 |
| [`02_algorithme_de_revision.alg`](./integration3/02_algorithme_de_revision.alg) | Algorithme de révision | 326 | 153 |
| [`03_tri_d_un_tableau.alg`](./integration3/03_tri_d_un_tableau.alg) | Tri d'un tableau | 456 | 104 |
### integration4

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_declaration_et_affectation_chaine.alg`](./integration4/01_declaration_et_affectation_chaine.alg) | Déclaration et affectation chaîne | 23 | 14 |
| [`02_affectation_et_concatenation_chaine.alg`](./integration4/02_affectation_et_concatenation_chaine.alg) | Affectation et concaténation chaîne | 34 | 5 |
| [`03_fonction_verif.alg`](./integration4/03_fonction_verif.alg) | Fonction verif | 59 | 20 |
| [`04_fonction_verif_et_saisie.alg`](./integration4/04_fonction_verif_et_saisie.alg) | Fonction verif et saisie | 106 | 28 |
| [`05_algorithme_degre_de_ressemblance.alg`](./integration4/05_algorithme_degre_de_ressemblance.alg) | Algorithme Degré de ressemblance | 248 | 101 |
| [`06_ex.alg`](./integration4/06_ex.alg) | Ex | 355 | 76 |
### output

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_output_simple.alg`](./output/01_output_simple.alg) | Output simple | 9 | 7 |
| [`02_output_with_variable.alg`](./output/02_output_with_variable.alg) | Output with variable | 23 | 5 |
| [`03_output_with_string_and_variable.alg`](./output/03_output_with_string_and_variable.alg) | Output with string and variable | 32 | 5 |
| [`04_output_with_multiple_variables.alg`](./output/04_output_with_multiple_variables.alg) | Output with multiple variables | 41 | 6 |
| [`05_output_with_expression.alg`](./output/05_output_with_expression.alg) | Output with expression | 51 | 6 |
| [`06_output_with_separator.alg`](./output/06_output_with_separator.alg) | Output with separator | 61 | 7 |
| [`07_output_with_end_parameter.alg`](./output/07_output_with_end_parameter.alg) | Output with end parameter | 72 | 7 |
| [`08_output_with_both_separator_and_end.alg`](./output/08_output_with_both_separator_and_end.alg) | Output with both separator and end | 83 | 10 |
### pointers

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_pointeur_entier_allocation_liberation.alg`](./pointers/01_pointeur_entier_allocation_liberation.alg) | Allocation et libération de pointeur entier | 1 | 23 |
| [`02_liste_chainee_simple.alg`](./pointers/02_liste_chainee_simple.alg) | Liste chaînée simple et parcours séquentiel | 1 | 48 |
| [`03_pile_pointeur.alg`](./pointers/03_pile_pointeur.alg) | Pile dynamique LIFO (Empiler / Dépiler) | 1 | 54 |
| [`04_file_pointeur.alg`](./pointers/04_file_pointeur.alg) | File dynamique FIFO (Enfiler / Défiler) | 1 | 63 |
| [`05_liste_doublement_chainee.alg`](./pointers/05_liste_doublement_chainee.alg) | Liste doublement chaînée (précédent/suivant) | 1 | 66 |
| [`06_arbre_binaire_recherche.alg`](./pointers/06_arbre_binaire_recherche.alg) | Arbre binaire de recherche (ABR) et parcours infixe | 1 | 58 |
| [`07_pointeur_alias_et_partage.alg`](./pointers/07_pointeur_alias_et_partage.alg) | Partage de références sur le tas et mutation | 1 | 30 |
| [`08_graphe_liste_adjacence.alg`](./pointers/08_graphe_liste_adjacence.alg) | Graphe représenté par listes d'adjacence dynamiques | 1 | 52 |
| [`09_syntaxe_synonymes_pointeurs.alg`](./pointers/09_syntaxe_synonymes_pointeurs.alg) | Syntaxes synonymes (^T, Pointeur sur T, NIL, NULL) | 1 | 35 |
### procedures

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_procedure_call.alg`](./procedures/01_procedure_call.alg) | Procedure call | 9 | 8 |
| [`02_procedure_with_parameters.alg`](./procedures/02_procedure_with_parameters.alg) | Procedure with parameters | 21 | 8 |
| [`03_function_call.alg`](./procedures/03_function_call.alg) | Function call | 37 | 8 |
| [`04_function_with_local_variables.alg`](./procedures/04_function_with_local_variables.alg) | Function with local variables | 49 | 13 |
| [`05_chr_function.alg`](./procedures/05_chr_function.alg) | chr function | 68 | 3 |
| [`06_ord_function.alg`](./procedures/06_ord_function.alg) | ord function | 75 | 3 |
| [`07_long_function.alg`](./procedures/07_long_function.alg) | Long function | 82 | 3 |
| [`08_racine_function.alg`](./procedures/08_racine_function.alg) | Racine function | 89 | 3 |
| [`09_ent_function.alg`](./procedures/09_ent_function.alg) | Ent function | 96 | 3 |
### robustness

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_infinite_loop_detection_while_true.alg`](./robustness/01_infinite_loop_detection_while_true.alg) | Infinite loop detection - while true | 20 | 6 |
| [`02_infinite_loop_detection_for_with_step_0.alg`](./robustness/02_infinite_loop_detection_for_with_step_0.alg) | Infinite loop detection - for with step 0 | 33 | 6 |
| [`03_large_loop_completes_within_limits.alg`](./robustness/03_large_loop_completes_within_limits.alg) | Large loop completes within limits | 45 | 7 |
| [`04_custom_iteration_limit.alg`](./robustness/04_custom_iteration_limit.alg) | Custom iteration limit | 59 | 7 |
| [`05_division_by_zero_throws_error.alg`](./robustness/05_division_by_zero_throws_error.alg) | Division by zero throws error | 72 | 5 |
| [`06_integer_division_by_zero_throws_error.alg`](./robustness/06_integer_division_by_zero_throws_error.alg) | Integer division by zero throws error | 82 | 5 |
| [`07_modulo_by_zero_throws_error.alg`](./robustness/07_modulo_by_zero_throws_error.alg) | Modulo by zero throws error | 92 | 5 |
| [`08_division_by_variable_that_is_zero.alg`](./robustness/08_division_by_variable_that_is_zero.alg) | Division by variable that is zero | 104 | 7 |
| [`09_array_access_out_of_bounds_positive_index.alg`](./robustness/09_array_access_out_of_bounds_positive_index.alg) | Array access out of bounds - positive index | 119 | 8 |
| [`10_array_access_out_of_bounds_negative_index.alg`](./robustness/10_array_access_out_of_bounds_negative_index.alg) | Array access out of bounds - negative index | 131 | 7 |
| [`11_array_assignment_out_of_bounds.alg`](./robustness/11_array_assignment_out_of_bounds.alg) | Array assignment out of bounds | 142 | 6 |
| [`12_valid_array_access_within_bounds.alg`](./robustness/12_valid_array_access_within_bounds.alg) | Valid array access within bounds | 155 | 8 |
| [`13_undefined_variable_throws_error.alg`](./robustness/13_undefined_variable_throws_error.alg) | Undefined variable throws error | 167 | 3 |
| [`14_undefined_procedure_throws_error.alg`](./robustness/14_undefined_procedure_throws_error.alg) | Undefined procedure throws error | 175 | 3 |
| [`15_undefined_function_throws_error.alg`](./robustness/15_undefined_function_throws_error.alg) | Undefined function throws error | 185 | 5 |
| [`16_for_loop_with_non_numeric_bounds_throws_error.alg`](./robustness/16_for_loop_with_non_numeric_bounds_throws_error.alg) | For loop with non-numeric bounds throws error | 198 | 6 |
| [`17_for_loop_with_string_step_throws_error.alg`](./robustness/17_for_loop_with_string_step_throws_error.alg) | For loop with string step throws error | 208 | 5 |
| [`18_empty_program_executes_without_error.alg`](./robustness/18_empty_program_executes_without_error.alg) | Empty program executes without error | 266 | 2 |
| [`19_program_with_only_comments.alg`](./robustness/19_program_with_only_comments.alg) | Program with only comments | 278 | 6 |
| [`20_nested_loops_with_limits.alg`](./robustness/20_nested_loops_with_limits.alg) | Nested loops with limits | 293 | 9 |
| [`21_deep_recursion_with_return_value.alg`](./robustness/21_deep_recursion_with_return_value.alg) | Deep recursion with return value | 311 | 12 |
| [`22_invalid_input_for_integer_converts_correctly.alg`](./robustness/22_invalid_input_for_integer_converts_correctly.alg) | Invalid input for integer converts correctly | 324 | 5 |
| [`23_invalid_input_for_real_converts_correctly.alg`](./robustness/23_invalid_input_for_real_converts_correctly.alg) | Invalid input for real converts correctly | 335 | 5 |
| [`24_boolean_input_conversion.alg`](./robustness/24_boolean_input_conversion.alg) | Boolean input conversion | 346 | 5 |
| [`25_custom_max_iterations_option.alg`](./robustness/25_custom_max_iterations_option.alg) | Custom max iterations option | 365 | 6 |
| [`26_custom_timeout_option.alg`](./robustness/26_custom_timeout_option.alg) | Custom timeout option | 390 | 7 |
### var-optional

| Fichier | Test Source | Ligne | Lignes de code |
| :--- | :--- | :---: | :---: |
| [`01_variable_declaration_without_var_keyword_single_variabl.alg`](./var-optional/01_variable_declaration_without_var_keyword_single_variabl.alg) | Variable declaration without var keyword - single variable | 9 | 5 |
| [`02_variable_declaration_without_var_keyword_multiple_varia.alg`](./var-optional/02_variable_declaration_without_var_keyword_multiple_varia.alg) | Variable declaration without var keyword - multiple variables same type | 19 | 6 |
| [`03_variable_declaration_without_var_keyword_multiple_lines.alg`](./var-optional/03_variable_declaration_without_var_keyword_multiple_lines.alg) | Variable declaration without var keyword - multiple lines | 31 | 9 |
| [`04_variable_declaration_without_var_keyword_boolean.alg`](./var-optional/04_variable_declaration_without_var_keyword_boolean.alg) | Variable declaration without var keyword - boolean | 47 | 5 |
| [`05_mixed_declarations_with_and_without_var.alg`](./var-optional/05_mixed_declarations_with_and_without_var.alg) | Mixed declarations with and without var | 57 | 7 |
