Proiect PA - Ultimate Tic Tac Toe - Etapa 3 - 10/05/2016
Echipa: Trustbit
Studenti:
    *   Dinu Alexandru
    *   Munteanu Filip
    *   Fusu Elian
    *   Parcioaga Catalin-Ionut
Grupa: 322CB

#   Detalii despre implementare

Pentru etapa 4 am pastrat aceeasi versiune ca si pentru etapa anterioara: 

Am regandit structura proiectului, pastrand o mare majoritate din functiile pe care le-am utilizat in etapele anterioare si optimizand anumiti algoritmi de verificare.

O schimbare importanta pe care am adus-o este imbunatatirea euristicii, care conceptual se bazeaza pe evaluarea individuala a fiecarui patrat 3x3 continut in macroboard si constructia unei matrice intermediare care va trece, la randul ei, printr-un proces de evaluare, la finalul caruia se va returna scorul asociat starii curente. 
Calculele necesare evaluarilor se bazeaza pe scoruri predefinite pentru anumite situatii generice, cum ar fi castigul / pierderea jocului.

Ideea pe care am avut-o pentru aceasta etapa a fost sa folosim sume de produse pentru a evalua o anumita stare, astfel incat in cazul in care bot-ul nostru nu are posibilitate de castig (de exemplu) pe o anumita coloana / linie / diagonala, scorul pe respectiva regiune va fi 0, iar scorul general va fi foarte "restrans". Acest rezultat este diferit de un scor care ar fi rezultat daca am fi asignat in toate cazurile punctaje pozitive pentru bot-ul nostru si punctaje negative pentru adversar, ideea noastra oferindu-ne un grad mai mare de restrictivitate (multiple scoruri 0 -> sanse mici (inexistente, poate) de castig).

Scorul starii curente (scorFinal = scorBot - scorAdvesar) este calculat in momentul in care, in minimax, se ajunge la depth-ul maxim sau se intra intr-o stare finala.

Imbunatatirea versiunii curente de minimax a constat in calculul dinamic al depth-ului, relativ la numarul de mutari disponibile (rezultate obtinute in urma observatiilor).

O alta optimizare pe care am adus-o pentru aceasta etapa a fost constituita de felul in care se calculeaza posiblitatea castigului final al meciului (fara apel minimax, ci cu observarea unor pattern-uri). 
In cazul in care jocul este aproape de o stare finala si configuratia tablei (codificata de noi ca un string de 9 biti) respecta un sablon (anume ca pentru castigul final al meciului este nevoie doar de castigul unui patrat 3x3).
Folosind string-uri binare eficientizam calculul si, totodata, simplificam felul in care ne referim la unul dintre cele 9 patrate din macroboard, printr-o mapare 1-to-1 intre pozita bitului in string si coordonatele (x, y) ale acestui patrat.

Conform observatiilor noastre, aceasta metoda s-a dovedit foarte eficienta pentru cazul in care, in mod normal, algoritmul minimax ar avea de analizat un numar relativ mare de mutari posibile.

Pe langa aceste aspecte, trebuie sa mentionam ca prima mutare a jocului (in cazul in care suntem player1) se face in centrul tablei. Totodata, am observat ca acest comportant conduce la meciuri identice cu aceeasi boti.

Detaliile concrete ale implementarii acestei solutii se afla in comentariile din cod.