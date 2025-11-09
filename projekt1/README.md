Sortowanie przez scalanie z użyciem wielkich buforów



Informacje ogólne:

technika sortowania zewnętrznego której celem jest minimalizacja liczby operacji dyskowych

jest konieczne gdy sortowany plik jest za duży żeby go pomieścić w całości w pamięci operacyjnej RAM

polega na sortowniu pliku w częściach (serie ,runs) , serie łączone są tworząc mniej ale dłuższych serii.

Kluczowe oznaczenia:

N - liczba rekordów w pliku

b - współczynnik blokowania (liczba rekordów w jednym buforze/stronie dyskowej B/R)

n - liczba buforów w pamięci dostępnych do procesu sortowania

Ogólnie o algorytmie:

Załąduj n*b rekordów z pliku wejściowego do n buforów pamięci głównej w moim wypadku bufor to poprostu vector<vector<record>>

ten vector sortujemy quicksortem

następnie posortowany blok jest zapisywany na dysk (wydaje się żę nowy txt)

i kroki 1-3 powtarzane są aż będzie skończony plik