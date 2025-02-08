Strategia abordata, implica utilizarea unei structuri ce contine date comune
fiecarui thread. Aceasta include o coada de fisiere, in care stochez numele
fisierului si id-ul lui, un vector in care sunt retinute valorile dupa rularea
thread-urilor de map (mapare a fiecarui cuvant la fisier), primitive de sincronizare
si o coada ce contine literele mici ale alfabetului si numarul de thread-uri map
respectiv reduce. Pentru a identifica fiecare thread am utilizat o alta structura
ce contine o referina catre datele comune si id-ul thread-ului.

In functia threadFunction() am extras parametrul si apoi am impartit executia
in 2 parti. Prima parte reprezinta operatia de map. Aici extrag din coada de
fisiere numele unui fisier pe care il deschid si citesc din el fiecare cuvant,
pe care il transform in litere mici eliminand caracterele speciale precum "'".
Apoi adaug cuvantul intr-un rezultat local, pe care il adaug la rezultatul partial
din structura. Thread-urile se opresc atunci cand nu mai sunt fisiere in coada.
Intre cele 2 parti este o bariera care se blocheaza pana cand thread-urile map
isi termina executia (asteapta ca nr. mapperi + nr. reduceri thread-uri sa 
ajunga la bariera).
A doua parte este reprezentata de operatia de reduce. Aici am mapat fiecare
string, care incepe cu un caracer din alfabet, la un set de fisiere in care se
gaseste, carecterele alfabetului fiind organizate intr-o coada din care fiecare
thread isi extrage cate un caracter. Apoi am utilizat un vector
pentru a stoca rezultatele finale din mapare si le-am sortat. Am deschis
fisierul cu denumirea corespunzatoare si am efectuat afisarea conform
cerintei. Am ales sa utilizez un map si apoi sa transfer rezultatul intr-un
vector pentru a evita cautarea unei perechi de fiecare data cand trebuie sa
adaug id-ul unui fisier la un cuvant.

In main() astept terminarea tuturor thread-urilor si fac join.
